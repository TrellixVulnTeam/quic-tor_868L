/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file buffers.c
 * \brief Implements a generic buffer interface.
 *
 * A buf_t is a (fairly) opaque byte-oriented FIFO that can read to or flush
 * from memory, sockets, file descriptors, TLS connections, or another buf_t.
 * Buffers are implemented as linked lists of memory chunks.
 *
 * All socket-backed and TLS-based connection_t objects have a pair of
 * buffers: one for incoming data, and one for outcoming data.  These are fed
 * and drained from functions in connection.c, trigged by events that are
 * monitored in main.c.
 **/

#define BUFFERS_PRIVATE
#include "orconfig.h"
#include <stddef.h>
#include "buffers.h"
#include "compat.h"
#include "compress.h"
#include "util.h"
#include "torint.h"
#include "torlog.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

//#define PARANOIA

#ifdef PARANOIA
/** Helper: If PARANOIA is defined, assert that the buffer in local variable
 * <b>buf</b> is well-formed. */
#define check() STMT_BEGIN buf_assert_ok(buf); STMT_END
#else
#define check() STMT_NIL
#endif /* defined(PARANOIA) */

/* Implementation notes:
 *
 * After flirting with memmove, and dallying with ring-buffers, we're finally
 * getting up to speed with the 1970s and implementing buffers as a linked
 * list of small chunks.  Each buffer has such a list; data is removed from
 * the head of the list, and added at the tail.  The list is singly linked,
 * and the buffer keeps a pointer to the head and the tail.
 *
 * Every chunk, except the tail, contains at least one byte of data.  Data in
 * each chunk is contiguous.
 *
 * When you need to treat the first N characters on a buffer as a contiguous
 * string, use the buf_pullup function to make them so.  Don't do this more
 * than necessary.
 *
 * The major free Unix kernels have handled buffers like this since, like,
 * forever.
 */

/* Chunk manipulation functions */

#define CHUNK_HEADER_LEN offsetof(chunk_t, mem[0])

/* We leave this many NUL bytes at the end of the buffer. */
#ifdef DISABLE_MEMORY_SENTINELS
#define SENTINEL_LEN 0
#else
#define SENTINEL_LEN 4
#endif

/* Header size plus NUL bytes at the end */
#define CHUNK_OVERHEAD (CHUNK_HEADER_LEN + SENTINEL_LEN)

/** Return the number of bytes needed to allocate a chunk to hold
 * <b>memlen</b> bytes. */
#define CHUNK_ALLOC_SIZE(memlen) (CHUNK_OVERHEAD + (memlen))
/** Return the number of usable bytes in a chunk allocated with
 * malloc(<b>memlen</b>). */
#define CHUNK_SIZE_WITH_ALLOC(memlen) ((memlen) - CHUNK_OVERHEAD)

#define DEBUG_SENTINEL

#if defined(DEBUG_SENTINEL) && !defined(DISABLE_MEMORY_SENTINELS)
#define DBG_S(s) s
#else
#define DBG_S(s) (void)0
#endif

#ifdef DISABLE_MEMORY_SENTINELS
#define CHUNK_SET_SENTINEL(chunk, alloclen) STMT_NIL
#else
#define CHUNK_SET_SENTINEL(chunk, alloclen) do {                        \
    uint8_t *a = (uint8_t*) &(chunk)->mem[(chunk)->memlen];             \
    DBG_S(uint8_t *b = &((uint8_t*)(chunk))[(alloclen)-SENTINEL_LEN]);  \
    DBG_S(tor_assert(a == b));                                          \
    memset(a,0,SENTINEL_LEN);                                           \
  } while (0)
#endif /* defined(DISABLE_MEMORY_SENTINELS) */

/** Move all bytes stored in <b>chunk</b> to the front of <b>chunk</b>->mem,
 * to free up space at the end. */
static inline void
chunk_repack(chunk_t *chunk)
{
  if (chunk->datalen && chunk->data != &chunk->mem[0]) {
    memmove(chunk->mem, chunk->data, chunk->datalen);
  }
  chunk->data = &chunk->mem[0];
}

/** Keep track of total size of allocated chunks for consistency asserts */
static size_t total_bytes_allocated_in_chunks = 0;
static void
buf_chunk_free_unchecked(chunk_t *chunk)
{
  if (!chunk)
    return;
#ifdef DEBUG_CHUNK_ALLOC
  tor_assert(CHUNK_ALLOC_SIZE(chunk->memlen) == chunk->DBG_alloc);
#endif
  tor_assert(total_bytes_allocated_in_chunks >=
             CHUNK_ALLOC_SIZE(chunk->memlen));
  total_bytes_allocated_in_chunks -= CHUNK_ALLOC_SIZE(chunk->memlen);
  tor_free(chunk);
}
static inline chunk_t *
chunk_new_with_alloc_size(size_t alloc)
{
  chunk_t *ch;
  ch = tor_malloc(alloc);
  ch->stream_id = 0;
  ch->next = NULL;
  ch->datalen = 0;
#ifdef DEBUG_CHUNK_ALLOC
  ch->DBG_alloc = alloc;
#endif
  ch->memlen = CHUNK_SIZE_WITH_ALLOC(alloc);
  total_bytes_allocated_in_chunks += alloc;
  ch->data = &ch->mem[0];
  CHUNK_SET_SENTINEL(ch, alloc);
  return ch;
}

/** Expand <b>chunk</b> until it can hold <b>sz</b> bytes, and return a
 * new pointer to <b>chunk</b>.  Old pointers are no longer valid. */
static inline chunk_t *
chunk_grow(chunk_t *chunk, size_t sz)
{
  off_t offset;
  const size_t memlen_orig = chunk->memlen;
  const size_t orig_alloc = CHUNK_ALLOC_SIZE(memlen_orig);
  const size_t new_alloc = CHUNK_ALLOC_SIZE(sz);
  tor_assert(sz > chunk->memlen);
  offset = chunk->data - chunk->mem;
  chunk = tor_realloc(chunk, new_alloc);
  chunk->memlen = sz;
  chunk->data = chunk->mem + offset;
#ifdef DEBUG_CHUNK_ALLOC
  tor_assert(chunk->DBG_alloc == orig_alloc);
  chunk->DBG_alloc = new_alloc;
#endif
  total_bytes_allocated_in_chunks += new_alloc - orig_alloc;
  CHUNK_SET_SENTINEL(chunk, new_alloc);
  return chunk;
}

/** Every chunk should take up at least this many bytes. */
#define MIN_CHUNK_ALLOC 256
/** No chunk should take up more than this many bytes. */
#define MAX_CHUNK_ALLOC 65536

/** Return the allocation size we'd like to use to hold <b>target</b>
 * bytes. */
size_t
buf_preferred_chunk_size(size_t target)
{
  tor_assert(target <= SIZE_T_CEILING - CHUNK_OVERHEAD);
  if (CHUNK_ALLOC_SIZE(target) >= MAX_CHUNK_ALLOC)
    return CHUNK_ALLOC_SIZE(target);
  size_t sz = MIN_CHUNK_ALLOC;
  while (CHUNK_SIZE_WITH_ALLOC(sz) < target) {
    sz <<= 1;
  }
  return sz;
}

/** Collapse data from the first N chunks from <b>buf</b> into buf->head,
 * growing it as necessary, until buf->head has the first <b>bytes</b> bytes
 * of data from the buffer, or until buf->head has all the data in <b>buf</b>.
 *
 * Set *<b>head_out</b> to point to the first byte of available data, and
 * *<b>len_out</b> to the number of bytes of data available at
 * *<b>head_out</b>. Note that *<b>len_out</b> may be more or less than
 * <b>bytes</b>, depending on the number of bytes available.
 */
void
buf_pullup(buf_t *buf, size_t bytes, const char **head_out, size_t *len_out)
{
  chunk_t *dest, *src;
  size_t capacity;
  if (!buf->head) {
    *head_out = NULL;
    *len_out = 0;
    return;
  }

  check();
  if (buf->datalen < bytes)
    bytes = buf->datalen;

  capacity = bytes;
  if (buf->head->datalen >= bytes) {
    *head_out = buf->head->data;
    *len_out = buf->head->datalen;
    return;
  }

  if (buf->head->memlen >= capacity) {
    /* We don't need to grow the first chunk, but we might need to repack it.*/
    size_t needed = capacity - buf->head->datalen;
    if (CHUNK_REMAINING_CAPACITY(buf->head) < needed)
      chunk_repack(buf->head);
    tor_assert(CHUNK_REMAINING_CAPACITY(buf->head) >= needed);
  } else {
    chunk_t *newhead;
    size_t newsize;
    /* We need to grow the chunk. */
    chunk_repack(buf->head);
    newsize = CHUNK_SIZE_WITH_ALLOC(buf_preferred_chunk_size(capacity));
    newhead = chunk_grow(buf->head, newsize);
    tor_assert(newhead->memlen >= capacity);
    if (newhead != buf->head) {
      if (buf->tail == buf->head)
        buf->tail = newhead;
      buf->head = newhead;
    }
  }

  dest = buf->head;
  while (dest->datalen < bytes) {
    size_t n = bytes - dest->datalen;
    src = dest->next;
    tor_assert(src);
    if (n >= src->datalen) {
      memcpy(CHUNK_WRITE_PTR(dest), src->data, src->datalen);
      dest->datalen += src->datalen;
      dest->next = src->next;
      if (buf->tail == src)
        buf->tail = dest;
      buf_chunk_free_unchecked(src);
    } else {
      memcpy(CHUNK_WRITE_PTR(dest), src->data, n);
      dest->datalen += n;
      src->data += n;
      src->datalen -= n;
      tor_assert(dest->datalen == bytes);
    }
  }

  check();
  *head_out = buf->head->data;
  *len_out = buf->head->datalen;
}

#ifdef TOR_UNIT_TESTS
/* Write sz bytes from cp into a newly allocated buffer buf.
 * Returns NULL when passed a NULL cp or zero sz.
 * Asserts on failure: only for use in unit tests.
 * buf must be freed using buf_free(). */
buf_t *
buf_new_with_data(const char *cp, size_t sz)
{
  /* Validate arguments */
  if (!cp || sz <= 0) {
    return NULL;
  }

  tor_assert(sz < SSIZE_T_CEILING);

  /* Allocate a buffer */
  buf_t *buf = buf_new_with_capacity(sz);
  tor_assert(buf);
  buf_assert_ok(buf);
  tor_assert(!buf->head);

  /* Allocate a chunk that is sz bytes long */
  buf->head = chunk_new_with_alloc_size(CHUNK_ALLOC_SIZE(sz));
  buf->tail = buf->head;
  tor_assert(buf->head);
  buf_assert_ok(buf);
  tor_assert(buf_allocation(buf) >= sz);

  /* Copy the data and size the buffers */
  tor_assert(sz <= buf_slack(buf));
  tor_assert(sz <= CHUNK_REMAINING_CAPACITY(buf->head));
  memcpy(&buf->head->mem[0], cp, sz);
  buf->datalen = sz;
  buf->head->datalen = sz;
  buf->head->data = &buf->head->mem[0];
  buf_assert_ok(buf);

  /* Make sure everything is large enough */
  tor_assert(buf_allocation(buf) >= sz);
  tor_assert(buf_allocation(buf) >= buf_datalen(buf) + buf_slack(buf));
  /* Does the buffer implementation allocate more than the requested size?
   * (for example, by rounding up). If so, these checks will fail. */
  tor_assert(buf_datalen(buf) == sz);
  tor_assert(buf_slack(buf) == 0);

  return buf;
}
#endif /* defined(TOR_UNIT_TESTS) */

/** Remove the first <b>n</b> bytes from buf. */
void
buf_drain(buf_t *buf, size_t n)
{
  tor_assert(buf->datalen >= n);
  while (n) {
    tor_assert(buf->head);
    if (buf->head->datalen > n) {
      buf->head->datalen -= n;
      buf->head->data += n;
      buf->datalen -= n;
      return;
    } else {
      chunk_t *victim = buf->head;
      n -= victim->datalen;
      buf->datalen -= victim->datalen;
      buf->head = victim->next;
      if (buf->tail == victim)
        buf->tail = NULL;
      buf_chunk_free_unchecked(victim);
    }
  }
  check();
}

/** Create and return a new buf with default chunk capacity <b>size</b>.
 */
buf_t *
buf_new_with_capacity(size_t size)
{
  buf_t *b = buf_new();
  b->default_chunk_size = buf_preferred_chunk_size(size);
  return b;
}

/** Allocate and return a new buffer with default capacity. */
buf_t *
buf_new(void)
{
  buf_t *buf = tor_malloc_zero(sizeof(buf_t));
  buf->magic = BUFFER_MAGIC;
  buf->default_chunk_size = 4096;
  return buf;
}

size_t
buf_get_default_chunk_size(const buf_t *buf)
{
  return buf->default_chunk_size;
}

/** Remove all data from <b>buf</b>. */
void
buf_clear(buf_t *buf)
{
  chunk_t *chunk, *next;
  buf->datalen = 0;
  for (chunk = buf->head; chunk; chunk = next) {
    next = chunk->next;
    buf_chunk_free_unchecked(chunk);
  }
  buf->head = buf->tail = NULL;
}

/** Return the number of bytes stored in <b>buf</b> */
MOCK_IMPL(size_t,
buf_datalen, (const buf_t *buf))
{
  return buf->datalen;
}

/** Return the total length of all chunks used in <b>buf</b>. */
size_t
buf_allocation(const buf_t *buf)
{
  size_t total = 0;
  const chunk_t *chunk;
  for (chunk = buf->head; chunk; chunk = chunk->next) {
    total += CHUNK_ALLOC_SIZE(chunk->memlen);
  }
  return total;
}

/** Return the number of bytes that can be added to <b>buf</b> without
 * performing any additional allocation. */
size_t
buf_slack(const buf_t *buf)
{
  if (!buf->tail)
    return 0;
  else
    return CHUNK_REMAINING_CAPACITY(buf->tail);
}

/** Release storage held by <b>buf</b>. */
void
buf_free_(buf_t *buf)
{
  if (!buf)
    return;

  buf_clear(buf);
  buf->magic = 0xdeadbeef;
  tor_free(buf);
}

/** Return a new copy of <b>in_chunk</b> */
static chunk_t *
chunk_copy(const chunk_t *in_chunk)
{
  chunk_t *newch = tor_memdup(in_chunk, CHUNK_ALLOC_SIZE(in_chunk->memlen));
  total_bytes_allocated_in_chunks += CHUNK_ALLOC_SIZE(in_chunk->memlen);
#ifdef DEBUG_CHUNK_ALLOC
  newch->DBG_alloc = CHUNK_ALLOC_SIZE(in_chunk->memlen);
#endif
  newch->next = NULL;
  if (in_chunk->data) {
    off_t offset = in_chunk->data - in_chunk->mem;
    newch->data = newch->mem + offset;
  }
  return newch;
}

/** Return a new copy of <b>buf</b> */
buf_t *
buf_copy(const buf_t *buf)
{
  chunk_t *ch;
  buf_t *out = buf_new();
  out->default_chunk_size = buf->default_chunk_size;
  for (ch = buf->head; ch; ch = ch->next) {
    chunk_t *newch = chunk_copy(ch);
    if (out->tail) {
      out->tail->next = newch;
      out->tail = newch;
    } else {
      out->head = out->tail = newch;
    }
  }
  out->datalen = buf->datalen;
  return out;
}

/** Append a new chunk with enough capacity to hold <b>capacity</b> bytes to
 * the tail of <b>buf</b>.  If <b>capped</b>, don't allocate a chunk bigger
 * than MAX_CHUNK_ALLOC. */
chunk_t *
buf_add_chunk_with_capacity(buf_t *buf, size_t capacity, int capped)
{
  chunk_t *chunk;

  if (CHUNK_ALLOC_SIZE(capacity) < buf->default_chunk_size) {
    chunk = chunk_new_with_alloc_size(buf->default_chunk_size);
  } else if (capped && CHUNK_ALLOC_SIZE(capacity) > MAX_CHUNK_ALLOC) {
    chunk = chunk_new_with_alloc_size(MAX_CHUNK_ALLOC);
  } else {
    chunk = chunk_new_with_alloc_size(buf_preferred_chunk_size(capacity));
  }

  chunk->inserted_time = monotime_coarse_get_stamp();

  if (buf->tail) {
    tor_assert(buf->head);
    buf->tail->next = chunk;
    buf->tail = chunk;
  } else {
    tor_assert(!buf->head);
    buf->head = buf->tail = chunk;
  }
  check();
  return chunk;
}

/** Return the age of the oldest chunk in the buffer <b>buf</b>, in
 * timestamp units.  Requires the current monotonic timestamp as its
 * input <b>now</b>.
 */
uint32_t
buf_get_oldest_chunk_timestamp(const buf_t *buf, uint32_t now)
{
  if (buf->head) {
    return now - buf->head->inserted_time;
  } else {
    return 0;
  }
}

size_t
buf_get_total_allocation(void)
{
  return total_bytes_allocated_in_chunks;
}

/** Read up to <b>at_most</b> bytes from the socket <b>fd</b> into
 * <b>chunk</b> (which must be on <b>buf</b>). If we get an EOF, set
 * *<b>reached_eof</b> to 1.  Return -1 on error, 0 on eof or blocking,
 * and the number of bytes read otherwise. */
static inline int
read_to_chunk(buf_t *buf, chunk_t *chunk, tor_socket_t fd, size_t at_most,
              int *reached_eof, int *socket_error)
{
  ssize_t read_result;
  if (at_most > CHUNK_REMAINING_CAPACITY(chunk))
    at_most = CHUNK_REMAINING_CAPACITY(chunk);
  read_result = tor_socket_recv(fd, CHUNK_WRITE_PTR(chunk), at_most, 0);

  if (read_result < 0) {
    int e = tor_socket_errno(fd);
    if (!ERRNO_IS_EAGAIN(e)) { /* it's a real error */
#ifdef _WIN32
      if (e == WSAENOBUFS)
        log_warn(LD_NET,"recv() failed: WSAENOBUFS. Not enough ram?");
#endif
      *socket_error = e;
      return -1;
    }
    return 0; /* would block. */
  } else if (read_result == 0) {
    log_debug(LD_NET,"Encountered eof on fd %d", (int)fd);
    *reached_eof = 1;
    return 0;
  } else { /* actually got bytes. */
    buf->datalen += read_result;
    chunk->datalen += read_result;
    log_debug(LD_NET,"Read %ld bytes. %d on inbuf.", (long)read_result,
              (int)buf->datalen);
    tor_assert(read_result < INT_MAX);
    return (int)read_result;
  }
}

// Read to buf for QUIC
static inline int
read_to_chunk_quic(buf_t *buf, chunk_t *chunk, tor_quicsock_t fd, size_t at_most,
              int *reached_eof, int *socket_error)
{
  ssize_t read_result;
  if (at_most > CHUNK_REMAINING_CAPACITY(chunk)) {
    at_most = CHUNK_REMAINING_CAPACITY(chunk);
  }

  read_result = qs_recv(fd, CHUNK_WRITE_PTR(chunk), at_most);

  if (read_result == -2) {
    // nothing more to read; not EOF though
    return 0;
  } else if (read_result == -1) {
    // problem
    return -1;
  } else if (read_result == 0) {
    log_debug(LD_NET,"Encountered eof on fd %d", qs_get_fd(fd));
    *reached_eof = 1;
    return 0;
  } else { /* actually got bytes. */
    buf->datalen += read_result;
    chunk->datalen += read_result;
    log_debug(LD_NET,"Read %ld bytes. %d on inbuf.", (long)read_result,
              (int)buf->datalen);
    tor_assert(read_result < INT_MAX);
    return (int)read_result;
  }
}
int
read_to_buf_quic(tor_quicsock_t s, size_t at_most, buf_t *buf, int *reached_eof,
            int *socket_error)
{
  int r = 0;
  size_t total_read = 0;

  check();
  tor_assert(reached_eof);
  tor_assert(QUICSOCK_OK(s));

  while (at_most > total_read) {
    size_t readlen = at_most - total_read;
    chunk_t *chunk;
    if (!buf->tail || CHUNK_REMAINING_CAPACITY(buf->tail) < MIN_READ_LEN) {
      chunk = buf_add_chunk_with_capacity(buf, at_most, 1);
      if (readlen > chunk->memlen)
        readlen = chunk->memlen;
    } else {
      size_t cap = CHUNK_REMAINING_CAPACITY(buf->tail);
      chunk = buf->tail;
      if (cap < readlen)
        readlen = cap;
    }

    r = read_to_chunk_quic(buf, chunk, s, readlen, reached_eof, socket_error);
    check();
    if (r < 0)
      return r; /* Error */
    tor_assert(total_read+r < INT_MAX);
    total_read += r;
    if ((size_t)r < readlen) { /* eof, block, or no more to read. */
      break;
    }
  }
  return (int)total_read;
}
/** Read from socket <b>s</b>, writing onto end of <b>buf</b>.  Read at most
 * <b>at_most</b> bytes, growing the buffer as necessary.  If recv() returns 0
 * (because of EOF), set *<b>reached_eof</b> to 1 and return 0. Return -1 on
 * error; else return the number of bytes read.
 */
/* XXXX indicate "read blocked" somehow? */
int
buf_read_from_socket(buf_t *buf, tor_socket_t s, size_t at_most,
                     int *reached_eof,
                     int *socket_error)
{
  /* XXXX It's stupid to overload the return values for these functions:
   * "error status" and "number of bytes read" are not mutually exclusive.
   */
  int r = 0;
  size_t total_read = 0;

  check();
  tor_assert(reached_eof);
  tor_assert(SOCKET_OK(s));

  if (BUG(buf->datalen >= INT_MAX))
    return -1;
  if (BUG(buf->datalen >= INT_MAX - at_most))
    return -1;

  while (at_most > total_read) {
    size_t readlen = at_most - total_read;
    chunk_t *chunk;
    if (!buf->tail || CHUNK_REMAINING_CAPACITY(buf->tail) < MIN_READ_LEN) {
      chunk = buf_add_chunk_with_capacity(buf, at_most, 1);
      if (readlen > chunk->memlen)
        readlen = chunk->memlen;
    } else {
      size_t cap = CHUNK_REMAINING_CAPACITY(buf->tail);
      chunk = buf->tail;
      if (cap < readlen)
        readlen = cap;
    }

    r = read_to_chunk(buf, chunk, s, readlen, reached_eof, socket_error);
    check();
    if (r < 0)
      return r; /* Error */
    tor_assert(total_read+r < INT_MAX);
    total_read += r;
    if ((size_t)r < readlen) { /* eof, block, or no more to read. */
      break;
    }
  }
  return (int)total_read;
}

/** Helper for buf_flush_to_socket(): try to write <b>sz</b> bytes from chunk
 * <b>chunk</b> of buffer <b>buf</b> onto socket <b>s</b>.  On success, deduct
 * the bytes written from *<b>buf_flushlen</b>.  Return the number of bytes
 * written on success, 0 on blocking, -1 on failure.
 */
static inline int
flush_chunk(tor_socket_t s, buf_t *buf, chunk_t *chunk, size_t sz,
            size_t *buf_flushlen)
{
  ssize_t write_result;

  if (sz > chunk->datalen)
    sz = chunk->datalen;
  write_result = tor_socket_send(s, chunk->data, sz, 0);

  if (write_result < 0) {
    int e = tor_socket_errno(s);
    if (!ERRNO_IS_EAGAIN(e)) { /* it's a real error */
#ifdef _WIN32
      if (e == WSAENOBUFS)
        log_warn(LD_NET,"write() failed: WSAENOBUFS. Not enough ram?");
#endif
      return -1;
    }
    log_debug(LD_NET,"write() would block, returning.");
    return 0;
  } else {
    *buf_flushlen -= write_result;
    buf_drain(buf, write_result);
    tor_assert(write_result < INT_MAX);
    return (int)write_result;
  }
}
// QUIC specific helper functions
void
print_cell_to_log(char *buf, ssize_t len) {
  char *str = (char*)malloc(len);
  for (int i = 0; i< len; i++) {
    if (buf[i] == '\0') {
      str[i] = '\n';
    } else {
      str[i] = buf[i];
    }
  }

  log_debug(LD_NET, "**** QUIC trying to write this (raw): ****");
  log_debug(LD_NET, "%s", str);
  log_debug(LD_NET, "******************************************");

  free(str);
}

/* TODO possibly merge the send calls */
static inline int
flush_chunk_quic(tor_quicsock_t s, buf_t *buf, chunk_t *chunk, size_t sz) {
  ssize_t write_result;
  if (sz > chunk->datalen)
    sz = chunk->datalen;
  //print_cell_to_log(chunk->data, sz);
  write_result = qs_send(s, chunk->data, sz, chunk->stream_id);
  if (write_result < 0) {
    log_warn(LD_NET,"QUIC write failed, returning.");
    return -1;
  } else {
	  buf_drain(buf, write_result);
    tor_assert(write_result < INT_MAX);
    return (int)write_result;
  }
}


int
flush_buf_quic(tor_quicsock_t s, buf_t *buf, size_t sz) {
  tor_assert(QUICSOCK_OK(s));

  int r;
  size_t flushed = 0;
  int count = 0;

  check();
  while (sz) {
    size_t flushlen0;
    tor_assert(buf->head);
    if (buf->head->datalen >= sz)
      flushlen0 = sz;
    else
      flushlen0 = buf->head->datalen;

    r = flush_chunk_quic(s, buf, buf->head, flushlen0);

    check();
    if (r < 0 || (size_t)r != flushlen0) {
      log_debug(LD_OR, "returning -1, r = %d", r);
      return -1;
    }

    flushed += r;
    sz -= r;
    count++;
  }

  tor_assert(flushed < INT_MAX);
  return (int)flushed;
}
/** Write data from <b>buf</b> to the socket <b>s</b>.  Write at most
 * <b>sz</b> bytes, decrement *<b>buf_flushlen</b> by
 * the number of bytes actually written, and remove the written bytes
 * from the buffer.  Return the number of bytes written on success,
 * -1 on failure.  Return 0 if write() would block.
 */
int
buf_flush_to_socket(buf_t *buf, tor_socket_t s, size_t sz,
                    size_t *buf_flushlen)
{
  /* XXXX It's stupid to overload the return values for these functions:
   * "error status" and "number of bytes flushed" are not mutually exclusive.
   */
  int r;
  size_t flushed = 0;
  tor_assert(buf_flushlen);
  tor_assert(SOCKET_OK(s));
  if (BUG(*buf_flushlen > buf->datalen)) {
    *buf_flushlen = buf->datalen;
  }
  if (BUG(sz > *buf_flushlen)) {
    sz = *buf_flushlen;
  }

  check();
  while (sz) {
    size_t flushlen0;
    tor_assert(buf->head);
    if (buf->head->datalen >= sz)
      flushlen0 = sz;
    else
      flushlen0 = buf->head->datalen;

    r = flush_chunk(s, buf, buf->head, flushlen0, buf_flushlen);
    check();
    if (r < 0)
      return r;
    flushed += r;
    sz -= r;
    if (r == 0 || (size_t)r < flushlen0) /* can't flush any more now. */
      break;
  }
  tor_assert(flushed < INT_MAX);
  return (int)flushed;
}
/** Append <b>string_len</b> bytes from <b>string</b> to the end of
 * <b>buf</b>.
 *
 * Return the new length of the buffer on success, -1 on failure.
 */
int
write_to_buf(const char *string, size_t string_len, buf_t *buf)
{
    return write_to_buf_quic(string, string_len, buf, (quicsock_stream_id_t)0);
}


// QUIC's write to buffer
int
write_to_buf_quic(const char *string, size_t string_len, buf_t *buf,
             quicsock_stream_id_t stream_id)
{
  if (!string_len)
    return (int)buf->datalen;
  check();

  while (string_len) {
    size_t copy;
    if (!buf->tail || !CHUNK_REMAINING_CAPACITY(buf->tail))
      buf_add_chunk_with_capacity(buf, string_len, 1);

    copy = CHUNK_REMAINING_CAPACITY(buf->tail);
    if (copy > string_len)
      copy = string_len;
    memcpy(CHUNK_WRITE_PTR(buf->tail), string, copy);
    string_len -= copy;
    string += copy;
    buf->datalen += copy;
    buf->tail->datalen += copy;
  }

  check();
  tor_assert(buf->datalen < INT_MAX);
  return (int)buf->datalen;
}

/** Append <b>string_len</b> bytes from <b>string</b> to the end of
 * <b>buf</b>.
 *
 * Return the new length of the buffer on success, -1 on failure.
 */
int
buf_add(buf_t *buf, const char *string, size_t string_len)
{
  if (!string_len)
    return (int)buf->datalen;
  check();

  if (BUG(buf->datalen >= INT_MAX))
    return -1;
  if (BUG(buf->datalen >= INT_MAX - string_len))
    return -1;

  while (string_len) {
    size_t copy;
    if (!buf->tail || !CHUNK_REMAINING_CAPACITY(buf->tail))
      buf_add_chunk_with_capacity(buf, string_len, 1);

    copy = CHUNK_REMAINING_CAPACITY(buf->tail);
    if (copy > string_len)
      copy = string_len;
    memcpy(CHUNK_WRITE_PTR(buf->tail), string, copy);
    string_len -= copy;
    string += copy;
    buf->datalen += copy;
    buf->tail->datalen += copy;
  }

  check();
  tor_assert(buf->datalen < INT_MAX);
  return (int)buf->datalen;
}

/** Add a nul-terminated <b>string</b> to <b>buf</b>, not including the
 * terminating NUL. */
void
buf_add_string(buf_t *buf, const char *string)
{
  buf_add(buf, string, strlen(string));
}

/** As tor_snprintf, but write the results into a buf_t */
void
buf_add_printf(buf_t *buf, const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  buf_add_vprintf(buf, format, ap);
  va_end(ap);
}

/** As tor_vsnprintf, but write the results into a buf_t. */
void
buf_add_vprintf(buf_t *buf, const char *format, va_list args)
{
  /* XXXX Faster implementations are easy enough, but let's optimize later */
  char *tmp;
  tor_vasprintf(&tmp, format, args);
  buf_add(buf, tmp, strlen(tmp));
  tor_free(tmp);
}

/** Return a heap-allocated string containing the contents of <b>buf</b>, plus
 * a NUL byte. If <b>sz_out</b> is provided, set *<b>sz_out</b> to the length
 * of the returned string, not including the terminating NUL. */
char *
buf_extract(buf_t *buf, size_t *sz_out)
{
  tor_assert(buf);

  size_t sz = buf_datalen(buf);
  char *result;
  result = tor_malloc(sz+1);
  buf_peek(buf, result, sz);
  result[sz] = 0;
  if (sz_out)
    *sz_out = sz;
  return result;
}

/** Helper: copy the first <b>string_len</b> bytes from <b>buf</b>
 * onto <b>string</b>.
 */
void
buf_peek(const buf_t *buf, char *string, size_t string_len)
{
  chunk_t *chunk;

  tor_assert(string);
  /* make sure we don't ask for too much */
  tor_assert(string_len <= buf->datalen);
  /* buf_assert_ok(buf); */

  chunk = buf->head;
  while (string_len) {
    size_t copy = string_len;
    tor_assert(chunk);
    if (chunk->datalen < copy)
      copy = chunk->datalen;
    memcpy(string, chunk->data, copy);
    string_len -= copy;
    string += copy;
    chunk = chunk->next;
  }
}

/** Remove <b>string_len</b> bytes from the front of <b>buf</b>, and store
 * them into <b>string</b>.  Return the new buffer size.  <b>string_len</b>
 * must be \<= the number of bytes on the buffer.
 */
int
buf_get_bytes(buf_t *buf, char *string, size_t string_len)
{
  /* There must be string_len bytes in buf; write them onto string,
   * then memmove buf back (that is, remove them from buf).
   *
   * Return the number of bytes still on the buffer. */

  check();
  buf_peek(buf, string, string_len);
  buf_drain(buf, string_len);
  check();
  tor_assert(buf->datalen < INT_MAX);
  return (int)buf->datalen;
}

/** Move up to *<b>buf_flushlen</b> bytes from <b>buf_in</b> to
 * <b>buf_out</b>, and modify *<b>buf_flushlen</b> appropriately.
 * Return the number of bytes actually copied.
 */
/* QUIC MOD: since this function is only used for linked_conn, we can
 * safely assume that stream_id is not very useful since data never goes
 * to the network layer!
 * */
int
buf_move_to_buf(buf_t *buf_out, buf_t *buf_in, size_t *buf_flushlen)
{
  /* We can do way better here, but this doesn't turn up in any profiles. */
  char b[4096];
  size_t cp, len;

  if (BUG(buf_out->datalen >= INT_MAX))
    return -1;
  if (BUG(buf_out->datalen >= INT_MAX - *buf_flushlen))
    return -1;

  len = *buf_flushlen;
  if (len > buf_in->datalen)
    len = buf_in->datalen;

  cp = len; /* Remember the number of bytes we intend to copy. */
  tor_assert(cp < INT_MAX);
  while (len) {
    /* This isn't the most efficient implementation one could imagine, since
     * it does two copies instead of 1, but I kinda doubt that this will be
     * critical path. */
    size_t n = len > sizeof(b) ? sizeof(b) : len;
    buf_get_bytes(buf_in, b, n);
    buf_add(buf_out, b, n);
    len -= n;
  }
  *buf_flushlen -= cp;
  return (int)cp;
}

/** Moves all data from <b>buf_in</b> to <b>buf_out</b>, without copying.
 */
void
buf_move_all(buf_t *buf_out, buf_t *buf_in)
{
  tor_assert(buf_out);
  if (!buf_in)
    return;

  if (buf_out->head == NULL) {
    buf_out->head = buf_in->head;
    buf_out->tail = buf_in->tail;
  } else {
    buf_out->tail->next = buf_in->head;
    buf_out->tail = buf_in->tail;
  }

  buf_out->datalen += buf_in->datalen;
  buf_in->head = buf_in->tail = NULL;
  buf_in->datalen = 0;
}

/** Internal structure: represents a position in a buffer. */
typedef struct buf_pos_t {
  const chunk_t *chunk; /**< Which chunk are we pointing to? */
  int pos;/**< Which character inside the chunk's data are we pointing to? */
  size_t chunk_pos; /**< Total length of all previous chunks. */
} buf_pos_t;

/** Initialize <b>out</b> to point to the first character of <b>buf</b>.*/
static void
buf_pos_init(const buf_t *buf, buf_pos_t *out)
{
  out->chunk = buf->head;
  out->pos = 0;
  out->chunk_pos = 0;
}

/** Advance <b>out</b> to the first appearance of <b>ch</b> at the current
 * position of <b>out</b>, or later.  Return -1 if no instances are found;
 * otherwise returns the absolute position of the character. */
static off_t
buf_find_pos_of_char(char ch, buf_pos_t *out)
{
  const chunk_t *chunk;
  int pos;
  tor_assert(out);
  if (out->chunk) {
    if (out->chunk->datalen) {
      tor_assert(out->pos < (off_t)out->chunk->datalen);
    } else {
      tor_assert(out->pos == 0);
    }
  }
  pos = out->pos;
  for (chunk = out->chunk; chunk; chunk = chunk->next) {
    char *cp = memchr(chunk->data+pos, ch, chunk->datalen - pos);
    if (cp) {
      out->chunk = chunk;
      tor_assert(cp - chunk->data < INT_MAX);
      out->pos = (int)(cp - chunk->data);
      return out->chunk_pos + out->pos;
    } else {
      out->chunk_pos += chunk->datalen;
      pos = 0;
    }
  }
  return -1;
}

/** Advance <b>pos</b> by a single character, if there are any more characters
 * in the buffer.  Returns 0 on success, -1 on failure. */
static inline int
buf_pos_inc(buf_pos_t *pos)
{
  ++pos->pos;
  if (pos->pos == (off_t)pos->chunk->datalen) {
    if (!pos->chunk->next)
      return -1;
    pos->chunk_pos += pos->chunk->datalen;
    pos->chunk = pos->chunk->next;
    pos->pos = 0;
  }
  return 0;
}

/** Return true iff the <b>n</b>-character string in <b>s</b> appears
 * (verbatim) at <b>pos</b>. */
static int
buf_matches_at_pos(const buf_pos_t *pos, const char *s, size_t n)
{
  buf_pos_t p;
  if (!n)
    return 1;

  memcpy(&p, pos, sizeof(p));

  while (1) {
    char ch = p.chunk->data[p.pos];
    if (ch != *s)
      return 0;
    ++s;
    /* If we're out of characters that don't match, we match.  Check this
     * _before_ we test incrementing pos, in case we're at the end of the
     * string. */
    if (--n == 0)
      return 1;
    if (buf_pos_inc(&p)<0)
      return 0;
  }
}

/** Return the first position in <b>buf</b> at which the <b>n</b>-character
 * string <b>s</b> occurs, or -1 if it does not occur. */
int
buf_find_string_offset(const buf_t *buf, const char *s, size_t n)
{
  buf_pos_t pos;
  buf_pos_init(buf, &pos);
  while (buf_find_pos_of_char(*s, &pos) >= 0) {
    if (buf_matches_at_pos(&pos, s, n)) {
      tor_assert(pos.chunk_pos + pos.pos < INT_MAX);
      return (int)(pos.chunk_pos + pos.pos);
    } else {
      if (buf_pos_inc(&pos)<0)
        return -1;
    }
  }
  return -1;
}

/** Return 1 iff <b>buf</b> starts with <b>cmd</b>. <b>cmd</b> must be a null
 * terminated string, of no more than PEEK_BUF_STARTSWITH_MAX bytes. */
int
buf_peek_startswith(const buf_t *buf, const char *cmd)
{
  char tmp[PEEK_BUF_STARTSWITH_MAX];
  size_t clen = strlen(cmd);
  if (clen == 0)
    return 1;
  if (BUG(clen > sizeof(tmp)))
    return 0;
  if (buf->datalen < clen)
    return 0;
  buf_peek(buf, tmp, clen);
  return fast_memeq(tmp, cmd, clen);
}

/** Return the index within <b>buf</b> at which <b>ch</b> first appears,
 * or -1 if <b>ch</b> does not appear on buf. */
static off_t
buf_find_offset_of_char(buf_t *buf, char ch)
{
  chunk_t *chunk;
  off_t offset = 0;
  for (chunk = buf->head; chunk; chunk = chunk->next) {
    char *cp = memchr(chunk->data, ch, chunk->datalen);
    if (cp)
      return offset + (cp - chunk->data);
    else
      offset += chunk->datalen;
  }
  return -1;
}

/** Try to read a single LF-terminated line from <b>buf</b>, and write it
 * (including the LF), NUL-terminated, into the *<b>data_len</b> byte buffer
 * at <b>data_out</b>.  Set *<b>data_len</b> to the number of bytes in the
 * line, not counting the terminating NUL.  Return 1 if we read a whole line,
 * return 0 if we don't have a whole line yet, and return -1 if the line
 * length exceeds *<b>data_len</b>.
 */
int
buf_get_line(buf_t *buf, char *data_out, size_t *data_len)
{
  size_t sz;
  off_t offset;

  if (!buf->head)
    return 0;

  offset = buf_find_offset_of_char(buf, '\n');
  if (offset < 0)
    return 0;
  sz = (size_t) offset;
  if (sz+2 > *data_len) {
    *data_len = sz + 2;
    return -1;
  }
  buf_get_bytes(buf, data_out, sz+1);
  data_out[sz+1] = '\0';
  *data_len = sz+1;
  return 1;
}

/** Compress or uncompress the <b>data_len</b> bytes in <b>data</b> using the
 * compression state <b>state</b>, appending the result to <b>buf</b>.  If
 * <b>done</b> is true, flush the data in the state and finish the
 * compression/uncompression.  Return -1 on failure, 0 on success. */
int
buf_add_compress(buf_t *buf, tor_compress_state_t *state,
                      const char *data, size_t data_len,
                      const int done)
{
  char *next;
  size_t old_avail, avail;
  int over = 0;

  do {
    int need_new_chunk = 0;
    if (!buf->tail || ! CHUNK_REMAINING_CAPACITY(buf->tail)) {
      size_t cap = data_len / 4;
      buf_add_chunk_with_capacity(buf, cap, 1);
    }
    next = CHUNK_WRITE_PTR(buf->tail);
    avail = old_avail = CHUNK_REMAINING_CAPACITY(buf->tail);
    switch (tor_compress_process(state, &next, &avail,
                                 &data, &data_len, done)) {
      case TOR_COMPRESS_DONE:
        over = 1;
        break;
      case TOR_COMPRESS_ERROR:
        return -1;
      case TOR_COMPRESS_OK:
        if (data_len == 0) {
          tor_assert_nonfatal(!done);
          over = 1;
        }
        break;
      case TOR_COMPRESS_BUFFER_FULL:
        if (avail) {
          /* The compression module says we need more room
           * (TOR_COMPRESS_BUFFER_FULL).  Start a new chunk automatically,
           * whether were going to or not. */
          need_new_chunk = 1;
        }
        if (data_len == 0 && !done) {
          /* We've consumed all the input data, though, so there's no
           * point in forging ahead right now. */
          over = 1;
        }
        break;
    }
    buf->datalen += old_avail - avail;
    buf->tail->datalen += old_avail - avail;
    if (need_new_chunk) {
      buf_add_chunk_with_capacity(buf, data_len/4, 1);
    }

  } while (!over);
  check();
  return 0;
}

/** Set *<b>output</b> to contain a copy of the data in *<b>input</b> */
int
buf_set_to_copy(buf_t **output,
                const buf_t *input)
{
  if (*output)
    buf_free(*output);
  *output = buf_copy(input);
  return 0;
}

/** Log an error and exit if <b>buf</b> is corrupted.
 */
void
buf_assert_ok(buf_t *buf)
{
  tor_assert(buf);
  tor_assert(buf->magic == BUFFER_MAGIC);

  if (! buf->head) {
    tor_assert(!buf->tail);
    tor_assert(buf->datalen == 0);
  } else {
    chunk_t *ch;
    size_t total = 0;
    tor_assert(buf->tail);
    for (ch = buf->head; ch; ch = ch->next) {
      total += ch->datalen;
      tor_assert(ch->datalen <= ch->memlen);
      tor_assert(ch->data >= &ch->mem[0]);
      tor_assert(ch->data <= &ch->mem[0]+ch->memlen);
      if (ch->data == &ch->mem[0]+ch->memlen) {
        /* LCOV_EXCL_START */
        static int warned = 0;
        if (! warned) {
          log_warn(LD_BUG, "Invariant violation in buf.c related to #15083");
          warned = 1;
        }
        /* LCOV_EXCL_STOP */
      }
      tor_assert(ch->data+ch->datalen <= &ch->mem[0] + ch->memlen);
      if (!ch->next)
        tor_assert(ch == buf->tail);
    }
    tor_assert(buf->datalen == total);
  }
}

