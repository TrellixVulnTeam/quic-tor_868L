 /* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file connection.c
 * \brief General high-level functions to handle reading and writing
 * on connections.
 *
 * Each connection (ideally) represents a TLS connection, a TCP socket, a unix
 * socket, or a UDP socket on which reads and writes can occur.  (But see
 * connection_edge.c for cases where connections can also represent streams
 * that do not have a corresponding socket.)
 *
 * The module implements the abstract type, connection_t.  The subtypes are:
 *  <ul>
 *   <li>listener_connection_t, implemented here in connection.c
 *   <li>dir_connection_t, implemented in directory.c
 *   <li>or_connection_t, implemented in connection_or.c
 *   <li>edge_connection_t, implemented in connection_edge.c, along with
 *      its subtype(s):
 *      <ul><li>entry_connection_t, also implemented in connection_edge.c
 *      </ul>
 *   <li>control_connection_t, implemented in control.c
 *  </ul>
 *
 * The base type implemented in this module is responsible for basic
 * rate limiting, flow control, and marshalling bytes onto and off of the
 * network (either directly or via TLS).
 *
 * Connections are registered with the main loop with connection_add(). As
 * they become able to read or write register the fact with the event main
 * loop by calling connection_watch_events(), connection_start_reading(), or
 * connection_start_writing().  When they no longer want to read or write,
 * they call connection_stop_reading() or connection_stop_writing().
 *
 * To queue data to be written on a connection, call
 * connection_buf_add().  When data arrives, the
 * connection_process_inbuf() callback is invoked, which dispatches to a
 * type-specific function (such as connection_edge_process_inbuf() for
 * example). Connection types that need notice of when data has been written
 * receive notification via connection_flushed_some() and
 * connection_finished_flushing().  These functions all delegate to
 * type-specific implementations.
 *
 * Additionally, beyond the core of connection_t, this module also implements:
 * <ul>
 * <li>Listeners, which wait for incoming sockets and launch connections
 * <li>Outgoing SOCKS proxy support
 * <li>Outgoing HTTP proxy support
 * <li>An out-of-sockets handler for dealing with socket exhaustion
 * </ul>
 **/

#define CONNECTION_PRIVATE
#include "or.h"
#include "bridges.h"
#include "buffers.h"
#include "buffers_tls.h"
/*
 * Define this so we get channel internal functions, since we're implementing
 * part of a subclass (channel_tls_t).
 */
#define TOR_CHANNEL_INTERNAL_
#define CONNECTION_PRIVATE
#include "backtrace.h"
#include "channel.h"
#include "channeltls.h"
#include "circuitbuild.h"
#include "circuitlist.h"
#include "circuituse.h"
#include "config.h"
#include "connection.h"
#include "connection_edge.h"
#include "connection_or.h"
#include "control.h"
#include "directory.h"
#include "dirserv.h"
#include "dns.h"
#include "dnsserv.h"
#include "dos.h"
#include "entrynodes.h"
#include "ext_orport.h"
#include "geoip.h"
#include "main.h"
#include "hs_common.h"
#include "hs_ident.h"
#include "nodelist.h"
#include "proto_http.h"
#include "proto_socks.h"
#include "policies.h"
#include "reasons.h"
#include "relay.h"
#include "rendclient.h"
#include "rendcommon.h"
#include "rephist.h"
#include "router.h"
#include "routerlist.h"
#include "transports.h"
#include "routerparse.h"
#include "sandbox.h"

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_SYS_UN_H
#include <sys/socket.h>
#include <sys/un.h>
#endif

static connection_t *connection_listener_new(
                               const struct sockaddr *listensockaddr,
                               socklen_t listensocklen, int type,
                               const char *address,
                               const port_cfg_t *portcfg);
static void connection_init(time_t now, connection_t *conn, int type,
                            int socket_family);
static int connection_handle_listener_read(connection_t *conn, int new_type);
static int connection_finished_flushing(connection_t *conn);
static int connection_flushed_some(connection_t *conn);
static int connection_finished_connecting(connection_t *conn);
static int connection_reached_eof(connection_t *conn);
static int connection_buf_read_from_socket(connection_t *conn,
                                           ssize_t *max_to_read,
                                           int *socket_error);
static int connection_read_to_buf_quic(connection_t *conn, ssize_t *max_to_read,
                                  int *socket_error);
static int connection_process_inbuf(connection_t *conn, int package_partial);
static void client_check_address_changed(tor_socket_t sock);
static void set_constrained_socket_buffers(tor_socket_t sock, int size);

static const char *connection_proxy_state_to_string(int state);
static int connection_read_https_proxy_response(connection_t *conn);
static void connection_send_socks5_connect(connection_t *conn);
static const char *proxy_type_to_string(int proxy_type);
static int get_proxy_type(void);
const tor_addr_t *conn_get_outbound_address(sa_family_t family,
                  const or_options_t *options, unsigned int conn_type);

/** The last addresses that our network interface seemed to have been
 * binding to.  We use this as one way to detect when our IP changes.
 *
 * XXXX+ We should really use the entire list of interfaces here.
 **/
static tor_addr_t *last_interface_ipv4 = NULL;
/* DOCDOC last_interface_ipv6 */
static tor_addr_t *last_interface_ipv6 = NULL;
/** A list of tor_addr_t for addresses we've used in outgoing connections.
 * Used to detect IP address changes. */
static smartlist_t *outgoing_addrs = NULL;

#define CASE_ANY_LISTENER_TYPE \
    case CONN_TYPE_OR_LISTENER: \
    case CONN_TYPE_EXT_OR_LISTENER: \
    case CONN_TYPE_AP_LISTENER: \
    case CONN_TYPE_DIR_LISTENER: \
    case CONN_TYPE_CONTROL_LISTENER: \
    case CONN_TYPE_AP_TRANS_LISTENER: \
    case CONN_TYPE_AP_NATD_LISTENER: \
    case CONN_TYPE_AP_DNS_LISTENER: \
    case CONN_TYPE_AP_HTTP_CONNECT_LISTENER

#define CASE_NOQUIC_LISTENER_TYPE \
    case CONN_TYPE_DIR_LISTENER: \
    case CONN_TYPE_CONTROL_LISTENER: \
    case CONN_TYPE_AP_TRANS_LISTENER: \
    case CONN_TYPE_AP_NATD_LISTENER: \
    case CONN_TYPE_AP_DNS_LISTENER: \
    case CONN_TYPE_AP_LISTENER: \
    case CONN_TYPE_EXT_OR_LISTENER: \
	case CONN_TYPE_AP_HTTP_CONNECT_LISTENER


#define CASE_QUIC_LISTENER_TYPE \
    case CONN_TYPE_OR_LISTENER

/**************************************************************/

/**
 * Return the human-readable name for the connection type <b>type</b>
 */
const char *
conn_type_to_string(int type)
{
  static char buf[64];
  switch (type) {
    case CONN_TYPE_OR_LISTENER: return "OR listener";
    case CONN_TYPE_OR: return "OR";
    case CONN_TYPE_EXIT: return "Exit";
    case CONN_TYPE_AP_LISTENER: return "Socks listener";
    case CONN_TYPE_AP_TRANS_LISTENER:
      return "Transparent pf/netfilter listener";
    case CONN_TYPE_AP_NATD_LISTENER: return "Transparent natd listener";
    case CONN_TYPE_AP_DNS_LISTENER: return "DNS listener";
    case CONN_TYPE_AP: return "Socks";
    case CONN_TYPE_DIR_LISTENER: return "Directory listener";
    case CONN_TYPE_DIR: return "Directory";
    case CONN_TYPE_CONTROL_LISTENER: return "Control listener";
    case CONN_TYPE_CONTROL: return "Control";
    case CONN_TYPE_EXT_OR: return "Extended OR";
    case CONN_TYPE_EXT_OR_LISTENER: return "Extended OR listener";
    case CONN_TYPE_AP_HTTP_CONNECT_LISTENER: return "HTTP tunnel listener";
    default:
      log_warn(LD_BUG, "unknown connection type %d", type);
      tor_snprintf(buf, sizeof(buf), "unknown [%d]", type);
      return buf;
  }
}

/**
 * Return the human-readable name for the connection state <b>state</b>
 * for the connection type <b>type</b>
 */
const char *
conn_state_to_string(int type, int state)
{
  static char buf[96];
  switch (type) {
    CASE_ANY_LISTENER_TYPE:
      if (state == LISTENER_STATE_READY)
        return "ready";
      break;
    case CONN_TYPE_OR:
      switch (state) {
        case OR_CONN_STATE_CONNECTING: return "connect()ing";
        case OR_CONN_STATE_PROXY_HANDSHAKING: return "handshaking (proxy)";
        case OR_CONN_STATE_TLS_HANDSHAKING: return "handshaking (TLS)";
        case OR_CONN_STATE_TLS_CLIENT_RENEGOTIATING:
          return "renegotiating (TLS, v2 handshake)";
        case OR_CONN_STATE_TLS_SERVER_RENEGOTIATING:
          return "waiting for renegotiation or V3 handshake";
        case OR_CONN_STATE_OR_HANDSHAKING_V2:
          return "handshaking (Tor, v2 handshake)";
        case OR_CONN_STATE_OR_HANDSHAKING_V3:
          return "handshaking (Tor, v3 handshake)";
        case OR_CONN_STATE_OPEN: return "open";
      }
      break;
    case CONN_TYPE_EXT_OR:
      switch (state) {
        case EXT_OR_CONN_STATE_AUTH_WAIT_AUTH_TYPE:
          return "waiting for authentication type";
        case EXT_OR_CONN_STATE_AUTH_WAIT_CLIENT_NONCE:
          return "waiting for client nonce";
        case EXT_OR_CONN_STATE_AUTH_WAIT_CLIENT_HASH:
          return "waiting for client hash";
        case EXT_OR_CONN_STATE_OPEN: return "open";
        case EXT_OR_CONN_STATE_FLUSHING: return "flushing final OKAY";
      }
      break;
    case CONN_TYPE_EXIT:
      switch (state) {
        case EXIT_CONN_STATE_RESOLVING: return "waiting for dest info";
        case EXIT_CONN_STATE_CONNECTING: return "connecting";
        case EXIT_CONN_STATE_OPEN: return "open";
        case EXIT_CONN_STATE_RESOLVEFAILED: return "resolve failed";
      }
      break;
    case CONN_TYPE_AP:
      switch (state) {
        case AP_CONN_STATE_SOCKS_WAIT: return "waiting for socks info";
        case AP_CONN_STATE_NATD_WAIT: return "waiting for natd dest info";
        case AP_CONN_STATE_RENDDESC_WAIT: return "waiting for rendezvous desc";
        case AP_CONN_STATE_CONTROLLER_WAIT: return "waiting for controller";
        case AP_CONN_STATE_CIRCUIT_WAIT: return "waiting for circuit";
        case AP_CONN_STATE_CONNECT_WAIT: return "waiting for connect response";
        case AP_CONN_STATE_RESOLVE_WAIT: return "waiting for resolve response";
        case AP_CONN_STATE_OPEN: return "open";
      }
      break;
    case CONN_TYPE_DIR:
      switch (state) {
        case DIR_CONN_STATE_CONNECTING: return "connecting";
        case DIR_CONN_STATE_CLIENT_SENDING: return "client sending";
        case DIR_CONN_STATE_CLIENT_READING: return "client reading";
        case DIR_CONN_STATE_CLIENT_FINISHED: return "client finished";
        case DIR_CONN_STATE_SERVER_COMMAND_WAIT: return "waiting for command";
        case DIR_CONN_STATE_SERVER_WRITING: return "writing";
      }
      break;
    case CONN_TYPE_CONTROL:
      switch (state) {
        case CONTROL_CONN_STATE_OPEN: return "open (protocol v1)";
        case CONTROL_CONN_STATE_NEEDAUTH:
          return "waiting for authentication (protocol v1)";
      }
      break;
  }

  log_warn(LD_BUG, "unknown connection state %d (type %d)", state, type);
  tor_snprintf(buf, sizeof(buf),
               "unknown state [%d] on unknown [%s] connection",
               state, conn_type_to_string(type));
  return buf;
}

/** Allocate and return a new dir_connection_t, initialized as by
 * connection_init(). */
dir_connection_t *
dir_connection_new(int socket_family)
{
  dir_connection_t *dir_conn = tor_malloc_zero(sizeof(dir_connection_t));
  connection_init(time(NULL), TO_CONN(dir_conn), CONN_TYPE_DIR, socket_family);
  return dir_conn;
}

/** Allocate and return a new or_connection_t, initialized as by
 * connection_init().
 *
 * Initialize active_circuit_pqueue.
 *
 * Set active_circuit_pqueue_last_recalibrated to current cell_ewma tick.
 */
or_connection_t *
or_connection_new(int type, int socket_family)
{
  or_connection_t *or_conn = tor_malloc_zero(sizeof(or_connection_t));
  time_t now = time(NULL);
  tor_assert(type == CONN_TYPE_OR || type == CONN_TYPE_EXT_OR);
  connection_init(now, TO_CONN(or_conn), type, socket_family);

  connection_or_set_canonical(or_conn, 0);

  if (type == CONN_TYPE_EXT_OR)
    connection_or_set_ext_or_identifier(or_conn);

  return or_conn;
}

/** Allocate and return a new entry_connection_t, initialized as by
 * connection_init().
 *
 * Allocate space to store the socks_request.
 */
entry_connection_t *
entry_connection_new(int type, int socket_family)
{
  entry_connection_t *entry_conn = tor_malloc_zero(sizeof(entry_connection_t));
  tor_assert(type == CONN_TYPE_AP);
  connection_init(time(NULL), ENTRY_TO_CONN(entry_conn), type, socket_family);
  entry_conn->socks_request = socks_request_new();
  /* If this is coming from a listener, we'll set it up based on the listener
   * in a little while.  Otherwise, we're doing this as a linked connection
   * of some kind, and we should set it up here based on the socket family */
  if (socket_family == AF_INET)
    entry_conn->entry_cfg.ipv4_traffic = 1;
  else if (socket_family == AF_INET6)
    entry_conn->entry_cfg.ipv6_traffic = 1;
  return entry_conn;
}

/** Allocate and return a new edge_connection_t, initialized as by
 * connection_init(). */
edge_connection_t *
edge_connection_new(int type, int socket_family)
{
  edge_connection_t *edge_conn = tor_malloc_zero(sizeof(edge_connection_t));
  tor_assert(type == CONN_TYPE_EXIT);
  connection_init(time(NULL), TO_CONN(edge_conn), type, socket_family);
  return edge_conn;
}

/** Allocate and return a new control_connection_t, initialized as by
 * connection_init(). */
control_connection_t *
control_connection_new(int socket_family)
{
  control_connection_t *control_conn =
    tor_malloc_zero(sizeof(control_connection_t));
  connection_init(time(NULL),
                  TO_CONN(control_conn), CONN_TYPE_CONTROL, socket_family);
  return control_conn;
}

/** Allocate and return a new listener_connection_t, initialized as by
 * connection_init(). */
listener_connection_t *
listener_connection_new(int type, int socket_family)
{
  listener_connection_t *listener_conn =
    tor_malloc_zero(sizeof(listener_connection_t));
  connection_init(time(NULL), TO_CONN(listener_conn), type, socket_family);
  return listener_conn;
}

/** Allocate, initialize, and return a new connection_t subtype of <b>type</b>
 * to make or receive connections of address family <b>socket_family</b>.  The
 * type should be one of the CONN_TYPE_* constants. */
connection_t *
connection_new(int type, int socket_family)
{
  switch (type) {
    case CONN_TYPE_OR:
    case CONN_TYPE_EXT_OR:
      return TO_CONN(or_connection_new(type, socket_family));

    case CONN_TYPE_EXIT:
      return TO_CONN(edge_connection_new(type, socket_family));

    case CONN_TYPE_AP:
      return ENTRY_TO_CONN(entry_connection_new(type, socket_family));

    case CONN_TYPE_DIR:
      return TO_CONN(dir_connection_new(socket_family));

    case CONN_TYPE_CONTROL:
      return TO_CONN(control_connection_new(socket_family));

    CASE_ANY_LISTENER_TYPE:
      return TO_CONN(listener_connection_new(type, socket_family));

    default: {
      connection_t *conn = tor_malloc_zero(sizeof(connection_t));
      connection_init(time(NULL), conn, type, socket_family);
      return conn;
    }
  }
}

/** Initializes conn. (you must call connection_add() to link it into the main
 * array).
 *
 * Set conn-\>magic to the correct value.
 *
 * Set conn-\>type to <b>type</b>. Set conn-\>s and conn-\>conn_array_index to
 * -1 to signify they are not yet assigned.
 *
 * Initialize conn's timestamps to now.
 */
static void
connection_init(time_t now, connection_t *conn, int type, int socket_family)
{
  static uint64_t n_connections_allocated = 1;
  conn->use_quic = 0;
  conn->q_sock = NULL;

  switch (type) {
    case CONN_TYPE_OR:
    case CONN_TYPE_EXT_OR:
      conn->magic = OR_CONNECTION_MAGIC;
      break;
    case CONN_TYPE_EXIT:
      conn->magic = EDGE_CONNECTION_MAGIC;
      break;
    case CONN_TYPE_AP:
      conn->magic = ENTRY_CONNECTION_MAGIC;
      break;
    case CONN_TYPE_DIR:
      conn->magic = DIR_CONNECTION_MAGIC;
      break;
    case CONN_TYPE_CONTROL:
      conn->magic = CONTROL_CONNECTION_MAGIC;
		break;
//	CASE_ANY_LISTENER_TYPE:
//		conn->magic = LISTENER_CONNECTION_MAGIC;
//		break;
	CASE_QUIC_LISTENER_TYPE:
		conn->magic = LISTENER_CONNECTION_MAGIC;
		break;
	CASE_NOQUIC_LISTENER_TYPE:
		conn->magic = LISTENER_CONNECTION_MAGIC;
		break;
	default:
		conn->magic = BASE_CONNECTION_MAGIC;
		break;
  }

  conn->s = TOR_INVALID_SOCKET; /* give it a default of 'not used' */
  conn->conn_array_index = -1; /* also default to 'not used' */
  conn->global_identifier = n_connections_allocated++;

  conn->type = type;
  conn->socket_family = socket_family;
  if (!connection_is_listener(conn)) {
    /* listeners never use their buf */
    conn->inbuf = buf_new();
    conn->outbuf = buf_new();
  }

  conn->timestamp_created = now;
  conn->timestamp_last_read_allowed = now;
  conn->timestamp_last_write_allowed = now;
}

/** Create a link between <b>conn_a</b> and <b>conn_b</b>. */
void
connection_link_connections(connection_t *conn_a, connection_t *conn_b)
{
  tor_assert(! SOCKET_OK(conn_a->s));
  tor_assert(! SOCKET_OK(conn_b->s));

  conn_a->linked = 1;
  conn_b->linked = 1;
  conn_a->linked_conn = conn_b;
  conn_b->linked_conn = conn_a;
}

/** Return true iff the provided connection listener type supports AF_UNIX
 * sockets. */
int
conn_listener_type_supports_af_unix(int type)
{
  /* For now only control ports or SOCKS ports can be Unix domain sockets
   * and listeners at the same time */
  switch (type) {
    case CONN_TYPE_CONTROL_LISTENER:
    case CONN_TYPE_AP_LISTENER:
      return 1;
    default:
      return 0;
  }
}

/** Deallocate memory used by <b>conn</b>. Deallocate its buffers if
 * necessary, close its socket if necessary, and mark the directory as dirty
 * if <b>conn</b> is an OR or OP connection.
 */
STATIC void
connection_free_minimal(connection_t *conn)
{
  void *mem;
  size_t memlen;
  if (!conn)
    return;

  switch (conn->type) {
    case CONN_TYPE_OR:
    case CONN_TYPE_EXT_OR:
      tor_assert(conn->magic == OR_CONNECTION_MAGIC);
      mem = TO_OR_CONN(conn);
      memlen = sizeof(or_connection_t);
      break;
    case CONN_TYPE_AP:
      tor_assert(conn->magic == ENTRY_CONNECTION_MAGIC);
      mem = TO_ENTRY_CONN(conn);
      memlen = sizeof(entry_connection_t);
      break;
    case CONN_TYPE_EXIT:
      tor_assert(conn->magic == EDGE_CONNECTION_MAGIC);
      mem = TO_EDGE_CONN(conn);
      memlen = sizeof(edge_connection_t);
      break;
    case CONN_TYPE_DIR:
      tor_assert(conn->magic == DIR_CONNECTION_MAGIC);
      mem = TO_DIR_CONN(conn);
      memlen = sizeof(dir_connection_t);
      break;
    case CONN_TYPE_CONTROL:
      tor_assert(conn->magic == CONTROL_CONNECTION_MAGIC);
      mem = TO_CONTROL_CONN(conn);
      memlen = sizeof(control_connection_t);
      break;
    CASE_ANY_LISTENER_TYPE:
      tor_assert(conn->magic == LISTENER_CONNECTION_MAGIC);
      mem = TO_LISTENER_CONN(conn);
      memlen = sizeof(listener_connection_t);
      break;
    default:
      tor_assert(conn->magic == BASE_CONNECTION_MAGIC);
      mem = conn;
      memlen = sizeof(connection_t);
      break;
  }

  if (conn->linked) {
    log_info(LD_GENERAL, "Freeing linked %s connection [%s] with %d "
             "bytes on inbuf, %d on outbuf.",
             conn_type_to_string(conn->type),
             conn_state_to_string(conn->type, conn->state),
             (int)connection_get_inbuf_len(conn),
             (int)connection_get_outbuf_len(conn));
  }

  if (!connection_is_listener(conn)) {
    buf_free(conn->inbuf);
    buf_free(conn->outbuf);
  } else {
    if (conn->socket_family == AF_UNIX) {
      /* For now only control and SOCKS ports can be Unix domain sockets
       * and listeners at the same time */
      tor_assert(conn_listener_type_supports_af_unix(conn->type));

      if (unlink(conn->address) < 0 && errno != ENOENT) {
        log_warn(LD_NET, "Could not unlink %s: %s", conn->address,
                         strerror(errno));
      }
    }
  }

  tor_free(conn->address);

  if (connection_speaks_cells(conn)) {
    or_connection_t *or_conn = TO_OR_CONN(conn);
    tor_tls_free(or_conn->tls);
    or_conn->tls = NULL;
    or_handshake_state_free(or_conn->handshake_state);
    or_conn->handshake_state = NULL;
    tor_free(or_conn->nickname);
    if (or_conn->chan) {
      /* Owww, this shouldn't happen, but... */
      log_info(LD_CHANNEL,
               "Freeing orconn at %p, saw channel %p with ID "
               U64_FORMAT " left un-NULLed",
               or_conn, TLS_CHAN_TO_BASE(or_conn->chan),
               U64_PRINTF_ARG(
                 TLS_CHAN_TO_BASE(or_conn->chan)->global_identifier));
      if (!CHANNEL_FINISHED(TLS_CHAN_TO_BASE(or_conn->chan))) {
        channel_close_for_error(TLS_CHAN_TO_BASE(or_conn->chan));
      }

      or_conn->chan->conn = NULL;
      or_conn->chan = NULL;
    }
  }
  if (conn->type == CONN_TYPE_AP) {
    entry_connection_t *entry_conn = TO_ENTRY_CONN(conn);
    tor_free(entry_conn->chosen_exit_name);
    tor_free(entry_conn->original_dest_address);
    if (entry_conn->socks_request)
      socks_request_free(entry_conn->socks_request);
    if (entry_conn->pending_optimistic_data) {
      buf_free(entry_conn->pending_optimistic_data);
    }
    if (entry_conn->sending_optimistic_data) {
      buf_free(entry_conn->sending_optimistic_data);
    }
  }
  if (CONN_IS_EDGE(conn)) {
    rend_data_free(TO_EDGE_CONN(conn)->rend_data);
    hs_ident_edge_conn_free(TO_EDGE_CONN(conn)->hs_ident);
  }
  if (conn->type == CONN_TYPE_CONTROL) {
    control_connection_t *control_conn = TO_CONTROL_CONN(conn);
    tor_free(control_conn->safecookie_client_hash);
    tor_free(control_conn->incoming_cmd);
    if (control_conn->ephemeral_onion_services) {
      SMARTLIST_FOREACH(control_conn->ephemeral_onion_services, char *, cp, {
        memwipe(cp, 0, strlen(cp));
        tor_free(cp);
      });
      smartlist_free(control_conn->ephemeral_onion_services);
    }
  }

  /* Probably already freed by connection_free. */
  tor_event_free(conn->read_event);
  tor_event_free(conn->write_event);
  conn->read_event = conn->write_event = NULL;

  if (conn->type == CONN_TYPE_DIR) {
    dir_connection_t *dir_conn = TO_DIR_CONN(conn);
    tor_free(dir_conn->requested_resource);

    tor_compress_free(dir_conn->compress_state);
    if (dir_conn->spool) {
      SMARTLIST_FOREACH(dir_conn->spool, spooled_resource_t *, spooled,
                        spooled_resource_free(spooled));
      smartlist_free(dir_conn->spool);
    }

    rend_data_free(dir_conn->rend_data);
    hs_ident_dir_conn_free(dir_conn->hs_ident);
    if (dir_conn->guard_state) {
      /* Cancel before freeing, if it's still there. */
      entry_guard_cancel(&dir_conn->guard_state);
    }
    circuit_guard_state_free(dir_conn->guard_state);
  }

  if (SOCKET_OK(conn->s)) {
    log_debug(LD_NET,"closing fd %d.",(int)conn->s);
    tor_close_socket(conn->s);
    conn->s = TOR_INVALID_SOCKET;
  }

  if (conn->type == CONN_TYPE_OR &&
      !tor_digest_is_zero(TO_OR_CONN(conn)->identity_digest)) {
    log_warn(LD_BUG, "called on OR conn with non-zeroed identity_digest");
    connection_or_clear_identity(TO_OR_CONN(conn));
  }
  if (conn->type == CONN_TYPE_OR || conn->type == CONN_TYPE_EXT_OR) {
    connection_or_remove_from_ext_or_id_map(TO_OR_CONN(conn));
    tor_free(TO_OR_CONN(conn)->ext_or_conn_id);
    tor_free(TO_OR_CONN(conn)->ext_or_auth_correct_client_hash);
    tor_free(TO_OR_CONN(conn)->ext_or_transport);
  }

  memwipe(mem, 0xCC, memlen); /* poison memory */
  tor_free(mem);
}

/** Make sure <b>conn</b> isn't in any of the global conn lists; then free it.
 */
MOCK_IMPL(void,
connection_free_,(connection_t *conn))
{
  if (!conn)
    return;
  tor_assert(!connection_is_on_closeable_list(conn));
  tor_assert(!connection_in_array(conn));
  if (BUG(conn->linked_conn)) {
    conn->linked_conn->linked_conn = NULL;
    if (! conn->linked_conn->marked_for_close &&
        conn->linked_conn->reading_from_linked_conn)
      connection_start_reading(conn->linked_conn);
    conn->linked_conn = NULL;
  }
  if (connection_speaks_cells(conn)) {
    if (!tor_digest_is_zero(TO_OR_CONN(conn)->identity_digest)) {
      connection_or_clear_identity(TO_OR_CONN(conn));
    }
  }
  if (conn->type == CONN_TYPE_CONTROL) {
    connection_control_closed(TO_CONTROL_CONN(conn));
  }
#if 1
  /* DEBUGGING */
  if (conn->type == CONN_TYPE_AP) {
    connection_ap_warn_and_unmark_if_pending_circ(TO_ENTRY_CONN(conn),
                                                  "connection_free");
  }
#endif /* 1 */

  /* Notify the circuit creation DoS mitigation subsystem that an OR client
   * connection has been closed. And only do that if we track it. */
  if (conn->type == CONN_TYPE_OR) {
    dos_close_client_conn(TO_OR_CONN(conn));
  }

  connection_unregister_events(conn);
  connection_free_minimal(conn);
}

/**
 * Called when we're about to finally unlink and free a connection:
 * perform necessary accounting and cleanup
 *   - Directory conns that failed to fetch a rendezvous descriptor
 *     need to inform pending rendezvous streams.
 *   - OR conns need to call rep_hist_note_*() to record status.
 *   - AP conns need to send a socks reject if necessary.
 *   - Exit conns need to call connection_dns_remove() if necessary.
 *   - AP and Exit conns need to send an end cell if they can.
 *   - DNS conns need to fail any resolves that are pending on them.
 *   - OR and edge connections need to be unlinked from circuits.
 */
void
connection_about_to_close_connection(connection_t *conn)
{
  tor_assert(conn->marked_for_close);

  switch (conn->type) {
    case CONN_TYPE_DIR:
      connection_dir_about_to_close(TO_DIR_CONN(conn));
      break;
    case CONN_TYPE_OR:
    case CONN_TYPE_EXT_OR:
      connection_or_about_to_close(TO_OR_CONN(conn));
      break;
    case CONN_TYPE_AP:
      connection_ap_about_to_close(TO_ENTRY_CONN(conn));
      break;
    case CONN_TYPE_EXIT:
      connection_exit_about_to_close(TO_EDGE_CONN(conn));
      break;
  }
}

/** Return true iff connection_close_immediate() has been called on this
 * connection. */
#define CONN_IS_CLOSED(c) \
  ((c)->linked ? ((c)->linked_conn_is_closed) : (! SOCKET_OK(c->s)))

/** Close the underlying socket for <b>conn</b>, so we don't try to
 * flush it. Must be used in conjunction with (right before)
 * connection_mark_for_close().
 */
void
connection_close_immediate(connection_t *conn)
{
  assert_connection_ok(conn,0);
  if (CONN_IS_CLOSED(conn)&& !(QUICSOCK_OK(conn->q_sock))) {
    log_err(LD_BUG,"Attempt to close already-closed connection.");
    tor_fragile_assert();
    return;
  }
  if (conn->outbuf_flushlen) {
    log_info(LD_NET,"fd %d, type %s, state %s, %d bytes on outbuf.",
             (int)conn->s, conn_type_to_string(conn->type),
             conn_state_to_string(conn->type, conn->state),
             (int)conn->outbuf_flushlen);
  }

  connection_unregister_events(conn);

  /* Prevent the event from getting unblocked. */
  conn->read_blocked_on_bw =
    conn->write_blocked_on_bw = 0;

  if (SOCKET_OK(conn->s))
    tor_close_socket(conn->s);
  conn->s = TOR_INVALID_SOCKET;
  if (conn->linked)
    conn->linked_conn_is_closed = 1;
  if (conn->outbuf)
    buf_clear(conn->outbuf);
  conn->outbuf_flushlen = 0;
}

/** Mark <b>conn</b> to be closed next time we loop through
 * conn_close_if_marked() in main.c. */
void
connection_mark_for_close_(connection_t *conn, int line, const char *file)
{
  assert_connection_ok(conn,0);
  tor_assert(line);
  tor_assert(line < 1<<16); /* marked_for_close can only fit a uint16_t. */
  tor_assert(file);

  if (conn->type == CONN_TYPE_OR) {
    /*
     * An or_connection should have been closed through one of the channel-
     * aware functions in connection_or.c.  We'll assume this is an error
     * close and do that, and log a bug warning.
     */
    log_warn(LD_CHANNEL | LD_BUG,
             "Something tried to close an or_connection_t without going "
             "through channels at %s:%d",
             file, line);
    connection_or_close_for_error(TO_OR_CONN(conn), 0);
  } else {
    /* Pass it down to the real function */
    connection_mark_for_close_internal_(conn, line, file);
  }
}

/** Mark <b>conn</b> to be closed next time we loop through
 * conn_close_if_marked() in main.c; the _internal version bypasses the
 * CONN_TYPE_OR checks; this should be called when you either are sure that
 * if this is an or_connection_t the controlling channel has been notified
 * (e.g. with connection_or_notify_error()), or you actually are the
 * connection_or_close_for_error() or connection_or_close_normally() function.
 * For all other cases, use connection_mark_and_flush() instead, which
 * checks for or_connection_t properly, instead.  See below.
 */
MOCK_IMPL(void,
connection_mark_for_close_internal_, (connection_t *conn,
                                      int line, const char *file))
{
  assert_connection_ok(conn,0);
  tor_assert(line);
  tor_assert(line < 1<<16); /* marked_for_close can only fit a uint16_t. */
  tor_assert(file);

  if (conn->marked_for_close) {
    log_warn(LD_BUG,"Duplicate call to connection_mark_for_close at %s:%d"
        " (first at %s:%d)", file, line, conn->marked_for_close_file,
        conn->marked_for_close);
    tor_fragile_assert();
    return;
  }

  if (conn->type == CONN_TYPE_OR) {
    /*
     * Bad news if this happens without telling the controlling channel; do
     * this so we can find things that call this wrongly when the asserts hit.
     */
    log_debug(LD_CHANNEL,
              "Calling connection_mark_for_close_internal_() on an OR conn "
              "at %s:%d",
              file, line);
  }

  conn->marked_for_close = line;
  conn->marked_for_close_file = file;
  add_connection_to_closeable_list(conn);

  /* in case we're going to be held-open-til-flushed, reset
   * the number of seconds since last successful write, so
   * we get our whole 15 seconds */
  conn->timestamp_last_write_allowed = time(NULL);
}

/** Find each connection that has hold_open_until_flushed set to
 * 1 but hasn't written in the past 15 seconds, and set
 * hold_open_until_flushed to 0. This means it will get cleaned
 * up in the next loop through close_if_marked() in main.c.
 */
void
connection_expire_held_open(void)
{
  time_t now;
  smartlist_t *conns = get_connection_array();

  now = time(NULL);

  SMARTLIST_FOREACH_BEGIN(conns, connection_t *, conn) {
    /* If we've been holding the connection open, but we haven't written
     * for 15 seconds...
     */
    if (conn->hold_open_until_flushed) {
      tor_assert(conn->marked_for_close);
      if (now - conn->timestamp_last_write_allowed >= 15) {
        int severity;
        if (conn->type == CONN_TYPE_EXIT ||
            (conn->type == CONN_TYPE_DIR &&
             conn->purpose == DIR_PURPOSE_SERVER))
          severity = LOG_INFO;
        else
          severity = LOG_NOTICE;
        log_fn(severity, LD_NET,
               "Giving up on marked_for_close conn that's been flushing "
               "for 15s (fd %d, type %s, state %s).",
               (int)conn->s, conn_type_to_string(conn->type),
               conn_state_to_string(conn->type, conn->state));
        conn->hold_open_until_flushed = 0;
      }
    }
  } SMARTLIST_FOREACH_END(conn);
}

#if defined(HAVE_SYS_UN_H) || defined(RUNNING_DOXYGEN)
/** Create an AF_UNIX listenaddr struct.
 * <b>listenaddress</b> provides the path to the Unix socket.
 *
 * Eventually <b>listenaddress</b> will also optionally contain user, group,
 * and file permissions for the new socket.  But not yet. XXX
 * Also, since we do not create the socket here the information doesn't help
 * here.
 *
 * If not NULL <b>readable_address</b> will contain a copy of the path part of
 * <b>listenaddress</b>.
 *
 * The listenaddr struct has to be freed by the caller.
 */
static struct sockaddr_un *
create_unix_sockaddr(const char *listenaddress, char **readable_address,
                     socklen_t *len_out)
{
  struct sockaddr_un *sockaddr = NULL;

  sockaddr = tor_malloc_zero(sizeof(struct sockaddr_un));
  sockaddr->sun_family = AF_UNIX;
  if (strlcpy(sockaddr->sun_path, listenaddress, sizeof(sockaddr->sun_path))
      >= sizeof(sockaddr->sun_path)) {
    log_warn(LD_CONFIG, "Unix socket path '%s' is too long to fit.",
             escaped(listenaddress));
    tor_free(sockaddr);
    return NULL;
  }

  if (readable_address)
    *readable_address = tor_strdup(listenaddress);

  *len_out = sizeof(struct sockaddr_un);
  return sockaddr;
}
#else /* !(defined(HAVE_SYS_UN_H) || defined(RUNNING_DOXYGEN)) */
static struct sockaddr *
create_unix_sockaddr(const char *listenaddress, char **readable_address,
                     socklen_t *len_out)
{
  (void)listenaddress;
  (void)readable_address;
  log_fn(LOG_ERR, LD_BUG,
         "Unix domain sockets not supported, yet we tried to create one.");
  *len_out = 0;
  tor_fragile_assert();
  return NULL;
}
#endif /* defined(HAVE_SYS_UN_H) || defined(RUNNING_DOXYGEN) */

/** Warn that an accept or a connect has failed because we're running out of
 * TCP sockets we can use on current system.  Rate-limit these warnings so
 * that we don't spam the log. */
static void
warn_too_many_conns(void)
{
#define WARN_TOO_MANY_CONNS_INTERVAL (6*60*60)
  static ratelim_t last_warned = RATELIM_INIT(WARN_TOO_MANY_CONNS_INTERVAL);
  char *m;
  if ((m = rate_limit_log(&last_warned, approx_time()))) {
    int n_conns = get_n_open_sockets();
    log_warn(LD_NET,"Failing because we have %d connections already. Please "
             "read doc/TUNING for guidance.%s", n_conns, m);
    tor_free(m);
    control_event_general_status(LOG_WARN, "TOO_MANY_CONNECTIONS CURRENT=%d",
                                 n_conns);
  }
}

#ifdef HAVE_SYS_UN_H

#define UNIX_SOCKET_PURPOSE_CONTROL_SOCKET 0
#define UNIX_SOCKET_PURPOSE_SOCKS_SOCKET 1

/** Check if the purpose isn't one of the ones we know what to do with */

static int
is_valid_unix_socket_purpose(int purpose)
{
  int valid = 0;

  switch (purpose) {
    case UNIX_SOCKET_PURPOSE_CONTROL_SOCKET:
    case UNIX_SOCKET_PURPOSE_SOCKS_SOCKET:
      valid = 1;
      break;
  }

  return valid;
}

/** Return a string description of a unix socket purpose */
static const char *
unix_socket_purpose_to_string(int purpose)
{
  const char *s = "unknown-purpose socket";

  switch (purpose) {
    case UNIX_SOCKET_PURPOSE_CONTROL_SOCKET:
      s = "control socket";
      break;
    case UNIX_SOCKET_PURPOSE_SOCKS_SOCKET:
      s = "SOCKS socket";
      break;
  }

  return s;
}

/** Check whether we should be willing to open an AF_UNIX socket in
 * <b>path</b>.  Return 0 if we should go ahead and -1 if we shouldn't. */
static int
check_location_for_unix_socket(const or_options_t *options, const char *path,
                               int purpose, const port_cfg_t *port)
{
  int r = -1;
  char *p = NULL;

  tor_assert(is_valid_unix_socket_purpose(purpose));

  p = tor_strdup(path);
  cpd_check_t flags = CPD_CHECK_MODE_ONLY;
  if (get_parent_directory(p)<0 || p[0] != '/') {
    log_warn(LD_GENERAL, "Bad unix socket address '%s'.  Tor does not support "
             "relative paths for unix sockets.", path);
    goto done;
  }

  if (port->is_world_writable) {
    /* World-writable sockets can go anywhere. */
    r = 0;
    goto done;
  }

  if (port->is_group_writable) {
    flags |= CPD_GROUP_OK;
  }

  if (port->relax_dirmode_check) {
    flags |= CPD_RELAX_DIRMODE_CHECK;
  }

  if (check_private_dir(p, flags, options->User) < 0) {
    char *escpath, *escdir;
    escpath = esc_for_log(path);
    escdir = esc_for_log(p);
    log_warn(LD_GENERAL, "Before Tor can create a %s in %s, the directory "
             "%s needs to exist, and to be accessible only by the user%s "
             "account that is running Tor.  (On some Unix systems, anybody "
             "who can list a socket can connect to it, so Tor is being "
             "careful.)",
             unix_socket_purpose_to_string(purpose), escpath, escdir,
             port->is_group_writable ? " and group" : "");
    tor_free(escpath);
    tor_free(escdir);
    goto done;
  }

  r = 0;
 done:
  tor_free(p);
  return r;
}
#endif /* defined(HAVE_SYS_UN_H) */

/** Tell the TCP stack that it shouldn't wait for a long time after
 * <b>sock</b> has closed before reusing its port. Return 0 on success,
 * -1 on failure. */
static int
make_socket_reuseable(tor_socket_t sock)
{
#ifdef _WIN32
  (void) sock;
  return 0;
#else
  int one=1;

  /* REUSEADDR on normal places means you can rebind to the port
   * right after somebody else has let it go. But REUSEADDR on win32
   * means you can bind to the port _even when somebody else
   * already has it bound_. So, don't do that on Win32. */
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &one,
             (socklen_t)sizeof(one)) == -1) {
    return -1;
  }
  return 0;
#endif /* defined(_WIN32) */
}

#ifdef _WIN32
/** Tell the Windows TCP stack to prevent other applications from receiving
 * traffic from tor's open ports. Return 0 on success, -1 on failure. */
static int
make_win32_socket_exclusive(tor_socket_t sock)
{
#ifdef SO_EXCLUSIVEADDRUSE
  int one=1;

  /* Any socket that sets REUSEADDR on win32 can bind to a port _even when
   * somebody else already has it bound_, and _even if the original socket
   * didn't set REUSEADDR_. Use EXCLUSIVEADDRUSE to prevent this port-stealing
   * on win32. */
  if (setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (void*) &one,
                 (socklen_t)sizeof(one))) {
    return -1;
  }
  return 0;
#else /* !(defined(SO_EXCLUSIVEADDRUSE)) */
  (void) sock;
  return 0;
#endif /* defined(SO_EXCLUSIVEADDRUSE) */
}
#endif /* defined(_WIN32) */

/** Max backlog to pass to listen.  We start at */
static int listen_limit = INT_MAX;

/* Listen on <b>fd</b> with appropriate backlog. Return as for listen. */
static int
tor_listen(tor_socket_t fd)
{
  int r;

  if ((r = listen(fd, listen_limit)) < 0) {
    if (listen_limit == SOMAXCONN)
      return r;
    if ((r = listen(fd, SOMAXCONN)) == 0) {
      listen_limit = SOMAXCONN;
      log_warn(LD_NET, "Setting listen backlog to INT_MAX connections "
               "didn't work, but SOMAXCONN did. Lowering backlog limit.");
    }
  }
  return r;
}

/** Bind a new non-blocking socket listening to the socket described
 * by <b>listensockaddr</b>.
 *
 * <b>address</b> is only used for logging purposes and to add the information
 * to the conn.
 */
static connection_t *
connection_listener_new_quic(const struct sockaddr *listensockaddr,
                            socklen_t socklen,
                            int type, const char *address,
                            const port_cfg_t *port_cfg)
{
  listener_connection_t *lis_conn;
  connection_t *conn = NULL;
  int use_quic = 1;
  tor_socket_t s = TOR_INVALID_SOCKET;  /* the socket we're NOT going to make */
  tor_quicsock_t q_sock = TOR_INVALID_QUICSOCK;
  or_options_t const *options = get_options();

#if defined(HAVE_PWD_H) && defined(HAVE_SYS_UN_H)
  const struct passwd *pw = NULL;
#endif
  uint16_t usePort = 0, gotPort = 0;
  static int global_next_session_group = SESSION_GROUP_FIRST_AUTO;
  tor_addr_t addr;

  if (listensockaddr->sa_family == AF_INET ||
      listensockaddr->sa_family == AF_INET6) {

    tor_addr_from_sockaddr(&addr, listensockaddr, &usePort);

    q_sock = qs_open();

    if (!QUICSOCK_OK(q_sock)) {
      int e = tor_socket_errno(s);
      if (ERRNO_IS_RESOURCE_LIMIT(e)) {
        warn_too_many_conns();
      } else {
        log_warn(LD_NET, "QUIC Socket creation failed: %s",
                 tor_socket_strerror(e));
      }
      goto err;
    }

    if (qs_bind(q_sock,listensockaddr,socklen) < 0) {
      int e = tor_socket_errno(s);
      if (ERRNO_IS_EADDRINUSE(e))
      log_warn(LD_NET, "QUIC Could not bind to %s:%u: %s", address, usePort,
               tor_socket_strerror(e));
      goto err;
    }

    if (qs_listen(q_sock) < 0) {
      log_warn(LD_NET, "QUIC Could not listen on %s:%u: %s", address, usePort,
               tor_socket_strerror(tor_socket_errno(s)));
      goto err;
    }

    if (usePort != 0) {
      gotPort = usePort;
    } else {
      log_warn(LD_NET, "QUIC does not support port == 0 yet, exiting..");
      exit(1);
      /*
      tor_addr_t addr2;
      struct sockaddr_storage ss;
      socklen_t ss_len=sizeof(ss);
      if (getsockname(s, (struct sockaddr*)&ss, &ss_len)<0) {
        log_warn(LD_NET, "getsockname() couldn't learn address for %s: %s",
                 conn_type_to_string(type),
                 tor_socket_strerror(tor_socket_errno(s)));
        gotPort = 0;
      }
      tor_addr_from_sockaddr(&addr2, (struct sockaddr*)&ss, &gotPort);
    */
    }
#ifdef HAVE_SYS_UN_H
  /*
   * AF_UNIX generic setup stuff
   */
  } else if (listensockaddr->sa_family == AF_UNIX) {
    /* We want to start reading for both AF_UNIX cases */
    tor_assert(conn_listener_type_supports_af_unix(type));

    if (check_location_for_unix_socket(options, address,
           UNIX_SOCKET_PURPOSE_SOCKS_SOCKET, port_cfg) < 0) {
        goto err;
    }

    log_warn(LD_NET, "WARNING: Using quic code path for UNIX socket listen");
    use_quic = 0;

    log_notice(LD_NET, "Opening %s on %s",
               conn_type_to_string(type), address);

    tor_addr_make_unspec(&addr);

    if (unlink(address) < 0 && errno != ENOENT) {
      log_warn(LD_NET, "Could not unlink %s: %s", address,
                       strerror(errno));
      goto err;
    }

    s = tor_open_socket_nonblocking(AF_UNIX, SOCK_STREAM, 0);
    if (! SOCKET_OK(s)) {
      int e = tor_socket_errno(s);
      if (ERRNO_IS_RESOURCE_LIMIT(e)) {
        warn_too_many_conns();
      } else {
        log_warn(LD_NET,"Socket creation failed: %s.", strerror(e));
      }
      goto err;
    }

    if (bind(s, listensockaddr,
             (socklen_t)sizeof(struct sockaddr_un)) == -1) {
      log_warn(LD_NET,"Bind to %s failed: %s.", address,
               tor_socket_strerror(tor_socket_errno(s)));
      goto err;
    }

#ifdef HAVE_PWD_H
    if (options->User) {
      pw = tor_getpwnam(options->User);
      if (pw == NULL) {
        log_warn(LD_NET,"Unable to chown() %s socket: user %s not found.",
                 address, options->User);
        goto err;
      } else if (chown(address, pw->pw_uid, pw->pw_gid) < 0) {
        log_warn(LD_NET,"Unable to chown() %s socket: %s.",
                 address, strerror(errno));
        goto err;
      }
    }
#endif

    {
      unsigned mode;
      const char *status;
      if (port_cfg->is_world_writable) {
        mode = 0666;
        status = "world-writable";
      } else if (port_cfg->is_group_writable) {
        mode = 0660;
        status = "group-writable";
      } else {
        mode = 0600;
        status = "private";
      }
      /* We need to use chmod; fchmod doesn't work on sockets on all
       * platforms. */
      if (chmod(address, mode) < 0) {
        log_warn(LD_FS,"Unable to make %s %s.", address, status);
        goto err;
      }
    }

    if (listen(s, SOMAXCONN) < 0) {
      log_warn(LD_NET, "Could not listen on %s: %s", address,
               tor_socket_strerror(tor_socket_errno(s)));
      goto err;
    }
#endif /* HAVE_SYS_UN_H */
  } else {
    log_err(LD_BUG, "Got unexpected address family %d.",
            listensockaddr->sa_family);
    tor_assert(0);
  }

  lis_conn = listener_connection_new(type, listensockaddr->sa_family);
  conn = TO_CONN(lis_conn);
  conn->socket_family = listensockaddr->sa_family;
  conn->s = s;
  conn->use_quic = use_quic;
  conn->q_sock = q_sock;
  s = TOR_INVALID_SOCKET; /* Prevent double-close */
  q_sock = TOR_INVALID_QUICSOCK;
  conn->address = tor_strdup(address);
  conn->port = gotPort;
  tor_addr_copy(&conn->addr, &addr);

  memcpy(&lis_conn->entry_cfg, &port_cfg->entry_cfg, sizeof(entry_port_cfg_t));

  if (port_cfg->entry_cfg.isolation_flags) {
    lis_conn->entry_cfg.isolation_flags = port_cfg->entry_cfg.isolation_flags;
    if (port_cfg->entry_cfg.session_group >= 0) {
      lis_conn->entry_cfg.session_group = port_cfg->entry_cfg.session_group;
    } else {
      /* This can wrap after around INT_MAX listeners are opened.  But I don't
       * believe that matters, since you would need to open a ridiculous
       * number of listeners while keeping the early ones open before you ever
       * hit this.  An OR with a dozen ports open, for example, would have to
       * close and re-open its listeners every second for 4 years nonstop.
       */
      lis_conn->entry_cfg.session_group = global_next_session_group--;
    }
  }

  lis_conn->entry_cfg.ipv4_traffic = 1;
  lis_conn->entry_cfg.ipv6_traffic = 1;
  lis_conn->entry_cfg.prefer_ipv6 = 0;

  if (connection_add(conn) < 0) { /* no space, forget it */
    log_warn(LD_NET,"connection_add for listener failed. Giving up.");
    goto err;
  }

  log_fn(usePort==gotPort ? LOG_DEBUG : LOG_NOTICE, LD_NET,
         "%s listening on port %u.",
         conn_type_to_string(type), gotPort);

  conn->state = LISTENER_STATE_READY;
  connection_start_reading(conn);

  return conn;

 err:
  if (SOCKET_OK(s))
    tor_close_socket(s);
  if (conn)
    connection_free(conn);

  return NULL;
}

/** Bind a new non-blocking socket listening to the socket described
 * by <b>listensockaddr</b>.
 *
 * <b>address</b> is only used for logging purposes and to add the information
 * to the conn.
 */
static connection_t *
connection_listener_new(const struct sockaddr *listensockaddr,
                        socklen_t socklen,
                        int type, const char *address,
                        const port_cfg_t *port_cfg)
{
  listener_connection_t *lis_conn;
  connection_t *conn = NULL;
  tor_socket_t s = TOR_INVALID_SOCKET;  /* the socket we're going to make */
  or_options_t const *options = get_options();
  (void) options; /* Windows doesn't use this. */
#if defined(HAVE_PWD_H) && defined(HAVE_SYS_UN_H)
  const struct passwd *pw = NULL;
#endif
  uint16_t usePort = 0, gotPort = 0;
  int start_reading = 0;
  static int global_next_session_group = SESSION_GROUP_FIRST_AUTO;
  tor_addr_t addr;
  int exhaustion = 0;

  if (listensockaddr->sa_family == AF_INET ||
      listensockaddr->sa_family == AF_INET6) {
    int is_stream = (type != CONN_TYPE_AP_DNS_LISTENER);
    if (is_stream)
      start_reading = 1;

    tor_addr_from_sockaddr(&addr, listensockaddr, &usePort);
    log_notice(LD_NET, "Opening %s on %s",
               conn_type_to_string(type), fmt_addrport(&addr, usePort));

    s = tor_open_socket_nonblocking(tor_addr_family(&addr),
      is_stream ? SOCK_STREAM : SOCK_DGRAM,
      is_stream ? IPPROTO_TCP: IPPROTO_UDP);
    if (!SOCKET_OK(s)) {
      int e = tor_socket_errno(s);
      if (ERRNO_IS_RESOURCE_LIMIT(e)) {
        warn_too_many_conns();
        /*
         * We'll call the OOS handler at the error exit, so set the
         * exhaustion flag for it.
         */
        exhaustion = 1;
      } else {
        log_warn(LD_NET, "Socket creation failed: %s",
                 tor_socket_strerror(e));
      }
      goto err;
    }

    if (make_socket_reuseable(s) < 0) {
      log_warn(LD_NET, "Error setting SO_REUSEADDR flag on %s: %s",
               conn_type_to_string(type),
               tor_socket_strerror(errno));
    }

#ifdef _WIN32
    if (make_win32_socket_exclusive(s) < 0) {
      log_warn(LD_NET, "Error setting SO_EXCLUSIVEADDRUSE flag on %s: %s",
               conn_type_to_string(type),
               tor_socket_strerror(errno));
    }
#endif /* defined(_WIN32) */

#if defined(USE_TRANSPARENT) && defined(IP_TRANSPARENT)
    if (options->TransProxyType_parsed == TPT_TPROXY &&
        type == CONN_TYPE_AP_TRANS_LISTENER) {
      int one = 1;
      if (setsockopt(s, SOL_IP, IP_TRANSPARENT, (void*)&one,
                     (socklen_t)sizeof(one)) < 0) {
        const char *extra = "";
        int e = tor_socket_errno(s);
        if (e == EPERM)
          extra = "TransTPROXY requires root privileges or similar"
            " capabilities.";
        log_warn(LD_NET, "Error setting IP_TRANSPARENT flag: %s.%s",
                 tor_socket_strerror(e), extra);
      }
    }
#endif /* defined(USE_TRANSPARENT) && defined(IP_TRANSPARENT) */

#ifdef IPV6_V6ONLY
    if (listensockaddr->sa_family == AF_INET6) {
      int one = 1;
      /* We need to set IPV6_V6ONLY so that this socket can't get used for
       * IPv4 connections. */
      if (setsockopt(s,IPPROTO_IPV6, IPV6_V6ONLY,
                     (void*)&one, (socklen_t)sizeof(one)) < 0) {
        int e = tor_socket_errno(s);
        log_warn(LD_NET, "Error setting IPV6_V6ONLY flag: %s",
                 tor_socket_strerror(e));
        /* Keep going; probably not harmful. */
      }
    }
#endif /* defined(IPV6_V6ONLY) */

    if (bind(s,listensockaddr,socklen) < 0) {
      const char *helpfulhint = "";
      int e = tor_socket_errno(s);
      if (ERRNO_IS_EADDRINUSE(e))
        helpfulhint = ". Is Tor already running?";
      log_warn(LD_NET, "Could not bind to %s:%u: %s%s", address, usePort,
               tor_socket_strerror(e), helpfulhint);
      goto err;
    }

    if (is_stream) {
      if (tor_listen(s) < 0) {
        log_warn(LD_NET, "Could not listen on %s:%u: %s", address, usePort,
                 tor_socket_strerror(tor_socket_errno(s)));
        goto err;
      }
    }

    if (usePort != 0) {
      gotPort = usePort;
    } else {
      tor_addr_t addr2;
      if (tor_addr_from_getsockname(&addr2, s)<0) {
        log_warn(LD_NET, "getsockname() couldn't learn address for %s: %s",
                 conn_type_to_string(type),
                 tor_socket_strerror(tor_socket_errno(s)));
        gotPort = 0;
      }
    }
#ifdef HAVE_SYS_UN_H
  /*
   * AF_UNIX generic setup stuff
   */
  } else if (listensockaddr->sa_family == AF_UNIX) {
    /* We want to start reading for both AF_UNIX cases */
    start_reading = 1;

    tor_assert(conn_listener_type_supports_af_unix(type));

    if (check_location_for_unix_socket(options, address,
          (type == CONN_TYPE_CONTROL_LISTENER) ?
           UNIX_SOCKET_PURPOSE_CONTROL_SOCKET :
           UNIX_SOCKET_PURPOSE_SOCKS_SOCKET, port_cfg) < 0) {
        goto err;
    }

    log_notice(LD_NET, "Opening %s on %s",
               conn_type_to_string(type), address);

    tor_addr_make_unspec(&addr);

    if (unlink(address) < 0 && errno != ENOENT) {
      log_warn(LD_NET, "Could not unlink %s: %s", address,
                       strerror(errno));
      goto err;
    }

    s = tor_open_socket_nonblocking(AF_UNIX, SOCK_STREAM, 0);
    if (! SOCKET_OK(s)) {
      int e = tor_socket_errno(s);
      if (ERRNO_IS_RESOURCE_LIMIT(e)) {
        warn_too_many_conns();
        /*
         * We'll call the OOS handler at the error exit, so set the
         * exhaustion flag for it.
         */
        exhaustion = 1;
      } else {
        log_warn(LD_NET,"Socket creation failed: %s.", strerror(e));
      }
      goto err;
    }

    if (bind(s, listensockaddr,
             (socklen_t)sizeof(struct sockaddr_un)) == -1) {
      log_warn(LD_NET,"Bind to %s failed: %s.", address,
               tor_socket_strerror(tor_socket_errno(s)));
      goto err;
    }

#ifdef HAVE_PWD_H
    if (options->User) {
      pw = tor_getpwnam(options->User);
      struct stat st;
      if (pw == NULL) {
        log_warn(LD_NET,"Unable to chown() %s socket: user %s not found.",
                 address, options->User);
        goto err;
      } else if (fstat(s, &st) == 0 &&
                 st.st_uid == pw->pw_uid && st.st_gid == pw->pw_gid) {
        /* No change needed */
      } else if (chown(sandbox_intern_string(address),
                       pw->pw_uid, pw->pw_gid) < 0) {
        log_warn(LD_NET,"Unable to chown() %s socket: %s.",
                 address, strerror(errno));
        goto err;
      }
    }
#endif /* defined(HAVE_PWD_H) */

    {
      unsigned mode;
      const char *status;
      struct stat st;
      if (port_cfg->is_world_writable) {
        mode = 0666;
        status = "world-writable";
      } else if (port_cfg->is_group_writable) {
        mode = 0660;
        status = "group-writable";
      } else {
        mode = 0600;
        status = "private";
      }
      /* We need to use chmod; fchmod doesn't work on sockets on all
       * platforms. */
      if (fstat(s, &st) == 0 && (st.st_mode & 0777) == mode) {
        /* no change needed */
      } else if (chmod(sandbox_intern_string(address), mode) < 0) {
        log_warn(LD_FS,"Unable to make %s %s.", address, status);
        goto err;
      }
    }

    if (listen(s, SOMAXCONN) < 0) {
      log_warn(LD_NET, "Could not listen on %s: %s", address,
               tor_socket_strerror(tor_socket_errno(s)));
      goto err;
    }
#endif /* defined(HAVE_SYS_UN_H) */
  } else {
    log_err(LD_BUG, "Got unexpected address family %d.",
            listensockaddr->sa_family);
    tor_assert(0);
  }

  lis_conn = listener_connection_new(type, listensockaddr->sa_family);
  conn = TO_CONN(lis_conn);
  conn->socket_family = listensockaddr->sa_family;
  conn->s = s;
  s = TOR_INVALID_SOCKET; /* Prevent double-close */
  conn->address = tor_strdup(address);
  conn->port = gotPort;
  tor_addr_copy(&conn->addr, &addr);

  memcpy(&lis_conn->entry_cfg, &port_cfg->entry_cfg, sizeof(entry_port_cfg_t));

  if (port_cfg->entry_cfg.isolation_flags) {
    lis_conn->entry_cfg.isolation_flags = port_cfg->entry_cfg.isolation_flags;
    if (port_cfg->entry_cfg.session_group >= 0) {
      lis_conn->entry_cfg.session_group = port_cfg->entry_cfg.session_group;
    } else {
      /* This can wrap after around INT_MAX listeners are opened.  But I don't
       * believe that matters, since you would need to open a ridiculous
       * number of listeners while keeping the early ones open before you ever
       * hit this.  An OR with a dozen ports open, for example, would have to
       * close and re-open its listeners every second for 4 years nonstop.
       */
      lis_conn->entry_cfg.session_group = global_next_session_group--;
    }
  }

  if (type != CONN_TYPE_AP_LISTENER) {
    lis_conn->entry_cfg.ipv4_traffic = 1;
    lis_conn->entry_cfg.ipv6_traffic = 1;
    lis_conn->entry_cfg.prefer_ipv6 = 0;
  }

  if (connection_add(conn) < 0) { /* no space, forget it */
    log_warn(LD_NET,"connection_add for listener failed. Giving up.");
    goto err;
  }

  log_fn(usePort==gotPort ? LOG_DEBUG : LOG_NOTICE, LD_NET,
         "%s listening on port %u.",
         conn_type_to_string(type), gotPort);

  conn->state = LISTENER_STATE_READY;
  if (start_reading) {
    connection_start_reading(conn);
  } else {
    tor_assert(type == CONN_TYPE_AP_DNS_LISTENER);
    dnsserv_configure_listener(conn);
  }

  /*
   * Normal exit; call the OOS handler since connection count just changed;
   * the exhaustion flag will always be zero here though.
   */
  connection_check_oos(get_n_open_sockets(), 0);

  return conn;

 err:
  if (SOCKET_OK(s))
    tor_close_socket(s);
  if (conn)
    connection_free(conn);

  /* Call the OOS handler, indicate if we saw an exhaustion-related error */
  connection_check_oos(get_n_open_sockets(), exhaustion);

  return NULL;
}

/** Do basic sanity checking on a newly received socket. Return 0
 * if it looks ok, else return -1.
 *
 * Notably, some TCP stacks can erroneously have accept() return successfully
 * with socklen 0, when the client sends an RST before the accept call (as
 * nmap does).  We want to detect that, and not go on with the connection.
 */
static int
check_sockaddr(const struct sockaddr *sa, int len, int level)
{
  int ok = 1;

  if (sa->sa_family == AF_INET) {
    struct sockaddr_in *sin=(struct sockaddr_in*)sa;
    if (len != sizeof(struct sockaddr_in)) {
      log_fn(level, LD_NET, "Length of address not as expected: %d vs %d",
             len,(int)sizeof(struct sockaddr_in));
      ok = 0;
    }
    if (sin->sin_addr.s_addr == 0 || sin->sin_port == 0) {
      log_fn(level, LD_NET,
             "Address for new connection has address/port equal to zero.");
      ok = 0;
    }
  } else if (sa->sa_family == AF_INET6) {
    struct sockaddr_in6 *sin6=(struct sockaddr_in6*)sa;
    if (len != sizeof(struct sockaddr_in6)) {
      log_fn(level, LD_NET, "Length of address not as expected: %d vs %d",
             len,(int)sizeof(struct sockaddr_in6));
      ok = 0;
    }
    if (tor_mem_is_zero((void*)sin6->sin6_addr.s6_addr, 16) ||
        sin6->sin6_port == 0) {
      log_fn(level, LD_NET,
             "Address for new connection has address/port equal to zero.");
      ok = 0;
    }
  } else if (sa->sa_family == AF_UNIX) {
    ok = 1;
  } else {
    ok = 0;
  }
  return ok ? 0 : -1;
}

/** Check whether the socket family from an accepted socket <b>got</b> is the
 * same as the one that <b>listener</b> is waiting for.  If it isn't, log
 * a useful message and return -1.  Else return 0.
 *
 * This is annoying, but can apparently happen on some Darwins. */
static int
check_sockaddr_family_match(sa_family_t got, connection_t *listener)
{
  if (got != listener->socket_family) {
    log_info(LD_BUG, "A listener connection returned a socket with a "
             "mismatched family. %s for addr_family %d gave us a socket "
             "with address family %d.  Dropping.",
             conn_type_to_string(listener->type),
             (int)listener->socket_family,
             (int)got);
    return -1;
  }
  return 0;
}


///////////////////////////////////////
// Connection accept for quic socket //
///////////////////////////////////////
static int
connection_handle_listener_read_quic(connection_t *conn, int new_type)
{
  tor_quicsock_t q_news;
  connection_t *newconn = 0;
  /* information about the remote peer when connecting to other routers */
  struct sockaddr_storage addrbuf;
  struct sockaddr *remote = (struct sockaddr*)&addrbuf;
  /* length of the remote address. Must be whatever accept() needs. */
  socklen_t remotelen = (socklen_t)sizeof(addrbuf);
  //const or_options_t *options = get_options();

  tor_assert((size_t)remotelen >= sizeof(struct sockaddr_in));
  memset(&addrbuf, 0, sizeof(addrbuf));

  q_news = qs_accept(conn->q_sock, remote, &remotelen);
  if (!QUICSOCK_OK(q_news)) { /* accept() error */
    log_warn(LD_NET,"accept() failed. ");
    return -1;
  }
  log_debug(LD_NET,
            "Connection accepted on socket %d (child of fd %d).",
            (int)qs_get_fd(q_news),(int)qs_get_fd(conn->q_sock));

  if (conn->socket_family == AF_INET || conn->socket_family == AF_INET6) {
    tor_addr_t addr;
    uint16_t port;
    if (check_sockaddr(remote, remotelen, LOG_INFO)<0) {
      log_info(LD_NET,
               "accept() returned a strange address; closing connection.");
      qs_close(q_news);
      return 0;
    }

    tor_addr_from_sockaddr(&addr, remote, &port);

    newconn = connection_new(new_type, conn->socket_family);
    newconn->use_quic = 1;
    newconn->q_sock = q_news;

    /* remember the remote address */
    tor_addr_copy(&newconn->addr, &addr);
    newconn->port = port;
    newconn->address = tor_strdup(&addr);

  } else {
    tor_assert(0);
  };

  if (connection_add(newconn) < 0) { /* no space, forget it */
    connection_free(newconn);
    return 0; /* no need to tear down the parent */
  }

  if (connection_init_accepted_conn(newconn, TO_LISTENER_CONN(conn)) < 0) {
    if (! newconn->marked_for_close)
      connection_mark_for_close(newconn);
    return 0;
  }
  return 0;
}

/** The listener connection <b>conn</b> told poll() it wanted to read.
 * Call accept() on conn-\>s, and add the new connection if necessary.
 */
static int
connection_handle_listener_read(connection_t *conn, int new_type)
{
  tor_socket_t news; /* the new socket */
  connection_t *newconn = 0;
  /* information about the remote peer when connecting to other routers */
  struct sockaddr_storage addrbuf;
  struct sockaddr *remote = (struct sockaddr*)&addrbuf;
  /* length of the remote address. Must be whatever accept() needs. */
  socklen_t remotelen = (socklen_t)sizeof(addrbuf);
  const or_options_t *options = get_options();

  tor_assert((size_t)remotelen >= sizeof(struct sockaddr_in));
  memset(&addrbuf, 0, sizeof(addrbuf));

  news = tor_accept_socket_nonblocking(conn->s,remote,&remotelen);
  if (!SOCKET_OK(news)) { /* accept() error */
    int e = tor_socket_errno(conn->s);
    if (ERRNO_IS_ACCEPT_EAGAIN(e)) {
      /*
       * they hung up before we could accept(). that's fine.
       *
       * give the OOS handler a chance to run though
       */
      connection_check_oos(get_n_open_sockets(), 0);
      return 0;
    } else if (ERRNO_IS_RESOURCE_LIMIT(e)) {
      warn_too_many_conns();
      /* Exhaustion; tell the OOS handler */
      connection_check_oos(get_n_open_sockets(), 1);
      return 0;
    }
    /* else there was a real error. */
    log_warn(LD_NET,"accept() failed: %s. Closing listener.",
             tor_socket_strerror(e));
    connection_mark_for_close(conn);
    /* Tell the OOS handler about this too */
    connection_check_oos(get_n_open_sockets(), 0);
    return -1;
  }
  log_debug(LD_NET,
            "Connection accepted on socket %d (child of fd %d).",
            (int)news,(int)conn->s);

  /* We accepted a new conn; run OOS handler */
  connection_check_oos(get_n_open_sockets(), 0);

  if (make_socket_reuseable(news) < 0) {
    if (tor_socket_errno(news) == EINVAL) {
      /* This can happen on OSX if we get a badly timed shutdown. */
      log_debug(LD_NET, "make_socket_reuseable returned EINVAL");
    } else {
      log_warn(LD_NET, "Error setting SO_REUSEADDR flag on %s: %s",
               conn_type_to_string(new_type),
               tor_socket_strerror(errno));
    }
    tor_close_socket(news);
    return 0;
  }

  if (options->ConstrainedSockets)
    set_constrained_socket_buffers(news, (int)options->ConstrainedSockSize);

  if (check_sockaddr_family_match(remote->sa_family, conn) < 0) {
    tor_close_socket(news);
    return 0;
  }

  if (conn->socket_family == AF_INET || conn->socket_family == AF_INET6 ||
     (conn->socket_family == AF_UNIX && new_type == CONN_TYPE_AP)) {
    tor_addr_t addr;
    uint16_t port;
    if (check_sockaddr(remote, remotelen, LOG_INFO)<0) {
      log_info(LD_NET,
               "accept() returned a strange address; closing connection.");
      tor_close_socket(news);
      return 0;
    }

    tor_addr_from_sockaddr(&addr, remote, &port);

    /* process entrance policies here, before we even create the connection */
    if (new_type == CONN_TYPE_AP) {
      /* check sockspolicy to see if we should accept it */
      if (socks_policy_permits_address(&addr) == 0) {
        log_notice(LD_APP,
                   "Denying socks connection from untrusted address %s.",
                   fmt_and_decorate_addr(&addr));
        tor_close_socket(news);
        return 0;
      }
    }
    if (new_type == CONN_TYPE_DIR) {
      /* check dirpolicy to see if we should accept it */
      if (dir_policy_permits_address(&addr) == 0) {
        log_notice(LD_DIRSERV,"Denying dir connection from address %s.",
                   fmt_and_decorate_addr(&addr));
        tor_close_socket(news);
        return 0;
      }
    }
    if (new_type == CONN_TYPE_OR) {
      /* Assess with the connection DoS mitigation subsystem if this address
       * can open a new connection. */
      if (dos_conn_addr_get_defense_type(&addr) == DOS_CONN_DEFENSE_CLOSE) {
        tor_close_socket(news);
        return 0;
      }
    }

    newconn = connection_new(new_type, conn->socket_family);
    newconn->s = news;

    /* remember the remote address */
    tor_addr_copy(&newconn->addr, &addr);
    if (new_type == CONN_TYPE_AP && conn->socket_family == AF_UNIX) {
      newconn->port = 0;
      newconn->address = tor_strdup(conn->address);
    } else {
      newconn->port = port;
      newconn->address = tor_addr_to_str_dup(&addr);
    }

    if (new_type == CONN_TYPE_AP && conn->socket_family != AF_UNIX) {
      log_info(LD_NET, "New SOCKS connection opened from %s.",
               fmt_and_decorate_addr(&addr));
    }
    if (new_type == CONN_TYPE_AP && conn->socket_family == AF_UNIX) {
      log_info(LD_NET, "New SOCKS AF_UNIX connection opened");
    }
    if (new_type == CONN_TYPE_CONTROL) {
      log_notice(LD_CONTROL, "New control connection opened from %s.",
                 fmt_and_decorate_addr(&addr));
    }

  } else if (conn->socket_family == AF_UNIX && conn->type != CONN_TYPE_AP) {
    tor_assert(conn->type == CONN_TYPE_CONTROL_LISTENER);
    tor_assert(new_type == CONN_TYPE_CONTROL);
    log_notice(LD_CONTROL, "New control connection opened.");

    newconn = connection_new(new_type, conn->socket_family);
    newconn->s = news;

    /* remember the remote address -- do we have anything sane to put here? */
    tor_addr_make_unspec(&newconn->addr);
    newconn->port = 1;
    newconn->address = tor_strdup(conn->address);
  } else {
    tor_assert(0);
  };

  if (connection_add(newconn) < 0) { /* no space, forget it */
    connection_free(newconn);
    return 0; /* no need to tear down the parent */
  }

  if (connection_init_accepted_conn(newconn, TO_LISTENER_CONN(conn)) < 0) {
    if (! newconn->marked_for_close)
      connection_mark_for_close(newconn);
    return 0;
  }
  return 0;
}

/** Initialize states for newly accepted connection <b>conn</b>.
 *
 * If conn is an OR, start the TLS handshake.
 *
 * If conn is a transparent AP, get its original destination
 * and place it in circuit_wait.
 *
 * The <b>listener</b> parameter is only used for AP connections.
 */
int
connection_init_accepted_conn(connection_t *conn,
                              const listener_connection_t *listener)
{
  int rv;

  connection_start_reading(conn);

  switch (conn->type) {
    case CONN_TYPE_EXT_OR:
      /* Initiate Extended ORPort authentication. */
      return connection_ext_or_start_auth(TO_OR_CONN(conn));
    case CONN_TYPE_OR: {
      return connection_or_accepted_quic_connection(TO_OR_CONN(conn));

      /*rv = connection_tls_start_handshake(TO_OR_CONN(conn), 1);
      if (rv < 0) {
        connection_or_close_for_error(TO_OR_CONN(conn), 0);
      }
      return rv;*/
      break;
    }
    case CONN_TYPE_AP:
      memcpy(&TO_ENTRY_CONN(conn)->entry_cfg, &listener->entry_cfg,
             sizeof(entry_port_cfg_t));
      TO_ENTRY_CONN(conn)->nym_epoch = get_signewnym_epoch();
      TO_ENTRY_CONN(conn)->socks_request->listener_type = listener->base_.type;

      switch (TO_CONN(listener)->type) {
        case CONN_TYPE_AP_LISTENER:
          conn->state = AP_CONN_STATE_SOCKS_WAIT;
          TO_ENTRY_CONN(conn)->socks_request->socks_prefer_no_auth =
            listener->entry_cfg.socks_prefer_no_auth;
          break;
        case CONN_TYPE_AP_TRANS_LISTENER:
          TO_ENTRY_CONN(conn)->is_transparent_ap = 1;
          /* XXXX028 -- is this correct still, with the addition of
           * pending_entry_connections ? */
          conn->state = AP_CONN_STATE_CIRCUIT_WAIT;
          return connection_ap_process_transparent(TO_ENTRY_CONN(conn));
        case CONN_TYPE_AP_NATD_LISTENER:
          TO_ENTRY_CONN(conn)->is_transparent_ap = 1;
          conn->state = AP_CONN_STATE_NATD_WAIT;
          break;
        case CONN_TYPE_AP_HTTP_CONNECT_LISTENER:
          conn->state = AP_CONN_STATE_HTTP_CONNECT_WAIT;
      }
      break;
    case CONN_TYPE_DIR:
      conn->purpose = DIR_PURPOSE_SERVER;
      conn->state = DIR_CONN_STATE_SERVER_COMMAND_WAIT;
      break;
    case CONN_TYPE_CONTROL:
      conn->state = CONTROL_CONN_STATE_NEEDAUTH;
      break;
  }
  return 0;
}

/** Take conn, make a nonblocking socket; try to connect to
 * sa, binding to bindaddr if sa is not localhost. If fail, return -1 and if
 * applicable put your best guess about errno into *<b>socket_error</b>.
 * If connected return 1, if EAGAIN return 0.
 */
MOCK_IMPL(STATIC int,
connection_connect_sockaddr,(connection_t *conn,
                            const struct sockaddr *sa,
                            socklen_t sa_len,
                            const struct sockaddr *bindaddr,
                            socklen_t bindaddr_len,
                            int *socket_error))
{
  tor_socket_t s;
  int inprogress = 0;
  const or_options_t *options = get_options();

  tor_assert(conn);
  tor_assert(sa);
  tor_assert(socket_error);

  if (get_options()->DisableNetwork) {
    /* We should never even try to connect anyplace if DisableNetwork is set.
     * Warn if we do, and refuse to make the connection.
     *
     * We only check DisableNetwork here, not we_are_hibernating(), since
     * we'll still try to fulfill client requests sometimes in the latter case
     * (if it is soft hibernation) */
    static ratelim_t disablenet_violated = RATELIM_INIT(30*60);
    *socket_error = SOCK_ERRNO(ENETUNREACH);
    log_fn_ratelim(&disablenet_violated, LOG_WARN, LD_BUG,
                   "Tried to open a socket with DisableNetwork set.");
    tor_fragile_assert();
    return -1;
  }

  const int protocol_family = sa->sa_family;
  const int proto = (sa->sa_family == AF_INET6 ||
                     sa->sa_family == AF_INET) ? IPPROTO_TCP : 0;

  s = tor_open_socket_nonblocking(protocol_family, SOCK_STREAM, proto);
  if (! SOCKET_OK(s)) {
    /*
     * Early OOS handler calls; it matters if it's an exhaustion-related
     * error or not.
     */
    *socket_error = tor_socket_errno(s);
    if (ERRNO_IS_RESOURCE_LIMIT(*socket_error)) {
      warn_too_many_conns();
      connection_check_oos(get_n_open_sockets(), 1);
    } else {
      log_warn(LD_NET,"Error creating network socket: %s",
               tor_socket_strerror(*socket_error));
      connection_check_oos(get_n_open_sockets(), 0);
    }
    return -1;
  }

  if (make_socket_reuseable(s) < 0) {
    log_warn(LD_NET, "Error setting SO_REUSEADDR flag on new connection: %s",
             tor_socket_strerror(errno));
  }

  /*
   * We've got the socket open; give the OOS handler a chance to check
   * against configured maximum socket number, but tell it no exhaustion
   * failure.
   */
  connection_check_oos(get_n_open_sockets(), 0);

  if (bindaddr && bind(s, bindaddr, bindaddr_len) < 0) {
    *socket_error = tor_socket_errno(s);
    log_warn(LD_NET,"Error binding network socket: %s",
             tor_socket_strerror(*socket_error));
    tor_close_socket(s);
    return -1;
  }

  tor_assert(options);
  if (options->ConstrainedSockets)
    set_constrained_socket_buffers(s, (int)options->ConstrainedSockSize);

  if (connect(s, sa, sa_len) < 0) {
    int e = tor_socket_errno(s);
    if (!ERRNO_IS_CONN_EINPROGRESS(e)) {
      /* yuck. kill it. */
      *socket_error = e;
      log_info(LD_NET,
               "connect() to socket failed: %s",
               tor_socket_strerror(e));
      tor_close_socket(s);
      return -1;
    } else {
      inprogress = 1;
    }
  }

  /* it succeeded. we're connected. */
  log_fn(inprogress ? LOG_DEBUG : LOG_INFO, LD_NET,
         "Connection to socket %s (sock "TOR_SOCKET_T_FORMAT").",
         inprogress ? "in progress" : "established", s);
  conn->s = s;
  if (connection_add_connecting(conn) < 0) {
    /* no space, forget it */
    *socket_error = SOCK_ERRNO(ENOBUFS);
    return -1;
  }

  return inprogress ? 0 : 1;
}


// For QUIC only
STATIC int
connection_connect_sockaddr_quic(connection_t *conn,
                            const struct sockaddr *sa,
                            socklen_t sa_len,
                            const struct sockaddr *bindaddr,
                            socklen_t bindaddr_len,
                            int *socket_error)
{
  tor_quicsock_t s;
  int inprogress = 0;
  const or_options_t *options = get_options();

  tor_assert(conn);
  tor_assert(sa);
  tor_assert(socket_error);

  if (get_options()->DisableNetwork) {
    /* We should never even try to connect anyplace if DisableNetwork is set.
     * Warn if we do, and refuse to make the connection. */
    static ratelim_t disablenet_violated = RATELIM_INIT(30*60);
    *socket_error = SOCK_ERRNO(ENETUNREACH);
    log_fn_ratelim(&disablenet_violated, LOG_WARN, LD_BUG,
                   "Tried to open a socket with DisableNetwork set.");
    tor_fragile_assert();
    return -1;
  }

  s = qs_open();
  // handle the alarm needed for QUIC since we are going to use this socket
  //quicsock_event_handler_t e_handler = qs_get_event_handler(s);
  //tor_assert(e_handler != NULL);
  //qs_util_set_libevent_handler_base(e_handler, tor_libevent_get_base());

  if (! SOCKET_OK(s)) {
    /*
    *socket_error = tor_socket_errno(s);
    if (ERRNO_IS_RESOURCE_LIMIT(*socket_error)) {
      warn_too_many_conns();
    } else {
      log_warn(LD_NET,"Error creating network socket: %s",
               tor_socket_strerror(*socket_error));
    }
    */
    return -1;
  }

  /* Turn off for now
  if (make_socket_reuseable(s) < 0) {
    log_warn(LD_NET, "Error setting SO_REUSEADDR flag on new connection: %s",
             tor_socket_strerror(errno));
  }
  */

  if (bindaddr && qs_bind(s, bindaddr, bindaddr_len) < 0) {
    log_warn(LD_NET,"Error binding network socket");
    qs_close(s);
    return -1;
  }

  tor_assert(options);
  /*
  if (options->ConstrainedSockets)
    set_constrained_socket_buffers(s, (int)options->ConstrainedSockSize);
  */
  log_info(LD_BUG,"Lamiaa : before qs_connect ");
  if (qs_connect(s, sa, sa_len) < 0) {
    /*
    int e = tor_socket_errno(s);
    if (!ERRNO_IS_CONN_EINPROGRESS(e)) {
      *socket_error = e;
      log_info(LD_NET,
               "connect() to socket failed: %s",
               tor_socket_strerror(e));
      tor_close_socket(s);
      return -1;
    } else {
      inprogress = 1;
    }
    */
    log_warn(LD_NET,"Error connect() failed network socket");
    qs_close(s);
    return -1;
  }

  conn->q_sock = s;

  /* it succeeded. we're connected. */
  log_fn(inprogress ? LOG_DEBUG : LOG_INFO, LD_NET,
         "Connection to socket %s (quic sock %d).",
         inprogress ? "in progress" : "established", (int)qs_get_fd(s));
  if (connection_add_connecting(conn) < 0) {
    /* no space, forget it */
    //*socket_error = SOCK_ERRNO(ENOBUFS);
    return -1;
  }

  // TODO XXX
  return 0;
}
/* Log a message if connection attempt is made when IPv4 or IPv6 is disabled.
 * Log a less severe message if we couldn't conform to ClientPreferIPv6ORPort
 * or ClientPreferIPv6ORPort. */
static void
connection_connect_log_client_use_ip_version(const connection_t *conn)
{
  const or_options_t *options = get_options();

  /* Only clients care about ClientUseIPv4/6, bail out early on servers, and
   * on connections we don't care about */
  if (server_mode(options) || !conn || conn->type == CONN_TYPE_EXIT) {
    return;
  }

  /* We're only prepared to log OR and DIR connections here */
  if (conn->type != CONN_TYPE_OR && conn->type != CONN_TYPE_DIR) {
    return;
  }

  const int must_ipv4 = !fascist_firewall_use_ipv6(options);
  const int must_ipv6 = (options->ClientUseIPv4 == 0);
  const int pref_ipv6 = (conn->type == CONN_TYPE_OR
                         ? fascist_firewall_prefer_ipv6_orport(options)
                         : fascist_firewall_prefer_ipv6_dirport(options));
  tor_addr_t real_addr;
  tor_addr_make_null(&real_addr, AF_UNSPEC);

  /* OR conns keep the original address in real_addr, as addr gets overwritten
   * with the descriptor address */
  if (conn->type == CONN_TYPE_OR) {
    const or_connection_t *or_conn = TO_OR_CONN((connection_t *)conn);
    tor_addr_copy(&real_addr, &or_conn->real_addr);
  } else if (conn->type == CONN_TYPE_DIR) {
    tor_addr_copy(&real_addr, &conn->addr);
  }

  /* Check if we broke a mandatory address family restriction */
  if ((must_ipv4 && tor_addr_family(&real_addr) == AF_INET6)
      || (must_ipv6 && tor_addr_family(&real_addr) == AF_INET)) {
    static int logged_backtrace = 0;
    log_info(LD_BUG, "Outgoing %s connection to %s violated ClientUseIPv%s 0.",
             conn->type == CONN_TYPE_OR ? "OR" : "Dir",
             fmt_addr(&real_addr),
             options->ClientUseIPv4 == 0 ? "4" : "6");
    if (!logged_backtrace) {
      log_backtrace(LOG_INFO, LD_BUG, "Address came from");
      logged_backtrace = 1;
    }
  }

  /* Bridges are allowed to break IPv4/IPv6 ORPort preferences to connect to
   * the node's configured address when ClientPreferIPv6ORPort is auto */
  if (options->UseBridges && conn->type == CONN_TYPE_OR
      && options->ClientPreferIPv6ORPort == -1) {
    return;
  }

  /* Check if we couldn't satisfy an address family preference */
  if ((!pref_ipv6 && tor_addr_family(&real_addr) == AF_INET6)
      || (pref_ipv6 && tor_addr_family(&real_addr) == AF_INET)) {
    log_info(LD_NET, "Outgoing connection to %s doesn't satisfy "
             "ClientPreferIPv6%sPort %d, with ClientUseIPv4 %d, and "
             "fascist_firewall_use_ipv6 %d (ClientUseIPv6 %d and UseBridges "
             "%d).",
             fmt_addr(&real_addr),
             conn->type == CONN_TYPE_OR ? "OR" : "Dir",
             conn->type == CONN_TYPE_OR ? options->ClientPreferIPv6ORPort
                                        : options->ClientPreferIPv6DirPort,
             options->ClientUseIPv4, fascist_firewall_use_ipv6(options),
             options->ClientUseIPv6, options->UseBridges);
  }
}

/** Retrieve the outbound address depending on the protocol (IPv4 or IPv6)
 * and the connection type (relay, exit, ...)
 * Return a socket address or NULL in case nothing is configured.
 **/
const tor_addr_t *
conn_get_outbound_address(sa_family_t family,
             const or_options_t *options, unsigned int conn_type)
{
  const tor_addr_t *ext_addr = NULL;

  int fam_index;
  switch (family) {
    case AF_INET:
      fam_index = 0;
      break;
    case AF_INET6:
      fam_index = 1;
      break;
    default:
      return NULL;
  }

  // If an exit connection, use the exit address (if present)
  if (conn_type == CONN_TYPE_EXIT) {
    if (!tor_addr_is_null(
        &options->OutboundBindAddresses[OUTBOUND_ADDR_EXIT][fam_index])) {
      ext_addr = &options->OutboundBindAddresses[OUTBOUND_ADDR_EXIT]
                 [fam_index];
    } else if (!tor_addr_is_null(
                 &options->OutboundBindAddresses[OUTBOUND_ADDR_EXIT_AND_OR]
                 [fam_index])) {
      ext_addr = &options->OutboundBindAddresses[OUTBOUND_ADDR_EXIT_AND_OR]
                 [fam_index];
    }
  } else { // All non-exit connections
    if (!tor_addr_is_null(
           &options->OutboundBindAddresses[OUTBOUND_ADDR_OR][fam_index])) {
      ext_addr = &options->OutboundBindAddresses[OUTBOUND_ADDR_OR]
                 [fam_index];
    } else if (!tor_addr_is_null(
                 &options->OutboundBindAddresses[OUTBOUND_ADDR_EXIT_AND_OR]
                 [fam_index])) {
      ext_addr = &options->OutboundBindAddresses[OUTBOUND_ADDR_EXIT_AND_OR]
                 [fam_index];
    }
  }
  return ext_addr;
}

/** Take conn, make a nonblocking socket; try to connect to
 * addr:port (port arrives in *host order*). If fail, return -1 and if
 * applicable put your best guess about errno into *<b>socket_error</b>.
 * Else assign s to conn-\>s: if connected return 1, if EAGAIN return 0.
 *
 * addr:port can be different to conn->addr:conn->port if connecting through
 * a proxy.
 *
 * address is used to make the logs useful.
 *
 * On success, add conn to the list of polled connections.
 */
int
connection_connect(connection_t *conn, const char *address,
                   const tor_addr_t *addr, uint16_t port, int *socket_error)
{
  struct sockaddr_storage addrbuf;
  struct sockaddr_storage bind_addr_ss;
  struct sockaddr *bind_addr = NULL;
  struct sockaddr *dest_addr;
  int dest_addr_len, bind_addr_len = 0;

  /* Log if we didn't stick to ClientUseIPv4/6 or ClientPreferIPv6OR/DirPort
   */
  connection_connect_log_client_use_ip_version(conn);

  if (!tor_addr_is_loopback(addr)) {
    const tor_addr_t *ext_addr = NULL;
    ext_addr = conn_get_outbound_address(tor_addr_family(addr), get_options(),
                                         conn->type);
    if (ext_addr) {
      memset(&bind_addr_ss, 0, sizeof(bind_addr_ss));
      bind_addr_len = tor_addr_to_sockaddr(ext_addr, 0,
                                           (struct sockaddr *) &bind_addr_ss,
                                           sizeof(bind_addr_ss));
      if (bind_addr_len == 0) {
        log_warn(LD_NET,
                 "Error converting OutboundBindAddress %s into sockaddr. "
                 "Ignoring.", fmt_and_decorate_addr(ext_addr));
      } else {
        bind_addr = (struct sockaddr *)&bind_addr_ss;
      }
    }
  }

  memset(&addrbuf,0,sizeof(addrbuf));
  dest_addr = (struct sockaddr*) &addrbuf;
  dest_addr_len = tor_addr_to_sockaddr(addr, port, dest_addr, sizeof(addrbuf));
  tor_assert(dest_addr_len > 0);

  log_debug(LD_NET, "Connecting to %s:%u.",
            escaped_safe_str_client(address), port);
  if (conn->use_quic)
     return connection_connect_sockaddr_quic(conn, dest_addr, dest_addr_len,
                                      bind_addr, bind_addr_len, socket_error);
   else
  return connection_connect_sockaddr(conn, dest_addr, dest_addr_len,
                                     bind_addr, bind_addr_len, socket_error);
}

#ifdef HAVE_SYS_UN_H

/** Take conn, make a nonblocking socket; try to connect to
 * an AF_UNIX socket at socket_path. If fail, return -1 and if applicable
 * put your best guess about errno into *<b>socket_error</b>. Else assign s
 * to conn-\>s: if connected return 1, if EAGAIN return 0.
 *
 * On success, add conn to the list of polled connections.
 */
int
connection_connect_unix(connection_t *conn, const char *socket_path,
                        int *socket_error)
{
  struct sockaddr_un dest_addr;

  tor_assert(socket_path);

  /* Check that we'll be able to fit it into dest_addr later */
  if (strlen(socket_path) + 1 > sizeof(dest_addr.sun_path)) {
    log_warn(LD_NET,
             "Path %s is too long for an AF_UNIX socket\n",
             escaped_safe_str_client(socket_path));
    *socket_error = SOCK_ERRNO(ENAMETOOLONG);
    return -1;
  }

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sun_family = AF_UNIX;
  strlcpy(dest_addr.sun_path, socket_path, sizeof(dest_addr.sun_path));

  log_debug(LD_NET,
            "Connecting to AF_UNIX socket at %s.",
            escaped_safe_str_client(socket_path));

  return connection_connect_sockaddr(conn,
                       (struct sockaddr *)&dest_addr, sizeof(dest_addr),
                       NULL, 0, socket_error);
}

#endif /* defined(HAVE_SYS_UN_H) */

/** Convert state number to string representation for logging purposes.
 */
static const char *
connection_proxy_state_to_string(int state)
{
  static const char *unknown = "???";
  static const char *states[] = {
    "PROXY_NONE",
    "PROXY_INFANT",
    "PROXY_HTTPS_WANT_CONNECT_OK",
    "PROXY_SOCKS4_WANT_CONNECT_OK",
    "PROXY_SOCKS5_WANT_AUTH_METHOD_NONE",
    "PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929",
    "PROXY_SOCKS5_WANT_AUTH_RFC1929_OK",
    "PROXY_SOCKS5_WANT_CONNECT_OK",
    "PROXY_CONNECTED",
  };

  if (state < PROXY_NONE || state > PROXY_CONNECTED)
    return unknown;

  return states[state];
}

/** Returns the global proxy type used by tor. Use this function for
 *  logging or high-level purposes, don't use it to fill the
 *  <b>proxy_type</b> field of or_connection_t; use the actual proxy
 *  protocol instead.*/
static int
get_proxy_type(void)
{
  const or_options_t *options = get_options();

  if (options->ClientTransportPlugin)
    return PROXY_PLUGGABLE;
  else if (options->HTTPSProxy)
    return PROXY_CONNECT;
  else if (options->Socks4Proxy)
    return PROXY_SOCKS4;
  else if (options->Socks5Proxy)
    return PROXY_SOCKS5;
  else
    return PROXY_NONE;
}

/* One byte for the version, one for the command, two for the
   port, and four for the addr... and, one more for the
   username NUL: */
#define SOCKS4_STANDARD_BUFFER_SIZE (1 + 1 + 2 + 4 + 1)

/** Write a proxy request of <b>type</b> (socks4, socks5, https) to conn
 * for conn->addr:conn->port, authenticating with the auth details given
 * in the configuration (if available). SOCKS 5 and HTTP CONNECT proxies
 * support authentication.
 *
 * Returns -1 if conn->addr is incompatible with the proxy protocol, and
 * 0 otherwise.
 *
 * Use connection_read_proxy_handshake() to complete the handshake.
 */
int
connection_proxy_connect(connection_t *conn, int type)
{
  const or_options_t *options;

  tor_assert(conn);

  options = get_options();

  switch (type) {
    case PROXY_CONNECT: {
      char buf[1024];
      char *base64_authenticator=NULL;
      const char *authenticator = options->HTTPSProxyAuthenticator;

      /* Send HTTP CONNECT and authentication (if available) in
       * one request */

      if (authenticator) {
        base64_authenticator = alloc_http_authenticator(authenticator);
        if (!base64_authenticator)
          log_warn(LD_OR, "Encoding https authenticator failed");
      }

      if (base64_authenticator) {
        const char *addrport = fmt_addrport(&conn->addr, conn->port);
        tor_snprintf(buf, sizeof(buf), "CONNECT %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Proxy-Authorization: Basic %s\r\n\r\n",
                     addrport,
                     addrport,
                     base64_authenticator);
        tor_free(base64_authenticator);
      } else {
        tor_snprintf(buf, sizeof(buf), "CONNECT %s HTTP/1.0\r\n\r\n",
                     fmt_addrport(&conn->addr, conn->port));
      }

      connection_buf_add(buf, strlen(buf), conn);
      conn->proxy_state = PROXY_HTTPS_WANT_CONNECT_OK;
      break;
    }

    case PROXY_SOCKS4: {
      unsigned char *buf;
      uint16_t portn;
      uint32_t ip4addr;
      size_t buf_size = 0;
      char *socks_args_string = NULL;

      /* Send a SOCKS4 connect request */

      if (tor_addr_family(&conn->addr) != AF_INET) {
        log_warn(LD_NET, "SOCKS4 client is incompatible with IPv6");
        return -1;
      }

      { /* If we are here because we are trying to connect to a
           pluggable transport proxy, check if we have any SOCKS
           arguments to transmit. If we do, compress all arguments to
           a single string in 'socks_args_string': */

        if (get_proxy_type() == PROXY_PLUGGABLE) {
          socks_args_string =
            pt_get_socks_args_for_proxy_addrport(&conn->addr, conn->port);
          if (socks_args_string)
            log_debug(LD_NET, "Sending out '%s' as our SOCKS argument string.",
                      socks_args_string);
        }
      }

      { /* Figure out the buffer size we need for the SOCKS message: */

        buf_size = SOCKS4_STANDARD_BUFFER_SIZE;

        /* If we have a SOCKS argument string, consider its size when
           calculating the buffer size: */
        if (socks_args_string)
          buf_size += strlen(socks_args_string);
      }

      buf = tor_malloc_zero(buf_size);

      ip4addr = tor_addr_to_ipv4n(&conn->addr);
      portn = htons(conn->port);

      buf[0] = 4; /* version */
      buf[1] = SOCKS_COMMAND_CONNECT; /* command */
      memcpy(buf + 2, &portn, 2); /* port */
      memcpy(buf + 4, &ip4addr, 4); /* addr */

      /* Next packet field is the userid. If we have pluggable
         transport SOCKS arguments, we have to embed them
         there. Otherwise, we use an empty userid.  */
      if (socks_args_string) { /* place the SOCKS args string: */
        tor_assert(strlen(socks_args_string) > 0);
        tor_assert(buf_size >=
                   SOCKS4_STANDARD_BUFFER_SIZE + strlen(socks_args_string));
        strlcpy((char *)buf + 8, socks_args_string, buf_size - 8);
        tor_free(socks_args_string);
      } else {
        buf[8] = 0; /* no userid */
      }

      connection_buf_add((char *)buf, buf_size, conn);
      tor_free(buf);

      conn->proxy_state = PROXY_SOCKS4_WANT_CONNECT_OK;
      break;
    }

    case PROXY_SOCKS5: {
      unsigned char buf[4]; /* fields: vers, num methods, method list */

      /* Send a SOCKS5 greeting (connect request must wait) */

      buf[0] = 5; /* version */

      /* We have to use SOCKS5 authentication, if we have a
         Socks5ProxyUsername or if we want to pass arguments to our
         pluggable transport proxy: */
      if ((options->Socks5ProxyUsername) ||
          (get_proxy_type() == PROXY_PLUGGABLE &&
           (get_socks_args_by_bridge_addrport(&conn->addr, conn->port)))) {
      /* number of auth methods */
        buf[1] = 2;
        buf[2] = 0x00; /* no authentication */
        buf[3] = 0x02; /* rfc1929 Username/Passwd auth */
        conn->proxy_state = PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929;
      } else {
        buf[1] = 1;
        buf[2] = 0x00; /* no authentication */
        conn->proxy_state = PROXY_SOCKS5_WANT_AUTH_METHOD_NONE;
      }

      connection_buf_add((char *)buf, 2 + buf[1], conn);
      break;
    }

    default:
      log_err(LD_BUG, "Invalid proxy protocol, %d", type);
      tor_fragile_assert();
      return -1;
  }

  log_debug(LD_NET, "set state %s",
            connection_proxy_state_to_string(conn->proxy_state));

  return 0;
}

/** Read conn's inbuf. If the http response from the proxy is all
 * here, make sure it's good news, then return 1. If it's bad news,
 * return -1. Else return 0 and hope for better luck next time.
 */
static int
connection_read_https_proxy_response(connection_t *conn)
{
  char *headers;
  char *reason=NULL;
  int status_code;
  time_t date_header;

  switch (fetch_from_buf_http(conn->inbuf,
                              &headers, MAX_HEADERS_SIZE,
                              NULL, NULL, 10000, 0)) {
    case -1: /* overflow */
      log_warn(LD_PROTOCOL,
               "Your https proxy sent back an oversized response. Closing.");
      return -1;
    case 0:
      log_info(LD_NET,"https proxy response not all here yet. Waiting.");
      return 0;
    /* case 1, fall through */
  }

  if (parse_http_response(headers, &status_code, &date_header,
                          NULL, &reason) < 0) {
    log_warn(LD_NET,
             "Unparseable headers from proxy (connecting to '%s'). Closing.",
             conn->address);
    tor_free(headers);
    return -1;
  }
  tor_free(headers);
  if (!reason) reason = tor_strdup("[no reason given]");

  if (status_code == 200) {
    log_info(LD_NET,
             "HTTPS connect to '%s' successful! (200 %s) Starting TLS.",
             conn->address, escaped(reason));
    tor_free(reason);
    return 1;
  }
  /* else, bad news on the status code */
  switch (status_code) {
    case 403:
      log_warn(LD_NET,
             "The https proxy refused to allow connection to %s "
             "(status code %d, %s). Closing.",
             conn->address, status_code, escaped(reason));
      break;
    default:
      log_warn(LD_NET,
             "The https proxy sent back an unexpected status code %d (%s). "
             "Closing.",
             status_code, escaped(reason));
      break;
  }
  tor_free(reason);
  return -1;
}

/** Send SOCKS5 CONNECT command to <b>conn</b>, copying <b>conn->addr</b>
 * and <b>conn->port</b> into the request.
 */
static void
connection_send_socks5_connect(connection_t *conn)
{
  unsigned char buf[1024];
  size_t reqsize = 6;
  uint16_t port = htons(conn->port);

  buf[0] = 5; /* version */
  buf[1] = SOCKS_COMMAND_CONNECT; /* command */
  buf[2] = 0; /* reserved */

  if (tor_addr_family(&conn->addr) == AF_INET) {
    uint32_t addr = tor_addr_to_ipv4n(&conn->addr);

    buf[3] = 1;
    reqsize += 4;
    memcpy(buf + 4, &addr, 4);
    memcpy(buf + 8, &port, 2);
  } else { /* AF_INET6 */
    buf[3] = 4;
    reqsize += 16;
    memcpy(buf + 4, tor_addr_to_in6_addr8(&conn->addr), 16);
    memcpy(buf + 20, &port, 2);
  }

  connection_buf_add((char *)buf, reqsize, conn);

  conn->proxy_state = PROXY_SOCKS5_WANT_CONNECT_OK;
}

/** Wrapper around fetch_from_buf_socks_client: see that functions
 * for documentation of its behavior. */
static int
connection_fetch_from_buf_socks_client(connection_t *conn,
                                       int state, char **reason)
{
  return fetch_from_buf_socks_client(conn->inbuf, state, reason);
}

/** Call this from connection_*_process_inbuf() to advance the proxy
 * handshake.
 *
 * No matter what proxy protocol is used, if this function returns 1, the
 * handshake is complete, and the data remaining on inbuf may contain the
 * start of the communication with the requested server.
 *
 * Returns 0 if the current buffer contains an incomplete response, and -1
 * on error.
 */
int
connection_read_proxy_handshake(connection_t *conn)
{
  int ret = 0;
  char *reason = NULL;

  log_debug(LD_NET, "enter state %s",
            connection_proxy_state_to_string(conn->proxy_state));

  switch (conn->proxy_state) {
    case PROXY_HTTPS_WANT_CONNECT_OK:
      ret = connection_read_https_proxy_response(conn);
      if (ret == 1)
        conn->proxy_state = PROXY_CONNECTED;
      break;

    case PROXY_SOCKS4_WANT_CONNECT_OK:
      ret = connection_fetch_from_buf_socks_client(conn,
                                                   conn->proxy_state,
                                                   &reason);
      if (ret == 1)
        conn->proxy_state = PROXY_CONNECTED;
      break;

    case PROXY_SOCKS5_WANT_AUTH_METHOD_NONE:
      ret = connection_fetch_from_buf_socks_client(conn,
                                                   conn->proxy_state,
                                                   &reason);
      /* no auth needed, do connect */
      if (ret == 1) {
        connection_send_socks5_connect(conn);
        ret = 0;
      }
      break;

    case PROXY_SOCKS5_WANT_AUTH_METHOD_RFC1929:
      ret = connection_fetch_from_buf_socks_client(conn,
                                                   conn->proxy_state,
                                                   &reason);

      /* send auth if needed, otherwise do connect */
      if (ret == 1) {
        connection_send_socks5_connect(conn);
        ret = 0;
      } else if (ret == 2) {
        unsigned char buf[1024];
        size_t reqsize, usize, psize;
        const char *user, *pass;
        char *socks_args_string = NULL;

        if (get_proxy_type() == PROXY_PLUGGABLE) {
          socks_args_string =
            pt_get_socks_args_for_proxy_addrport(&conn->addr, conn->port);
          if (!socks_args_string) {
            log_warn(LD_NET, "Could not create SOCKS args string.");
            ret = -1;
            break;
          }

          log_debug(LD_NET, "SOCKS5 arguments: %s", socks_args_string);
          tor_assert(strlen(socks_args_string) > 0);
          tor_assert(strlen(socks_args_string) <= MAX_SOCKS5_AUTH_SIZE_TOTAL);

          if (strlen(socks_args_string) > MAX_SOCKS5_AUTH_FIELD_SIZE) {
            user = socks_args_string;
            usize = MAX_SOCKS5_AUTH_FIELD_SIZE;
            pass = socks_args_string + MAX_SOCKS5_AUTH_FIELD_SIZE;
            psize = strlen(socks_args_string) - MAX_SOCKS5_AUTH_FIELD_SIZE;
          } else {
            user = socks_args_string;
            usize = strlen(socks_args_string);
            pass = "\0";
            psize = 1;
          }
        } else if (get_options()->Socks5ProxyUsername) {
          user = get_options()->Socks5ProxyUsername;
          pass = get_options()->Socks5ProxyPassword;
          tor_assert(user && pass);
          usize = strlen(user);
          psize = strlen(pass);
        } else {
          log_err(LD_BUG, "We entered %s for no reason!", __func__);
          tor_fragile_assert();
          ret = -1;
          break;
        }

        /* Username and password lengths should have been checked
           above and during torrc parsing. */
        tor_assert(usize <= MAX_SOCKS5_AUTH_FIELD_SIZE &&
                   psize <= MAX_SOCKS5_AUTH_FIELD_SIZE);
        reqsize = 3 + usize + psize;

        buf[0] = 1; /* negotiation version */
        buf[1] = usize;
        memcpy(buf + 2, user, usize);
        buf[2 + usize] = psize;
        memcpy(buf + 3 + usize, pass, psize);

        if (socks_args_string)
          tor_free(socks_args_string);

        connection_buf_add((char *)buf, reqsize, conn);

        conn->proxy_state = PROXY_SOCKS5_WANT_AUTH_RFC1929_OK;
        ret = 0;
      }
      break;

    case PROXY_SOCKS5_WANT_AUTH_RFC1929_OK:
      ret = connection_fetch_from_buf_socks_client(conn,
                                                   conn->proxy_state,
                                                   &reason);
      /* send the connect request */
      if (ret == 1) {
        connection_send_socks5_connect(conn);
        ret = 0;
      }
      break;

    case PROXY_SOCKS5_WANT_CONNECT_OK:
      ret = connection_fetch_from_buf_socks_client(conn,
                                                   conn->proxy_state,
                                                   &reason);
      if (ret == 1)
        conn->proxy_state = PROXY_CONNECTED;
      break;

    default:
      log_err(LD_BUG, "Invalid proxy_state for reading, %d",
              conn->proxy_state);
      tor_fragile_assert();
      ret = -1;
      break;
  }

  log_debug(LD_NET, "leaving state %s",
            connection_proxy_state_to_string(conn->proxy_state));

  if (ret < 0) {
    if (reason) {
      log_warn(LD_NET, "Proxy Client: unable to connect to %s:%d (%s)",
                conn->address, conn->port, escaped(reason));
      tor_free(reason);
    } else {
      log_warn(LD_NET, "Proxy Client: unable to connect to %s:%d",
                conn->address, conn->port);
    }
  } else if (ret == 1) {
    log_info(LD_NET, "Proxy Client: connection to %s:%d successful",
              conn->address, conn->port);
  }

  return ret;
}

/** Given a list of listener connections in <b>old_conns</b>, and list of
 * port_cfg_t entries in <b>ports</b>, open a new listener for every port in
 * <b>ports</b> that does not already have a listener in <b>old_conns</b>.
 *
 * Remove from <b>old_conns</b> every connection that has a corresponding
 * entry in <b>ports</b>.  Add to <b>new_conns</b> new every connection we
 * launch.
 *
 * If <b>control_listeners_only</b> is true, then we only open control
 * listeners, and we do not remove any noncontrol listeners from old_conns.
 *
 * Return 0 on success, -1 on failure.
 **/
static int
retry_listener_ports(smartlist_t *old_conns,
                     const smartlist_t *ports,
                     smartlist_t *new_conns,
                     int control_listeners_only)
{
  smartlist_t *launch = smartlist_new();
  int r = 0;

  if (control_listeners_only) {
    SMARTLIST_FOREACH(ports, port_cfg_t *, p, {
        if (p->type == CONN_TYPE_CONTROL_LISTENER)
          smartlist_add(launch, p);
    });
  } else {
    smartlist_add_all(launch, ports);
  }

  /* Iterate through old_conns, comparing it to launch: remove from both lists
   * each pair of elements that corresponds to the same port. */
  SMARTLIST_FOREACH_BEGIN(old_conns, connection_t *, conn) {
    const port_cfg_t *found_port = NULL;

    /* Okay, so this is a listener.  Is it configured? */
    SMARTLIST_FOREACH_BEGIN(launch, const port_cfg_t *, wanted) {
      if (conn->type != wanted->type)
        continue;
      if ((conn->socket_family != AF_UNIX && wanted->is_unix_addr) ||
          (conn->socket_family == AF_UNIX && ! wanted->is_unix_addr))
        continue;

      if (wanted->server_cfg.no_listen)
        continue; /* We don't want to open a listener for this one */

      if (wanted->is_unix_addr) {
        if (conn->socket_family == AF_UNIX &&
            !strcmp(wanted->unix_addr, conn->address)) {
          found_port = wanted;
          break;
        }
      } else {
        int port_matches;
        if (wanted->port == CFG_AUTO_PORT) {
          port_matches = 1;
        } else {
          port_matches = (wanted->port == conn->port);
        }
        if (port_matches && tor_addr_eq(&wanted->addr, &conn->addr)) {
          found_port = wanted;
          break;
        }
      }
    } SMARTLIST_FOREACH_END(wanted);

    if (found_port) {
      /* This listener is already running; we don't need to launch it. */
      //log_debug(LD_NET, "Already have %s on %s:%d",
      //    conn_type_to_string(found_port->type), conn->address, conn->port);
      smartlist_remove(launch, found_port);
      /* And we can remove the connection from old_conns too. */
      SMARTLIST_DEL_CURRENT(old_conns, conn);
    }
  } SMARTLIST_FOREACH_END(conn);

  /* Now open all the listeners that are configured but not opened. */
  SMARTLIST_FOREACH_BEGIN(launch, const port_cfg_t *, port) {
    struct sockaddr *listensockaddr;
    socklen_t listensocklen = 0;
    char *address=NULL;
    connection_t *conn;
    int real_port = port->port == CFG_AUTO_PORT ? 0 : port->port;
    tor_assert(real_port <= UINT16_MAX);
    if (port->server_cfg.no_listen)
      continue;

#ifndef _WIN32
    /* We don't need to be root to create a UNIX socket, so defer until after
     * setuid. */
    const or_options_t *options = get_options();
    if (port->is_unix_addr && !geteuid() && (options->User) &&
        strcmp(options->User, "root"))
      continue;
#endif /* !defined(_WIN32) */

    if (port->is_unix_addr) {
      listensockaddr = (struct sockaddr *)
        create_unix_sockaddr(port->unix_addr,
                             &address, &listensocklen);
    } else {
      listensockaddr = tor_malloc(sizeof(struct sockaddr_storage));
      listensocklen = tor_addr_to_sockaddr(&port->addr,
                                           real_port,
                                           listensockaddr,
                                           sizeof(struct sockaddr_storage));
      address = tor_addr_to_str_dup(&port->addr);
    }

    if (listensockaddr) {
    	 // Only use quic for OR now, might change later
    	      if(port->type == CONN_TYPE_OR_LISTENER) {
    	        conn = connection_listener_new_quic(listensockaddr, listensocklen,
    	                                     port->type, address, port);

    	      } else {
    	        conn = connection_listener_new(listensockaddr, listensocklen,
    	                                     port->type, address, port);
    	      }
      tor_free(listensockaddr);
      tor_free(address);
    } else {
      conn = NULL;
    }

    if (!conn) {
      r = -1;
    } else {
      if (new_conns)
        smartlist_add(new_conns, conn);
    }
  } SMARTLIST_FOREACH_END(port);

  smartlist_free(launch);

  return r;
}

/** Launch listeners for each port you should have open.  Only launch
 * listeners who are not already open, and only close listeners we no longer
 * want.
 *
 * Add all old conns that should be closed to <b>replaced_conns</b>.
 * Add all new connections to <b>new_conns</b>.
 *
 * If <b>close_all_noncontrol</b> is true, then we only open control
 * listeners, and we close all other listeners.
 */
int
retry_all_listeners(smartlist_t *replaced_conns,
                    smartlist_t *new_conns, int close_all_noncontrol)
{
  smartlist_t *listeners = smartlist_new();
  const or_options_t *options = get_options();
  int retval = 0;
  const uint16_t old_or_port = router_get_advertised_or_port(options);
  const uint16_t old_or_port_ipv6 =
    router_get_advertised_or_port_by_af(options,AF_INET6);
  const uint16_t old_dir_port = router_get_advertised_dir_port(options, 0);

  SMARTLIST_FOREACH_BEGIN(get_connection_array(), connection_t *, conn) {
    if (connection_is_listener(conn) && !conn->marked_for_close)
      smartlist_add(listeners, conn);
  } SMARTLIST_FOREACH_END(conn);

  if (retry_listener_ports(listeners,
                           get_configured_ports(),
                           new_conns,
                           close_all_noncontrol) < 0)
    retval = -1;

  /* Any members that were still in 'listeners' don't correspond to
   * any configured port.  Kill 'em. */
  SMARTLIST_FOREACH_BEGIN(listeners, connection_t *, conn) {
    log_notice(LD_NET, "Closing no-longer-configured %s on %s:%d",
               conn_type_to_string(conn->type), conn->address, conn->port);
    if (replaced_conns) {
      smartlist_add(replaced_conns, conn);
    } else {
      connection_close_immediate(conn);
      connection_mark_for_close(conn);
    }
  } SMARTLIST_FOREACH_END(conn);

  smartlist_free(listeners);

  if (old_or_port != router_get_advertised_or_port(options) ||
      old_or_port_ipv6 != router_get_advertised_or_port_by_af(options,
                                                              AF_INET6) ||
      old_dir_port != router_get_advertised_dir_port(options, 0)) {
    /* Our chosen ORPort or DirPort is not what it used to be: the
     * descriptor we had (if any) should be regenerated.  (We won't
     * automatically notice this because of changes in the option,
     * since the value could be "auto".) */
    mark_my_descriptor_dirty("Chosen Or/DirPort changed");
  }

  return retval;
}

/** Mark every listener of type other than CONTROL_LISTENER to be closed. */
void
connection_mark_all_noncontrol_listeners(void)
{
  SMARTLIST_FOREACH_BEGIN(get_connection_array(), connection_t *, conn) {
    if (conn->marked_for_close)
      continue;
    if (conn->type == CONN_TYPE_CONTROL_LISTENER)
      continue;
    if (connection_is_listener(conn))
      connection_mark_for_close(conn);
  } SMARTLIST_FOREACH_END(conn);
}

/** Mark every external connection not used for controllers for close. */
void
connection_mark_all_noncontrol_connections(void)
{
  SMARTLIST_FOREACH_BEGIN(get_connection_array(), connection_t *, conn) {
    if (conn->marked_for_close)
      continue;
    switch (conn->type) {
      case CONN_TYPE_CONTROL_LISTENER:
      case CONN_TYPE_CONTROL:
        break;
      case CONN_TYPE_AP:
        connection_mark_unattached_ap(TO_ENTRY_CONN(conn),
                                      END_STREAM_REASON_HIBERNATING);
        break;
      case CONN_TYPE_OR:
        {
          or_connection_t *orconn = TO_OR_CONN(conn);
          if (orconn->chan) {
            connection_or_close_normally(orconn, 0);
          } else {
            /*
             * There should have been one, but mark for close and hope
             * for the best..
             */
            connection_mark_for_close(conn);
          }
        }
        break;
      default:
        connection_mark_for_close(conn);
        break;
    }
  } SMARTLIST_FOREACH_END(conn);
}

/** Return 1 if we should apply rate limiting to <b>conn</b>, and 0
 * otherwise.
 * Right now this just checks if it's an internal IP address or an
 * internal connection. We also should, but don't, check if the connection
 * uses pluggable transports, since we should then limit it even if it
 * comes from an internal IP address. */
static int
connection_is_rate_limited(connection_t *conn)
{
  const or_options_t *options = get_options();
  if (conn->linked)
    return 0; /* Internal connection */
  else if (! options->CountPrivateBandwidth &&
           (tor_addr_family(&conn->addr) == AF_UNSPEC || /* no address */
            tor_addr_family(&conn->addr) == AF_UNIX ||   /* no address */
            tor_addr_is_internal(&conn->addr, 0)))
    return 0; /* Internal address */
  else
    return 1;
}

/** Did either global write bucket run dry last second? If so,
 * we are likely to run dry again this second, so be stingy with the
 * tokens we just put in. */
static int write_buckets_empty_last_second = 0;

/** How many seconds of no active local circuits will make the
 * connection revert to the "relayed" bandwidth class? */
#define CLIENT_IDLE_TIME_FOR_PRIORITY 30

/** Return 1 if <b>conn</b> should use tokens from the "relayed"
 * bandwidth rates, else 0. Currently, only OR conns with bandwidth
 * class 1, and directory conns that are serving data out, count.
 */
static int
connection_counts_as_relayed_traffic(connection_t *conn, time_t now)
{
  if (conn->type == CONN_TYPE_OR &&
      connection_or_client_used(TO_OR_CONN(conn)) +
                                CLIENT_IDLE_TIME_FOR_PRIORITY < now)
    return 1;
  if (conn->type == CONN_TYPE_DIR && DIR_CONN_IS_SERVER(conn))
    return 1;
  return 0;
}

/** Helper function to decide how many bytes out of <b>global_bucket</b>
 * we're willing to use for this transaction. <b>base</b> is the size
 * of a cell on the network; <b>priority</b> says whether we should
 * write many of them or just a few; and <b>conn_bucket</b> (if
 * non-negative) provides an upper limit for our answer. */
static ssize_t
connection_bucket_round_robin(int base, int priority,
                              ssize_t global_bucket_val, ssize_t conn_bucket)
{
  ssize_t at_most;
  ssize_t num_bytes_high = (priority ? 32 : 16) * base;
  ssize_t num_bytes_low = (priority ? 4 : 2) * base;

  /* Do a rudimentary round-robin so one circuit can't hog a connection.
   * Pick at most 32 cells, at least 4 cells if possible, and if we're in
   * the middle pick 1/8 of the available bandwidth. */
  at_most = global_bucket_val / 8;
  at_most -= (at_most % base); /* round down */
  if (at_most > num_bytes_high) /* 16 KB, or 8 KB for low-priority */
    at_most = num_bytes_high;
  else if (at_most < num_bytes_low) /* 2 KB, or 1 KB for low-priority */
    at_most = num_bytes_low;

  if (at_most > global_bucket_val)
    at_most = global_bucket_val;

  if (conn_bucket >= 0 && at_most > conn_bucket)
    at_most = conn_bucket;

  if (at_most < 0)
    return 0;
  return at_most;
}

/** How many bytes at most can we read onto this connection? */
static ssize_t
connection_bucket_read_limit(connection_t *conn, time_t now)
{
  int base = RELAY_PAYLOAD_SIZE;
  int priority = conn->type != CONN_TYPE_DIR;
  ssize_t conn_bucket = -1;
  size_t global_bucket_val = token_bucket_rw_get_read(&global_bucket);

  if (connection_speaks_cells(conn)) {
    or_connection_t *or_conn = TO_OR_CONN(conn);
    if (conn->state == OR_CONN_STATE_OPEN)
      conn_bucket = token_bucket_rw_get_read(&or_conn->bucket);
    base = get_cell_network_size(or_conn->wide_circ_ids);
  }

  if (!connection_is_rate_limited(conn)) {
    /* be willing to read on local conns even if our buckets are empty */
    return conn_bucket>=0 ? conn_bucket : 1<<14;
  }

  if (connection_counts_as_relayed_traffic(conn, now)) {
    size_t relayed = token_bucket_rw_get_read(&global_relayed_bucket);
    global_bucket_val = MIN(global_bucket_val, relayed);
  }

  return connection_bucket_round_robin(base, priority,
                                       global_bucket_val, conn_bucket);
}

/** How many bytes at most can we write onto this connection? */
ssize_t
connection_bucket_write_limit(connection_t *conn, time_t now)
{
  int base = RELAY_PAYLOAD_SIZE;
  int priority = conn->type != CONN_TYPE_DIR;
  size_t conn_bucket = conn->outbuf_flushlen;
  size_t global_bucket_val = token_bucket_rw_get_write(&global_bucket);

  if (!connection_is_rate_limited(conn)) {
    /* be willing to write to local conns even if our buckets are empty */
    return conn->outbuf_flushlen;
  }

  if (connection_speaks_cells(conn)) {
    /* use the per-conn write limit if it's lower */
    or_connection_t *or_conn = TO_OR_CONN(conn);
    if (conn->state == OR_CONN_STATE_OPEN)
      conn_bucket = MIN(conn_bucket,
                        token_bucket_rw_get_write(&or_conn->bucket));
    base = get_cell_network_size(or_conn->wide_circ_ids);
  }

  if (connection_counts_as_relayed_traffic(conn, now)) {
    size_t relayed = token_bucket_rw_get_write(&global_relayed_bucket);
    global_bucket_val = MIN(global_bucket_val, relayed);
  }

  return connection_bucket_round_robin(base, priority,
                                       global_bucket_val, conn_bucket);
}

/** Return 1 if the global write buckets are low enough that we
 * shouldn't send <b>attempt</b> bytes of low-priority directory stuff
 * out to <b>conn</b>. Else return 0.

 * Priority was 1 for v1 requests (directories and running-routers),
 * and 2 for v2 requests and later (statuses and descriptors).
 *
 * There are a lot of parameters we could use here:
 * - global_relayed_write_bucket. Low is bad.
 * - global_write_bucket. Low is bad.
 * - bandwidthrate. Low is bad.
 * - bandwidthburst. Not a big factor?
 * - attempt. High is bad.
 * - total bytes queued on outbufs. High is bad. But I'm wary of
 *   using this, since a few slow-flushing queues will pump up the
 *   number without meaning what we meant to mean. What we really
 *   mean is "total directory bytes added to outbufs recently", but
 *   that's harder to quantify and harder to keep track of.
 */
int
global_write_bucket_low(connection_t *conn, size_t attempt, int priority)
{
  size_t smaller_bucket =
    MIN(token_bucket_rw_get_write(&global_bucket),
        token_bucket_rw_get_write(&global_relayed_bucket));
  if (authdir_mode(get_options()) && priority>1)
    return 0; /* there's always room to answer v2 if we're an auth dir */

  if (!connection_is_rate_limited(conn))
    return 0; /* local conns don't get limited */

  if (smaller_bucket < attempt)
    return 1; /* not enough space no matter the priority */

  if (write_buckets_empty_last_second)
    return 1; /* we're already hitting our limits, no more please */

  if (priority == 1) { /* old-style v1 query */
    /* Could we handle *two* of these requests within the next two seconds? */
    const or_options_t *options = get_options();
    size_t can_write = (size_t) (smaller_bucket
      + 2*(options->RelayBandwidthRate ? options->RelayBandwidthRate :
           options->BandwidthRate));
    if (can_write < 2*attempt)
      return 1;
  } else { /* v2 query */
    /* no further constraints yet */
  }
  return 0;
}

/** Helper: adjusts our bandwidth history and informs the controller as
 * appropriate, given that we have just read <b>num_read</b> bytes and written
 * <b>num_written</b> bytes on <b>conn</b>. */
static void
record_num_bytes_transferred_impl(connection_t *conn,
                             time_t now, size_t num_read, size_t num_written)
{
  /* Count bytes of answering direct and tunneled directory requests */
  if (conn->type == CONN_TYPE_DIR && conn->purpose == DIR_PURPOSE_SERVER) {
    if (num_read > 0)
      rep_hist_note_dir_bytes_read(num_read, now);
    if (num_written > 0)
      rep_hist_note_dir_bytes_written(num_written, now);
  }

  if (!connection_is_rate_limited(conn))
    return; /* local IPs are free */

  if (conn->type == CONN_TYPE_OR)
    rep_hist_note_or_conn_bytes(conn->global_identifier, num_read,
                                num_written, now);

  if (num_read > 0) {
    rep_hist_note_bytes_read(num_read, now);
  }
  if (num_written > 0) {
    rep_hist_note_bytes_written(num_written, now);
  }
  if (conn->type == CONN_TYPE_EXIT)
    rep_hist_note_exit_bytes(conn->port, num_written, num_read);
}

/** We just read <b>num_read</b> and wrote <b>num_written</b> bytes
 * onto <b>conn</b>. Decrement buckets appropriately. */
static void
connection_buckets_decrement(connection_t *conn, time_t now,
                             size_t num_read, size_t num_written)
{
  if (num_written >= INT_MAX || num_read >= INT_MAX) {
    log_err(LD_BUG, "Value out of range. num_read=%lu, num_written=%lu, "
             "connection type=%s, state=%s",
             (unsigned long)num_read, (unsigned long)num_written,
             conn_type_to_string(conn->type),
             conn_state_to_string(conn->type, conn->state));
    tor_assert_nonfatal_unreached();
    if (num_written >= INT_MAX)
      num_written = 1;
    if (num_read >= INT_MAX)
      num_read = 1;
  }

  record_num_bytes_transferred_impl(conn, now, num_read, num_written);

  if (!connection_is_rate_limited(conn))
    return; /* local IPs are free */

  if (connection_counts_as_relayed_traffic(conn, now)) {
    token_bucket_rw_dec(&global_relayed_bucket, num_read, num_written);
  }
  token_bucket_rw_dec(&global_bucket, num_read, num_written);
  if (connection_speaks_cells(conn) && conn->state == OR_CONN_STATE_OPEN) {
    or_connection_t *or_conn = TO_OR_CONN(conn);
    token_bucket_rw_dec(&or_conn->bucket, num_read, num_written);
  }
}

/** If we have exhausted our global buckets, or the buckets for conn,
 * stop reading. */
static void
connection_consider_empty_read_buckets(connection_t *conn)
{
  const char *reason;

  if (!connection_is_rate_limited(conn))
    return; /* Always okay. */

  if (token_bucket_rw_get_read(&global_bucket) <= 0) {
    reason = "global read bucket exhausted. Pausing.";
  } else if (connection_counts_as_relayed_traffic(conn, approx_time()) &&
             token_bucket_rw_get_read(&global_relayed_bucket) <= 0) {
    reason = "global relayed read bucket exhausted. Pausing.";
  } else if (connection_speaks_cells(conn) &&
             conn->state == OR_CONN_STATE_OPEN &&
             token_bucket_rw_get_read(&TO_OR_CONN(conn)->bucket) <= 0) {
    reason = "connection read bucket exhausted. Pausing.";
  } else
    return; /* all good, no need to stop it */

  LOG_FN_CONN(conn, (LOG_DEBUG, LD_NET, "%s", reason));
  conn->read_blocked_on_bw = 1;
  connection_stop_reading(conn);
}

/** If we have exhausted our global buckets, or the buckets for conn,
 * stop writing. */
static void
connection_consider_empty_write_buckets(connection_t *conn)
{
  const char *reason;

  if (!connection_is_rate_limited(conn))
    return; /* Always okay. */

  if (token_bucket_rw_get_write(&global_bucket) <= 0) {
    reason = "global write bucket exhausted. Pausing.";
  } else if (connection_counts_as_relayed_traffic(conn, approx_time()) &&
             token_bucket_rw_get_write(&global_relayed_bucket) <= 0) {
    reason = "global relayed write bucket exhausted. Pausing.";
  } else if (connection_speaks_cells(conn) &&
             conn->state == OR_CONN_STATE_OPEN &&
             token_bucket_rw_get_write(&TO_OR_CONN(conn)->bucket) <= 0) {
    reason = "connection write bucket exhausted. Pausing.";
  } else
    return; /* all good, no need to stop it */

  LOG_FN_CONN(conn, (LOG_DEBUG, LD_NET, "%s", reason));
  conn->write_blocked_on_bw = 1;
  connection_stop_writing(conn);
}

/** Initialize the global buckets to the values configured in the
 * options */
void
connection_bucket_init(void)
{
  const or_options_t *options = get_options();
  const uint32_t now_ts = monotime_coarse_get_stamp();
  token_bucket_rw_init(&global_bucket,
                    (int32_t)options->BandwidthRate,
                    (int32_t)options->BandwidthBurst,
                    now_ts);
  if (options->RelayBandwidthRate) {
    token_bucket_rw_init(&global_relayed_bucket,
                      (int32_t)options->RelayBandwidthRate,
                      (int32_t)options->RelayBandwidthBurst,
                      now_ts);
  } else {
    token_bucket_rw_init(&global_relayed_bucket,
                      (int32_t)options->BandwidthRate,
                      (int32_t)options->BandwidthBurst,
                      now_ts);
  }
}

/** Update the global connection bucket settings to a new value. */
void
connection_bucket_adjust(const or_options_t *options)
{
  token_bucket_rw_adjust(&global_bucket,
                      (int32_t)options->BandwidthRate,
                      (int32_t)options->BandwidthBurst);
  if (options->RelayBandwidthRate) {
    token_bucket_rw_adjust(&global_relayed_bucket,
                        (int32_t)options->RelayBandwidthRate,
                        (int32_t)options->RelayBandwidthBurst);
  } else {
    token_bucket_rw_adjust(&global_relayed_bucket,
                        (int32_t)options->BandwidthRate,
                        (int32_t)options->BandwidthBurst);
  }
}

/** Time has passed; increment buckets appropriately. */
void
connection_bucket_refill(time_t now, uint32_t now_ts)
{
  smartlist_t *conns = get_connection_array();

  write_buckets_empty_last_second =
    token_bucket_rw_get_write(&global_bucket) <= 0 ||
    token_bucket_rw_get_write(&global_relayed_bucket) <= 0;

  /* refill the global buckets */
  token_bucket_rw_refill(&global_bucket, now_ts);
  token_bucket_rw_refill(&global_relayed_bucket, now_ts);

  /* refill the per-connection buckets */
  SMARTLIST_FOREACH_BEGIN(conns, connection_t *, conn) {
    if (connection_speaks_cells(conn)) {
      or_connection_t *or_conn = TO_OR_CONN(conn);

      if (conn->state == OR_CONN_STATE_OPEN) {
        token_bucket_rw_refill(&or_conn->bucket, now_ts);
      }
    }

    if (conn->read_blocked_on_bw == 1 /* marked to turn reading back on now */
        && token_bucket_rw_get_read(&global_bucket) > 0 /* and we can read */
        && (!connection_counts_as_relayed_traffic(conn, now) ||
            token_bucket_rw_get_read(&global_relayed_bucket) > 0)
        && (!connection_speaks_cells(conn) ||
            conn->state != OR_CONN_STATE_OPEN ||
            token_bucket_rw_get_read(&TO_OR_CONN(conn)->bucket) > 0)) {
        /* and either a non-cell conn or a cell conn with non-empty bucket */
      LOG_FN_CONN(conn, (LOG_DEBUG,LD_NET,
                         "waking up conn (fd %d) for read", (int)conn->s));
      conn->read_blocked_on_bw = 0;
      connection_start_reading(conn);
    }

    if (conn->write_blocked_on_bw == 1
        && token_bucket_rw_get_write(&global_bucket) > 0 /* and we can write */
        && (!connection_counts_as_relayed_traffic(conn, now) ||
            token_bucket_rw_get_write(&global_relayed_bucket) > 0)
        && (!connection_speaks_cells(conn) ||
            conn->state != OR_CONN_STATE_OPEN ||
            token_bucket_rw_get_write(&TO_OR_CONN(conn)->bucket) > 0)) {
      LOG_FN_CONN(conn, (LOG_DEBUG,LD_NET,
                         "waking up conn (fd %d) for write", (int)conn->s));
      conn->write_blocked_on_bw = 0;
      connection_start_writing(conn);
    }
  } SMARTLIST_FOREACH_END(conn);
}

/** Read bytes from conn-\>s and process them.
 *
 * It calls connection_buf_read_from_socket() to bring in any new bytes,
 * and then calls connection_process_inbuf() to process them.
 *
 * Mark the connection and return -1 if you want to close it, else
 * return 0.
 */
static int
connection_handle_read_impl(connection_t *conn)
{
  ssize_t max_to_read=-1, try_to_read;
  size_t before, n_read = 0;
  int socket_error = 0;

  if (conn->marked_for_close)
    return 0; /* do nothing */

  conn->timestamp_last_read_allowed = approx_time();

  switch (conn->type) {
  case CONN_TYPE_OR_LISTENER:
        return connection_handle_listener_read_quic(conn, CONN_TYPE_OR);
      case CONN_TYPE_EXT_OR_LISTENER:
        return connection_handle_listener_read_quic(conn, CONN_TYPE_EXT_OR);
      case CONN_TYPE_AP_LISTENER:
      case CONN_TYPE_AP_TRANS_LISTENER:
      case CONN_TYPE_AP_NATD_LISTENER:
        return connection_handle_listener_read(conn, CONN_TYPE_AP);
      case CONN_TYPE_DIR_LISTENER:
        return connection_handle_listener_read(conn, CONN_TYPE_DIR);
      case CONN_TYPE_CONTROL_LISTENER:
        return connection_handle_listener_read(conn, CONN_TYPE_CONTROL);
      case CONN_TYPE_AP_DNS_LISTENER:
        /* This should never happen; eventdns.c handles the reads here. */
        tor_fragile_assert();

      return 0;
  }

 loop_again:
  try_to_read = max_to_read;
  tor_assert(!conn->marked_for_close);

  before = buf_datalen(conn->inbuf);
  int result = 0;
  if (conn->use_quic) {
      result = connection_read_to_buf_quic(conn, &max_to_read, &socket_error);
    } else {connection_buf_read_from_socket(conn, &max_to_read, &socket_error); }
  if( result < 0){
    /* There's a read error; kill the connection.*/
    if (conn->type == CONN_TYPE_OR) {
      connection_or_notify_error(TO_OR_CONN(conn),
                                 socket_error != 0 ?
                                   errno_to_orconn_end_reason(socket_error) :
                                   END_OR_CONN_REASON_CONNRESET,
                                 socket_error != 0 ?
                                   tor_socket_strerror(socket_error) :
                                   "(unknown, errno was 0)");
    }
    if (CONN_IS_EDGE(conn)) {
      edge_connection_t *edge_conn = TO_EDGE_CONN(conn);
      connection_edge_end_errno(edge_conn);
      if (conn->type == CONN_TYPE_AP && TO_ENTRY_CONN(conn)->socks_request) {
        /* broken, don't send a socks reply back */
        TO_ENTRY_CONN(conn)->socks_request->has_finished = 1;
      }
    }
    connection_close_immediate(conn); /* Don't flush; connection is dead. */
    /*
     * This can bypass normal channel checking since we did
     * connection_or_notify_error() above.
     */
    connection_mark_for_close_internal(conn);
    return -1;
  }
  n_read += buf_datalen(conn->inbuf) - before;
  if (CONN_IS_EDGE(conn) && try_to_read != max_to_read) {
    /* instruct it not to try to package partial cells. */
    if (connection_process_inbuf(conn, 0) < 0) {
      return -1;
    }
    if (!conn->marked_for_close &&
        connection_is_reading(conn) &&
        !conn->inbuf_reached_eof &&
        max_to_read > 0)
      goto loop_again; /* try reading again, in case more is here now */
  }
  /* one last try, packaging partial cells and all. */
  if (!conn->marked_for_close &&
      connection_process_inbuf(conn, 1) < 0) {
    return -1;
  }
  if (conn->linked_conn) {
    /* The other side's handle_write() will never actually get called, so
     * we need to invoke the appropriate callbacks ourself. */
    connection_t *linked = conn->linked_conn;

    if (n_read) {
      /* Probably a no-op, since linked conns typically don't count for
       * bandwidth rate limiting. But do it anyway so we can keep stats
       * accurately. Note that since we read the bytes from conn, and
       * we're writing the bytes onto the linked connection, we count
       * these as <i>written</i> bytes. */
      connection_buckets_decrement(linked, approx_time(), 0, n_read);

      if (connection_flushed_some(linked) < 0)
        connection_mark_for_close(linked);
      if (!connection_wants_to_flush(linked))
        connection_finished_flushing(linked);
    }

    if (!buf_datalen(linked->outbuf) && conn->active_on_link)
      connection_stop_reading_from_linked_conn(conn);
  }
  /* If we hit the EOF, call connection_reached_eof(). */
  if (!conn->marked_for_close &&
      conn->inbuf_reached_eof &&
      connection_reached_eof(conn) < 0) {
    return -1;
  }
  return 0;
}

/* DOCDOC connection_handle_read */
int
connection_handle_read(connection_t *conn)
{
  int res;

  tor_gettimeofday_cache_clear();
  res = connection_handle_read_impl(conn);
  return res;
}


// QUIC read main code
static int
connection_read_to_buf_quic(connection_t *conn, ssize_t *max_to_read,
                       int *socket_error)
{
  int result;
  ssize_t at_most = *max_to_read;
  size_t slack_in_buf, more_to_read;
  size_t n_read = 0;

  if (at_most == -1) { /* we need to initialize it */
    /* how many bytes are we allowed to read? */
    at_most = connection_bucket_read_limit(conn, approx_time());
  }

  tor_assert(QUICSOCK_OK(conn->q_sock));
  slack_in_buf = buf_slack(conn->inbuf);

 again:
  if ((size_t)at_most > slack_in_buf && slack_in_buf >= 1024) {
    more_to_read = at_most - slack_in_buf;
    at_most = slack_in_buf;
  } else {
    more_to_read = 0;
  }

  // TODO Can we have a linked quic connection?
  tor_assert(!conn->linked);

  //if (conn->linked) {
  //  if (conn->linked_conn) {
  //    result = move_buf_to_buf(conn->inbuf, conn->linked_conn->outbuf,
  //                             &conn->linked_conn->outbuf_flushlen);
  //  } else {
  //    result = 0;
  //  }
  //  log_notice(LD_GENERAL, "Moved %d bytes on an internal link!", result);
  //  /* If the other side has disappeared, or if it's been marked for close and
  //   * we flushed its outbuf, then we should set our inbuf_reached_eof. */
  //  if (!conn->linked_conn ||
  //      (conn->linked_conn->marked_for_close &&
  //       buf_datalen(conn->linked_conn->outbuf) == 0))
  //    conn->inbuf_reached_eof = 1;

  //  n_read = (size_t) result;
  //
  //} else {
  //}

  tor_assert(conn->type == CONN_TYPE_OR);
  //tor_assert(conn->state >= OR_CONN_STATE_CONNECTING);
  //or_connection_t *or_conn = TO_OR_CONN(conn);
  int reached_eof = 0;

  log_debug(LD_NET,
            "%d: starting, inbuf_datalen %ld, at_most %ld.",
            (int)qs_get_fd(conn->q_sock),(long)buf_datalen(conn->inbuf),
            (long)at_most);

  /* else open, or closing */
  result = read_to_buf_quic(conn->q_sock, at_most, conn->inbuf, &reached_eof,
                            socket_error);
  /* !connection_speaks_cells, !conn->linked_conn. */

  if (reached_eof)
    conn->inbuf_reached_eof = 1;

  log_debug(LD_OR,"read_to_buf returned %d.",result);

  if (result < 0)
    return -1;
  n_read = (size_t) result;

  if (n_read > 0) {
     /* change *max_to_read */
    *max_to_read = at_most - n_read;

    /* If CONN_BW events are enabled, update conn->n_read_conn_bw for
     * OR/DIR/EXIT connections, checking for overflow. */
    if (get_options()->TestingEnableConnBwEvent){
      if (PREDICT_LIKELY(UINT32_MAX - conn->n_read_conn_bw > n_read))
        conn->n_read_conn_bw += (int)n_read;
      else
        conn->n_read_conn_bw = UINT32_MAX;
    }
  }

  connection_buckets_decrement(conn, approx_time(), n_read, 0);

  if (more_to_read && result == at_most) {
    slack_in_buf = buf_slack(conn->inbuf);
    at_most = more_to_read;
    goto again;
  }

  /* Call even if result is 0, since the global read bucket may
   * have reached 0 on a different conn, and this guy needs to
   * know to stop reading. */
  connection_consider_empty_read_buckets(conn);
    
  return 0;
}

/** Pull in new bytes from conn-\>s or conn-\>linked_conn onto conn-\>inbuf,
 * either directly or via TLS. Reduce the token buckets by the number of bytes
 * read.
 *
 * If *max_to_read is -1, then decide it ourselves, else go with the
 * value passed to us. When returning, if it's changed, subtract the
 * number of bytes we read from *max_to_read.
 *
 * Return -1 if we want to break conn, else return 0.
 */
static int
connection_buf_read_from_socket(connection_t *conn, ssize_t *max_to_read,
                       int *socket_error)
{
  int result;
  ssize_t at_most = *max_to_read;
  size_t slack_in_buf, more_to_read;
  size_t n_read = 0, n_written = 0;

  if (at_most == -1) { /* we need to initialize it */
    /* how many bytes are we allowed to read? */
    at_most = connection_bucket_read_limit(conn, approx_time());
  }

  slack_in_buf = buf_slack(conn->inbuf);
 again:
  if ((size_t)at_most > slack_in_buf && slack_in_buf >= 1024) {
    more_to_read = at_most - slack_in_buf;
    at_most = slack_in_buf;
  } else {
    more_to_read = 0;
  }

  if (connection_speaks_cells(conn) &&
      conn->state > OR_CONN_STATE_PROXY_HANDSHAKING) {
    int pending;
    or_connection_t *or_conn = TO_OR_CONN(conn);
    size_t initial_size;
    if (conn->state == OR_CONN_STATE_TLS_HANDSHAKING ||
        conn->state == OR_CONN_STATE_TLS_CLIENT_RENEGOTIATING) {
      /* continue handshaking even if global token bucket is empty */
      return connection_tls_continue_handshake(or_conn);
    }

    log_debug(LD_NET,
              "%d: starting, inbuf_datalen %ld (%d pending in tls object)."
              " at_most %ld.",
              (int)conn->s,(long)buf_datalen(conn->inbuf),
              tor_tls_get_pending_bytes(or_conn->tls), (long)at_most);

    initial_size = buf_datalen(conn->inbuf);
    /* else open, or closing */
    result = buf_read_from_tls(conn->inbuf, or_conn->tls, at_most);
    if (TOR_TLS_IS_ERROR(result) || result == TOR_TLS_CLOSE)
      or_conn->tls_error = result;
    else
      or_conn->tls_error = 0;

    switch (result) {
      case TOR_TLS_CLOSE:
      case TOR_TLS_ERROR_IO:
        log_debug(LD_NET,"TLS connection closed %son read. Closing. "
                 "(Nickname %s, address %s)",
                 result == TOR_TLS_CLOSE ? "cleanly " : "",
                 or_conn->nickname ? or_conn->nickname : "not set",
                 conn->address);
        return result;
      CASE_TOR_TLS_ERROR_ANY_NONIO:
        log_debug(LD_NET,"tls error [%s]. breaking (nickname %s, address %s).",
                 tor_tls_err_to_string(result),
                 or_conn->nickname ? or_conn->nickname : "not set",
                 conn->address);
        return result;
      case TOR_TLS_WANTWRITE:
        connection_start_writing(conn);
        return 0;
      case TOR_TLS_WANTREAD:
        if (conn->in_connection_handle_write) {
          /* We've been invoked from connection_handle_write, because we're
           * waiting for a TLS renegotiation, the renegotiation started, and
           * SSL_read returned WANTWRITE.  But now SSL_read is saying WANTREAD
           * again.  Stop waiting for write events now, or else we'll
           * busy-loop until data arrives for us to read. */
          connection_stop_writing(conn);
          if (!connection_is_reading(conn))
            connection_start_reading(conn);
        }
        /* we're already reading, one hopes */
        break;
      case TOR_TLS_DONE: /* no data read, so nothing to process */
        break; /* so we call bucket_decrement below */
      default:
        break;
    }
    pending = tor_tls_get_pending_bytes(or_conn->tls);
    if (pending) {
      /* If we have any pending bytes, we read them now.  This *can*
       * take us over our read allotment, but really we shouldn't be
       * believing that SSL bytes are the same as TCP bytes anyway. */
      int r2 = buf_read_from_tls(conn->inbuf, or_conn->tls, pending);
      if (BUG(r2<0)) {
        log_warn(LD_BUG, "apparently, reading pending bytes can fail.");
        return -1;
      }
    }
    result = (int)(buf_datalen(conn->inbuf)-initial_size);
    tor_tls_get_n_raw_bytes(or_conn->tls, &n_read, &n_written);
    log_debug(LD_GENERAL, "After TLS read of %d: %ld read, %ld written",
              result, (long)n_read, (long)n_written);
  } else if (conn->linked) {
    if (conn->linked_conn) {
      result = buf_move_to_buf(conn->inbuf, conn->linked_conn->outbuf,
                               &conn->linked_conn->outbuf_flushlen);
    } else {
      result = 0;
    }
    //log_notice(LD_GENERAL, "Moved %d bytes on an internal link!", result);
    /* If the other side has disappeared, or if it's been marked for close and
     * we flushed its outbuf, then we should set our inbuf_reached_eof. */
    if (!conn->linked_conn ||
        (conn->linked_conn->marked_for_close &&
         buf_datalen(conn->linked_conn->outbuf) == 0))
      conn->inbuf_reached_eof = 1;

    n_read = (size_t) result;
  } else {
    /* !connection_speaks_cells, !conn->linked_conn. */
    int reached_eof = 0;
    CONN_LOG_PROTECT(conn,
                     result = buf_read_from_socket(conn->inbuf, conn->s,
                                                   at_most,
                                                   &reached_eof,
                                                   socket_error));
    if (reached_eof)
      conn->inbuf_reached_eof = 1;

//  log_fn(LOG_DEBUG,"read_to_buf returned %d.",read_result);

    if (result < 0)
      return -1;
    n_read = (size_t) result;
  }

  if (n_read > 0) {
     /* change *max_to_read */
    *max_to_read = at_most - n_read;

    /* Update edge_conn->n_read */
    if (conn->type == CONN_TYPE_AP) {
      edge_connection_t *edge_conn = TO_EDGE_CONN(conn);

      /* Check for overflow: */
      if (PREDICT_LIKELY(UINT32_MAX - edge_conn->n_read > n_read))
        edge_conn->n_read += (int)n_read;
      else
        edge_conn->n_read = UINT32_MAX;
    }

    /* If CONN_BW events are enabled, update conn->n_read_conn_bw for
     * OR/DIR/EXIT connections, checking for overflow. */
    if (get_options()->TestingEnableConnBwEvent &&
       (conn->type == CONN_TYPE_OR ||
        conn->type == CONN_TYPE_DIR ||
        conn->type == CONN_TYPE_EXIT)) {
      if (PREDICT_LIKELY(UINT32_MAX - conn->n_read_conn_bw > n_read))
        conn->n_read_conn_bw += (int)n_read;
      else
        conn->n_read_conn_bw = UINT32_MAX;
    }
  }

  connection_buckets_decrement(conn, approx_time(), n_read, n_written);

  if (more_to_read && result == at_most) {
    slack_in_buf = buf_slack(conn->inbuf);
    at_most = more_to_read;
    goto again;
  }

  /* Call even if result is 0, since the global read bucket may
   * have reached 0 on a different conn, and this connection needs to
   * know to stop reading. */
  connection_consider_empty_read_buckets(conn);
  if (n_written > 0 && connection_is_writing(conn))
    connection_consider_empty_write_buckets(conn);

  return 0;
}

/** A pass-through to fetch_from_buf. */
int
connection_buf_get_bytes(char *string, size_t len, connection_t *conn)
{
  return buf_get_bytes(conn->inbuf, string, len);
}

/** As buf_get_line(), but read from a connection's input buffer. */
int
connection_buf_get_line(connection_t *conn, char *data,
                               size_t *data_len)
{
  return buf_get_line(conn->inbuf, data, data_len);
}

/** As fetch_from_buf_http, but fetches from a connection's input buffer_t as
 * appropriate. */
int
connection_fetch_from_buf_http(connection_t *conn,
                               char **headers_out, size_t max_headerlen,
                               char **body_out, size_t *body_used,
                               size_t max_bodylen, int force_complete)
{
  return fetch_from_buf_http(conn->inbuf, headers_out, max_headerlen,
                             body_out, body_used, max_bodylen, force_complete);
}

/** Return conn-\>outbuf_flushlen: how many bytes conn wants to flush
 * from its outbuf. */
int
connection_wants_to_flush(connection_t *conn)
{
  return conn->outbuf_flushlen > 0;
}

/** Are there too many bytes on edge connection <b>conn</b>'s outbuf to
 * send back a relay-level sendme yet? Return 1 if so, 0 if not. Used by
 * connection_edge_consider_sending_sendme().
 */
int
connection_outbuf_too_full(connection_t *conn)
{
  return (conn->outbuf_flushlen > 10*CELL_PAYLOAD_SIZE);
}




static int
connection_handle_write_impl_quic(connection_t *conn, int force)
{
  int result = -1;
  ssize_t max_to_write;
  time_t now = approx_time();

  tor_assert(!connection_is_listener(conn));
  if (conn->marked_for_close || !QUICSOCK_OK(conn->q_sock)) {
    log_warn(LD_BUG, "marked for close or q_sock is not ok");
    return 0; /* do nothing */
  }

  if (conn->in_flushed_some) {
    log_warn(LD_BUG, "called recursively from inside conn->in_flushed_some");
    return 0;
  }

  conn->timestamp_lastwritten = now;

  /* Sometimes, "writable" means "connected". */
  if (connection_state_is_connecting(conn)) {
    /* The connection is successful. */
    if (connection_finished_connecting(conn)<0)
      return -1;
  }

  max_to_write = force ? (ssize_t)conn->outbuf_flushlen
    : connection_bucket_write_limit(conn, now);

  // make sure we can write on a OPEN connection only
  if (connection_speaks_cells(conn) &&
      conn->state > OR_CONN_STATE_PROXY_HANDSHAKING) {

    or_connection_t *or_conn = TO_OR_CONN(conn);

    log_debug(LD_OR, "Starting to write now want to send %d, outbuf_len %d", (int)max_to_write, (int)conn->outbuf_flushlen);

    result = flush_buf_quic(conn->q_sock, conn->outbuf, max_to_write);
    log_debug(LD_OR, "Finished writing, result %d", result);
    if (result < 0) {
      log_warn(LD_BUG, "QUIC sending failed on %d", qs_get_fd(conn->q_sock));
      connection_close_immediate(conn); /* Don't flush; connection is dead. */
      connection_mark_for_close(conn);
      return -1;
    }

    // record the #of bytes just flushed
    or_conn->bytes_xmitted += result;
    conn->outbuf_flushlen -= result;
  } else {
    log_warn(LD_BUG, "QUIC write used on non-OR type sock %d, type %d", qs_get_fd(conn->q_sock), conn->type);
    exit(-1);

  }

  connection_buckets_decrement(conn, approx_time(), 0, result);

  if (result > 0) {
    /* If we wrote any bytes from our buffer, then call the appropriate
     * functions. */
    if (connection_flushed_some(conn) < 0) {
      if (connection_speaks_cells(conn)) {
        connection_or_notify_error(TO_OR_CONN(conn),
                                   END_OR_CONN_REASON_MISC,
                                   "Got error back from "
                                   "connection_flushed_some()");
      }

      /*
       * This can bypass normal channel checking since we did
       * connection_or_notify_error() above.
       */
      connection_mark_for_close_internal(conn);
    }
  }

  if (!connection_wants_to_flush(conn)) { /* it's done flushing */
    log_debug(LD_OR, "this conn is done flushing -> no write event for a while");
    if (connection_finished_flushing(conn) < 0) {
      /* already marked */
      return -1;
    }

    return 0;
  }

  /* Call even if result is 0, since the global write bucket may
   * have reached 0 on a different conn, and this guy needs to
   * know to stop writing. */
  connection_consider_empty_write_buckets(conn);
  return 0;
}


/**
 * On Windows Vista and Windows 7, tune the send buffer size according to a
 * hint from the OS.
 *
 * This should help fix slow upload rates.
 */
static void

update_send_buffer_size(tor_socket_t sock)
{
#ifdef _WIN32
  /* We only do this on Vista and 7, because earlier versions of Windows
   * don't have the SIO_IDEAL_SEND_BACKLOG_QUERY functionality, and on
   * later versions it isn't necessary. */
  static int isVistaOr7 = -1;
  if (isVistaOr7 == -1) {
    isVistaOr7 = 0;
    OSVERSIONINFO osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion < 2)
      isVistaOr7 = 1;
  }
  if (!isVistaOr7)
    return;
  if (get_options()->ConstrainedSockets)
    return;
  ULONG isb = 0;
  DWORD bytesReturned = 0;
  if (!WSAIoctl(sock, SIO_IDEAL_SEND_BACKLOG_QUERY, NULL, 0,
      &isb, sizeof(isb), &bytesReturned, NULL, NULL)) {
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&isb, sizeof(isb));
  }
#else
  (void) sock;
#endif
}

/** Try to flush more bytes onto <b>conn</b>-\>s.
 *
 * This function gets called either from conn_write_callback() in main.c
 * when libevent tells us that conn wants to write, or below
 * from connection_buf_add() when an entire TLS record is ready.
 *
 * Update <b>conn</b>-\>timestamp_last_write_allowed to now, and call flush_buf
 * or flush_buf_tls appropriately. If it succeeds and there are no more
 * more bytes on <b>conn</b>-\>outbuf, then call connection_finished_flushing
 * on it too.
 *
 * If <b>force</b>, then write as many bytes as possible, ignoring bandwidth
 * limits.  (Used for flushing messages to controller connections on fatal
 * errors.)
 *
 * Mark the connection and return -1 if you want to close it, else
 * return 0.
 */
static int
connection_handle_write_impl(connection_t *conn, int force)
{
  int e;
  socklen_t len=(socklen_t)sizeof(e);
  int result;
  ssize_t max_to_write;
  time_t now = approx_time();
  size_t n_read = 0, n_written = 0;
  int dont_stop_writing = 0;

  tor_assert(!connection_is_listener(conn));

  if (conn->marked_for_close || !SOCKET_OK(conn->s))
    return 0; /* do nothing */

  if (conn->in_flushed_some) {
    log_warn(LD_BUG, "called recursively from inside conn->in_flushed_some");
    return 0;
  }

  conn->timestamp_last_write_allowed = now;

  /* Sometimes, "writable" means "connected". */
  if (connection_state_is_connecting(conn)) {
    if (getsockopt(conn->s, SOL_SOCKET, SO_ERROR, (void*)&e, &len) < 0) {
      log_warn(LD_BUG, "getsockopt() syscall failed");
      if (conn->type == CONN_TYPE_OR) {
        or_connection_t *orconn = TO_OR_CONN(conn);
        connection_or_close_for_error(orconn, 0);
      } else {
        if (CONN_IS_EDGE(conn)) {
          connection_edge_end_errno(TO_EDGE_CONN(conn));
        }
        connection_mark_for_close(conn);
      }
      return -1;
    }
    if (e) {
      /* some sort of error, but maybe just inprogress still */
      if (!ERRNO_IS_CONN_EINPROGRESS(e)) {
        log_info(LD_NET,"in-progress connect failed. Removing. (%s)",
                 tor_socket_strerror(e));
        if (CONN_IS_EDGE(conn))
          connection_edge_end_errno(TO_EDGE_CONN(conn));
        if (conn->type == CONN_TYPE_OR)
          connection_or_notify_error(TO_OR_CONN(conn),
                                     errno_to_orconn_end_reason(e),
                                     tor_socket_strerror(e));

        connection_close_immediate(conn);
        /*
         * This can bypass normal channel checking since we did
         * connection_or_notify_error() above.
         */
        connection_mark_for_close_internal(conn);
        return -1;
      } else {
        return 0; /* no change, see if next time is better */
      }
    }
    /* The connection is successful. */
    if (connection_finished_connecting(conn)<0)
      return -1;
  }

  max_to_write = force ? (ssize_t)conn->outbuf_flushlen
    : connection_bucket_write_limit(conn, now);

  if (connection_speaks_cells(conn) &&
      conn->state > OR_CONN_STATE_PROXY_HANDSHAKING) {
    or_connection_t *or_conn = TO_OR_CONN(conn);
    size_t initial_size;
    if (conn->state == OR_CONN_STATE_TLS_HANDSHAKING ||
        conn->state == OR_CONN_STATE_TLS_CLIENT_RENEGOTIATING) {
      connection_stop_writing(conn);
      if (connection_tls_continue_handshake(or_conn) < 0) {
        /* Don't flush; connection is dead. */
        connection_or_notify_error(or_conn,
                                   END_OR_CONN_REASON_MISC,
                                   "TLS error in connection_tls_"
                                   "continue_handshake()");
        connection_close_immediate(conn);
        /*
         * This can bypass normal channel checking since we did
         * connection_or_notify_error() above.
         */
        connection_mark_for_close_internal(conn);
        return -1;
      }
      return 0;
    } else if (conn->state == OR_CONN_STATE_TLS_SERVER_RENEGOTIATING) {
      return connection_handle_read(conn);
    }

    /* else open, or closing */
    initial_size = buf_datalen(conn->outbuf);
    result = buf_flush_to_tls(conn->outbuf, or_conn->tls,
                           max_to_write, &conn->outbuf_flushlen);

    if (result >= 0)
      update_send_buffer_size(conn->s);

    /* If we just flushed the last bytes, tell the channel on the
     * or_conn to check if it needs to geoip_change_dirreq_state() */
    /* XXXX move this to flushed_some or finished_flushing -NM */
    if (buf_datalen(conn->outbuf) == 0 && or_conn->chan)
      channel_notify_flushed(TLS_CHAN_TO_BASE(or_conn->chan));

    switch (result) {
      CASE_TOR_TLS_ERROR_ANY:
      case TOR_TLS_CLOSE:
        log_info(LD_NET, result != TOR_TLS_CLOSE ?
                 "tls error. breaking.":"TLS connection closed on flush");
        /* Don't flush; connection is dead. */
        connection_or_notify_error(or_conn,
                                   END_OR_CONN_REASON_MISC,
                                   result != TOR_TLS_CLOSE ?
                                     "TLS error in during flush" :
                                     "TLS closed during flush");
        connection_close_immediate(conn);
        /*
         * This can bypass normal channel checking since we did
         * connection_or_notify_error() above.
         */
        connection_mark_for_close_internal(conn);
        return -1;
      case TOR_TLS_WANTWRITE:
        log_debug(LD_NET,"wanted write.");
        /* we're already writing */
        dont_stop_writing = 1;
        break;
      case TOR_TLS_WANTREAD:
        /* Make sure to avoid a loop if the receive buckets are empty. */
        log_debug(LD_NET,"wanted read.");
        if (!connection_is_reading(conn)) {
          connection_stop_writing(conn);
          conn->write_blocked_on_bw = 1;
          /* we'll start reading again when we get more tokens in our
           * read bucket; then we'll start writing again too.
           */
        }
        /* else no problem, we're already reading */
        return 0;
      /* case TOR_TLS_DONE:
       * for TOR_TLS_DONE, fall through to check if the flushlen
       * is empty, so we can stop writing.
       */
    }

    tor_tls_get_n_raw_bytes(or_conn->tls, &n_read, &n_written);
    log_debug(LD_GENERAL, "After TLS write of %d: %ld read, %ld written",
              result, (long)n_read, (long)n_written);
    or_conn->bytes_xmitted += result;
    or_conn->bytes_xmitted_by_tls += n_written;
    /* So we notice bytes were written even on error */
    /* XXXX This cast is safe since we can never write INT_MAX bytes in a
     * single set of TLS operations. But it looks kinda ugly. If we refactor
     * the *_buf_tls functions, we should make them return ssize_t or size_t
     * or something. */
    result = (int)(initial_size-buf_datalen(conn->outbuf));
  } else {
    CONN_LOG_PROTECT(conn,
                     result = buf_flush_to_socket(conn->outbuf, conn->s,
                                        max_to_write, &conn->outbuf_flushlen));
    if (result < 0) {
      if (CONN_IS_EDGE(conn))
        connection_edge_end_errno(TO_EDGE_CONN(conn));
      if (conn->type == CONN_TYPE_AP) {
        /* writing failed; we couldn't send a SOCKS reply if we wanted to */
        TO_ENTRY_CONN(conn)->socks_request->has_finished = 1;
      }

      connection_close_immediate(conn); /* Don't flush; connection is dead. */
      connection_mark_for_close(conn);
      return -1;
    }
    update_send_buffer_size(conn->s);
    n_written = (size_t) result;
  }

  if (n_written && conn->type == CONN_TYPE_AP) {
    edge_connection_t *edge_conn = TO_EDGE_CONN(conn);

    /* Check for overflow: */
    if (PREDICT_LIKELY(UINT32_MAX - edge_conn->n_written > n_written))
      edge_conn->n_written += (int)n_written;
    else
      edge_conn->n_written = UINT32_MAX;
  }

  /* If CONN_BW events are enabled, update conn->n_written_conn_bw for
   * OR/DIR/EXIT connections, checking for overflow. */
  if (n_written && get_options()->TestingEnableConnBwEvent &&
     (conn->type == CONN_TYPE_OR ||
      conn->type == CONN_TYPE_DIR ||
      conn->type == CONN_TYPE_EXIT)) {
    if (PREDICT_LIKELY(UINT32_MAX - conn->n_written_conn_bw > n_written))
      conn->n_written_conn_bw += (int)n_written;
    else
      conn->n_written_conn_bw = UINT32_MAX;
  }

  connection_buckets_decrement(conn, approx_time(), n_read, n_written);

  if (result > 0) {
    /* If we wrote any bytes from our buffer, then call the appropriate
     * functions. */
    if (connection_flushed_some(conn) < 0) {
      if (connection_speaks_cells(conn)) {
        connection_or_notify_error(TO_OR_CONN(conn),
                                   END_OR_CONN_REASON_MISC,
                                   "Got error back from "
                                   "connection_flushed_some()");
      }

      /*
       * This can bypass normal channel checking since we did
       * connection_or_notify_error() above.
       */
      connection_mark_for_close_internal(conn);
    }
  }

  if (!connection_wants_to_flush(conn) &&
      !dont_stop_writing) { /* it's done flushing */
    if (connection_finished_flushing(conn) < 0) {
      /* already marked */
      return -1;
    }
    return 0;
  }

  /* Call even if result is 0, since the global write bucket may
   * have reached 0 on a different conn, and this connection needs to
   * know to stop writing. */
  connection_consider_empty_write_buckets(conn);
  if (n_read > 0 && connection_is_reading(conn))
    connection_consider_empty_read_buckets(conn);

  return 0;
}

/* DOCDOC connection_handle_write */
int
connection_handle_write(connection_t *conn, int force)
{
    int res;
    tor_gettimeofday_cache_clear();
    conn->in_connection_handle_write = 1;
    if (conn->use_quic) {
          res = connection_handle_write_impl_quic(conn, force);
        } else {
        	res = connection_handle_write_impl(conn, force);
        }
    conn->in_connection_handle_write = 0;
    return res;
}

/**
 * Try to flush data that's waiting for a write on <b>conn</b>.  Return
 * -1 on failure, 0 on success.
 *
 * Don't use this function for regular writing; the buffers
 * system should be good enough at scheduling writes there.  Instead, this
 * function is for cases when we're about to exit or something and we want
 * to report it right away.
 */
int
connection_flush(connection_t *conn)
{
  return connection_handle_write(conn, 1);
}

/** Helper for connection_write_to_buf_impl and connection_write_buf_to_buf:
 *
 * Return true iff it is okay to queue bytes on <b>conn</b>'s outbuf for
 * writing.
 */
static int
connection_may_write_to_buf(connection_t *conn)
{
  /* if it's marked for close, only allow write if we mean to flush it */
  if (conn->marked_for_close && !conn->hold_open_until_flushed)
    return 0;

  return 1;
}

/** Helper for connection_write_to_buf_impl and connection_write_buf_to_buf:
 *
 * Called when an attempt to add bytes on <b>conn</b>'s outbuf has failed;
 * mark the connection and warn as appropriate.
 */
static void
connection_write_to_buf_failed(connection_t *conn)
{
  if (CONN_IS_EDGE(conn)) {
    /* if it failed, it means we have our package/delivery windows set
       wrong compared to our max outbuf size. close the whole circuit. */
    log_warn(LD_NET,
             "write_to_buf failed. Closing circuit (fd %d).", (int)conn->s);
    circuit_mark_for_close(circuit_get_by_edge_conn(TO_EDGE_CONN(conn)),
                           END_CIRC_REASON_INTERNAL);
  } else if (conn->type == CONN_TYPE_OR) {
    or_connection_t *orconn = TO_OR_CONN(conn);
    log_warn(LD_NET,
             "write_to_buf failed on an orconn; notifying of error "
             "(fd %d)", (int)(conn->s));
    connection_or_close_for_error(orconn, 0);
  } else {
    log_warn(LD_NET,
             "write_to_buf failed. Closing connection (fd %d).",
             (int)conn->s);
    connection_mark_for_close(conn);
  }
}

/** Helper for connection_write_to_buf_impl and connection_write_buf_to_buf:
 *
 * Called when an attempt to add bytes on <b>conn</b>'s outbuf has succeeded:
 * record the number of bytes added.
 */
static void
connection_write_to_buf_commit(connection_t *conn, size_t len)
{
  /* If we receive optimistic data in the EXIT_CONN_STATE_RESOLVING
   * state, we don't want to try to write it right away, since
   * conn->write_event won't be set yet.  Otherwise, write data from
   * this conn as the socket is available. */
  if (conn->write_event) {
    connection_start_writing(conn);
  }
  conn->outbuf_flushlen += len;
}

/** Append <b>len</b> bytes of <b>string</b> onto <b>conn</b>'s
 * outbuf, and ask it to start writing.
 *
 * If <b>zlib</b> is nonzero, this is a directory connection that should get
 * its contents compressed or decompressed as they're written.  If zlib is
 * negative, this is the last data to be compressed, and the connection's zlib
 * state should be flushed.
 */
MOCK_IMPL(void,
connection_write_to_buf_impl_,(const char *string, size_t len,
                               connection_t *conn, int zlib))
{
	// separate out for using QUIC conveniently
	    connection_write_to_buf_impl_generic(string, len, conn, zlib, (quicsock_stream_id_t)0);
}
MOCK_IMPL(void,
connection_write_to_buf_impl_generic, (const char *string, size_t len,
                               connection_t *conn, int zlib,
                               quicsock_stream_id_t stream_id))
{
  if (conn->use_quic) {
    log_debug(LD_OR, "connection_write_to_buf_impl_ invoked on stream %lld",
                        (long long unsigned int)stream_id);
  }
  /* XXXX This function really needs to return -1 on failure. */
  int r;
  if (!len && !(zlib<0))
    return;

  if (!connection_may_write_to_buf(conn))
    return;

  size_t written;

  if (zlib) {
    size_t old_datalen = buf_datalen(conn->outbuf);
    dir_connection_t *dir_conn = TO_DIR_CONN(conn);
    int done = zlib < 0;
    CONN_LOG_PROTECT(conn, r = buf_add_compress(conn->outbuf,
                                                dir_conn->compress_state,
                                                string, len, done));
    written = buf_datalen(conn->outbuf) - old_datalen;
  } else {
      if (conn->use_quic) {
  	    CONN_LOG_PROTECT(conn, r = write_to_buf_quic(string, len, conn->outbuf, stream_id));
      } else {
        CONN_LOG_PROTECT(conn, r = buf_add(conn->outbuf, string, len));
      }

    written = len;
  }
  if (r < 0) {
    connection_write_to_buf_failed(conn);
    return;
  }
  connection_write_to_buf_commit(conn, written);
}

/**
 * Add all bytes from <b>buf</b> to <b>conn</b>'s outbuf, draining them
 * from <b>buf</b>. (If the connection is marked and will soon be closed,
 * nothing is drained.)
 */
void
connection_buf_add_buf(connection_t *conn, buf_t *buf)
{
  tor_assert(conn);
  tor_assert(buf);
  size_t len = buf_datalen(buf);
  if (len == 0)
    return;

  if (!connection_may_write_to_buf(conn))
    return;

  buf_move_all(conn->outbuf, buf);
  connection_write_to_buf_commit(conn, len);
}

#define CONN_GET_ALL_TEMPLATE(var, test) \
  STMT_BEGIN \
    smartlist_t *conns = get_connection_array();   \
    smartlist_t *ret_conns = smartlist_new();     \
    SMARTLIST_FOREACH_BEGIN(conns, connection_t *, var) { \
      if (var && (test) && !var->marked_for_close) \
        smartlist_add(ret_conns, var); \
    } SMARTLIST_FOREACH_END(var);                                            \
    return ret_conns; \
  STMT_END

/* Return a list of connections that aren't close and matches the given type
 * and state. The returned list can be empty and must be freed using
 * smartlist_free(). The caller does NOT have ownership of the objects in the
 * list so it must not free them nor reference them as they can disappear. */
smartlist_t *
connection_list_by_type_state(int type, int state)
{
  CONN_GET_ALL_TEMPLATE(conn, (conn->type == type && conn->state == state));
}

/* Return a list of connections that aren't close and matches the given type
 * and purpose. The returned list can be empty and must be freed using
 * smartlist_free(). The caller does NOT have ownership of the objects in the
 * list so it must not free them nor reference them as they can disappear. */
smartlist_t *
connection_list_by_type_purpose(int type, int purpose)
{
  CONN_GET_ALL_TEMPLATE(conn,
                        (conn->type == type && conn->purpose == purpose));
}

/** Return a connection_t * from get_connection_array() that satisfies test on
 * var, and that is not marked for close. */
#define CONN_GET_TEMPLATE(var, test)               \
  STMT_BEGIN                                       \
    smartlist_t *conns = get_connection_array();   \
    SMARTLIST_FOREACH(conns, connection_t *, var,  \
    {                                              \
      if (var && (test) && !var->marked_for_close) \
        return var;                                \
    });                                            \
    return NULL;                                   \
  STMT_END

/** Return a connection with given type, address, port, and purpose;
 * or NULL if no such connection exists (or if all such connections are marked
 * for close). */
MOCK_IMPL(connection_t *,
connection_get_by_type_addr_port_purpose,(int type,
                                         const tor_addr_t *addr, uint16_t port,
                                         int purpose))
{
  CONN_GET_TEMPLATE(conn,
       (conn->type == type &&
        tor_addr_eq(&conn->addr, addr) &&
        conn->port == port &&
        conn->purpose == purpose));
}

/** Return the stream with id <b>id</b> if it is not already marked for
 * close.
 */
connection_t *
connection_get_by_global_id(uint64_t id)
{
  CONN_GET_TEMPLATE(conn, conn->global_identifier == id);
}

/** Return a connection of type <b>type</b> that is not marked for close.
 */
connection_t *
connection_get_by_type(int type)
{
  CONN_GET_TEMPLATE(conn, conn->type == type);
}

/** Return a connection of type <b>type</b> that is in state <b>state</b>,
 * and that is not marked for close.
 */
connection_t *
connection_get_by_type_state(int type, int state)
{
  CONN_GET_TEMPLATE(conn, conn->type == type && conn->state == state);
}

/** Return a connection of type <b>type</b> that has rendquery equal
 * to <b>rendquery</b>, and that is not marked for close. If state
 * is non-zero, conn must be of that state too.
 */
connection_t *
connection_get_by_type_state_rendquery(int type, int state,
                                       const char *rendquery)
{
  tor_assert(type == CONN_TYPE_DIR ||
             type == CONN_TYPE_AP || type == CONN_TYPE_EXIT);
  tor_assert(rendquery);

  CONN_GET_TEMPLATE(conn,
       (conn->type == type &&
        (!state || state == conn->state)) &&
        (
         (type == CONN_TYPE_DIR &&
          TO_DIR_CONN(conn)->rend_data &&
          !rend_cmp_service_ids(rendquery,
                    rend_data_get_address(TO_DIR_CONN(conn)->rend_data)))
         ||
              (CONN_IS_EDGE(conn) &&
               TO_EDGE_CONN(conn)->rend_data &&
               !rend_cmp_service_ids(rendquery,
                    rend_data_get_address(TO_EDGE_CONN(conn)->rend_data)))
         ));
}

/** Return a new smartlist of dir_connection_t * from get_connection_array()
 * that satisfy conn_test on connection_t *conn_var, and dirconn_test on
 * dir_connection_t *dirconn_var. conn_var must be of CONN_TYPE_DIR and not
 * marked for close to be included in the list. */
#define DIR_CONN_LIST_TEMPLATE(conn_var, conn_test,             \
                               dirconn_var, dirconn_test)       \
  STMT_BEGIN                                                    \
    smartlist_t *conns = get_connection_array();                \
    smartlist_t *dir_conns = smartlist_new();                   \
    SMARTLIST_FOREACH_BEGIN(conns, connection_t *, conn_var) {  \
      if (conn_var && (conn_test)                               \
          && conn_var->type == CONN_TYPE_DIR                    \
          && !conn_var->marked_for_close) {                     \
        dir_connection_t *dirconn_var = TO_DIR_CONN(conn_var);  \
        if (dirconn_var && (dirconn_test)) {                    \
          smartlist_add(dir_conns, dirconn_var);                \
        }                                                       \
      }                                                         \
    } SMARTLIST_FOREACH_END(conn_var);                          \
    return dir_conns;                                           \
  STMT_END

/** Return a list of directory connections that are fetching the item
 * described by <b>purpose</b>/<b>resource</b>. If there are none,
 * return an empty list. This list must be freed using smartlist_free,
 * but the pointers in it must not be freed.
 * Note that this list should not be cached, as the pointers in it can be
 * freed if their connections close. */
smartlist_t *
connection_dir_list_by_purpose_and_resource(
                                            int purpose,
                                            const char *resource)
{
  DIR_CONN_LIST_TEMPLATE(conn,
                         conn->purpose == purpose,
                         dirconn,
                         0 == strcmp_opt(resource,
                                         dirconn->requested_resource));
}

/** Return a list of directory connections that are fetching the item
 * described by <b>purpose</b>/<b>resource</b>/<b>state</b>. If there are
 * none, return an empty list. This list must be freed using smartlist_free,
 * but the pointers in it must not be freed.
 * Note that this list should not be cached, as the pointers in it can be
 * freed if their connections close. */
smartlist_t *
connection_dir_list_by_purpose_resource_and_state(
                                                  int purpose,
                                                  const char *resource,
                                                  int state)
{
  DIR_CONN_LIST_TEMPLATE(conn,
                         conn->purpose == purpose && conn->state == state,
                         dirconn,
                         0 == strcmp_opt(resource,
                                         dirconn->requested_resource));
}

#undef DIR_CONN_LIST_TEMPLATE

/** Return an arbitrary active OR connection that isn't <b>this_conn</b>.
 *
 * We use this to guess if we should tell the controller that we
 * didn't manage to connect to any of our bridges. */
static connection_t *
connection_get_another_active_or_conn(const or_connection_t *this_conn)
{
  CONN_GET_TEMPLATE(conn,
                    conn != TO_CONN(this_conn) && conn->type == CONN_TYPE_OR);
}

/** Return 1 if there are any active OR connections apart from
 * <b>this_conn</b>.
 *
 * We use this to guess if we should tell the controller that we
 * didn't manage to connect to any of our bridges. */
int
any_other_active_or_conns(const or_connection_t *this_conn)
{
  connection_t *conn = connection_get_another_active_or_conn(this_conn);
  if (conn != NULL) {
    log_debug(LD_DIR, "%s: Found an OR connection: %s",
              __func__, conn->address);
    return 1;
  }

  return 0;
}

#undef CONN_GET_TEMPLATE

/** Return 1 if <b>conn</b> is a listener conn, else return 0. */
int
connection_is_listener(connection_t *conn)
{
  if (conn->type == CONN_TYPE_OR_LISTENER ||
      conn->type == CONN_TYPE_EXT_OR_LISTENER ||
      conn->type == CONN_TYPE_AP_LISTENER ||
      conn->type == CONN_TYPE_AP_TRANS_LISTENER ||
      conn->type == CONN_TYPE_AP_DNS_LISTENER ||
      conn->type == CONN_TYPE_AP_NATD_LISTENER ||
      conn->type == CONN_TYPE_AP_HTTP_CONNECT_LISTENER ||
      conn->type == CONN_TYPE_DIR_LISTENER ||
      conn->type == CONN_TYPE_CONTROL_LISTENER)
    return 1;
  return 0;
}

/** Return 1 if <b>conn</b> is in state "open" and is not marked
 * for close, else return 0.
 */
int
connection_state_is_open(connection_t *conn)
{
  tor_assert(conn);

  if (conn->marked_for_close)
    return 0;

  if ((conn->type == CONN_TYPE_OR && conn->state == OR_CONN_STATE_OPEN) ||
      (conn->type == CONN_TYPE_EXT_OR) ||
      (conn->type == CONN_TYPE_AP && conn->state == AP_CONN_STATE_OPEN) ||
      (conn->type == CONN_TYPE_EXIT && conn->state == EXIT_CONN_STATE_OPEN) ||
      (conn->type == CONN_TYPE_CONTROL &&
       conn->state == CONTROL_CONN_STATE_OPEN))
    return 1;

  return 0;
}

/** Return 1 if conn is in 'connecting' state, else return 0. */
int
connection_state_is_connecting(connection_t *conn)
{
  tor_assert(conn);

  if (conn->marked_for_close)
    return 0;
  switch (conn->type)
    {
    case CONN_TYPE_OR:
      return conn->state == OR_CONN_STATE_CONNECTING;
    case CONN_TYPE_EXIT:
      return conn->state == EXIT_CONN_STATE_CONNECTING;
    case CONN_TYPE_DIR:
      return conn->state == DIR_CONN_STATE_CONNECTING;
    }

  return 0;
}

/** Allocates a base64'ed authenticator for use in http or https
 * auth, based on the input string <b>authenticator</b>. Returns it
 * if success, else returns NULL. */
char *
alloc_http_authenticator(const char *authenticator)
{
  /* an authenticator in Basic authentication
   * is just the string "username:password" */
  const size_t authenticator_length = strlen(authenticator);
  const size_t base64_authenticator_length =
      base64_encode_size(authenticator_length, 0) + 1;
  char *base64_authenticator = tor_malloc(base64_authenticator_length);
  if (base64_encode(base64_authenticator, base64_authenticator_length,
                    authenticator, authenticator_length, 0) < 0) {
    tor_free(base64_authenticator); /* free and set to null */
  }
  return base64_authenticator;
}

/** Given a socket handle, check whether the local address (sockname) of the
 * socket is one that we've connected from before.  If so, double-check
 * whether our address has changed and we need to generate keys.  If we do,
 * call init_keys().
 */
static void
client_check_address_changed(tor_socket_t sock)
{
  tor_addr_t out_addr, iface_addr;
  tor_addr_t **last_interface_ip_ptr;
  sa_family_t family;

  if (!outgoing_addrs)
    outgoing_addrs = smartlist_new();

  if (tor_addr_from_getsockname(&out_addr, sock) < 0) {
    int e = tor_socket_errno(sock);
    log_warn(LD_NET, "getsockname() to check for address change failed: %s",
             tor_socket_strerror(e));
    return;
  }
  family = tor_addr_family(&out_addr);

  if (family == AF_INET)
    last_interface_ip_ptr = &last_interface_ipv4;
  else if (family == AF_INET6)
    last_interface_ip_ptr = &last_interface_ipv6;
  else
    return;

  if (! *last_interface_ip_ptr) {
    tor_addr_t *a = tor_malloc_zero(sizeof(tor_addr_t));
    if (get_interface_address6(LOG_INFO, family, a)==0) {
      *last_interface_ip_ptr = a;
    } else {
      tor_free(a);
    }
  }

  /* If we've used this address previously, we're okay. */
  SMARTLIST_FOREACH(outgoing_addrs, const tor_addr_t *, a_ptr,
                    if (tor_addr_eq(a_ptr, &out_addr))
                      return;
                    );

  /* Uh-oh.  We haven't connected from this address before. Has the interface
   * address changed? */
  if (get_interface_address6(LOG_INFO, family, &iface_addr)<0)
    return;

  if (tor_addr_eq(&iface_addr, *last_interface_ip_ptr)) {
    /* Nope, it hasn't changed.  Add this address to the list. */
    smartlist_add(outgoing_addrs, tor_memdup(&out_addr, sizeof(tor_addr_t)));
  } else {
    /* The interface changed.  We're a client, so we need to regenerate our
     * keys.  First, reset the state. */
    log_notice(LD_NET, "Our IP address has changed.  Rotating keys...");
    tor_addr_copy(*last_interface_ip_ptr, &iface_addr);
    SMARTLIST_FOREACH(outgoing_addrs, tor_addr_t*, a_ptr, tor_free(a_ptr));
    smartlist_clear(outgoing_addrs);
    smartlist_add(outgoing_addrs, tor_memdup(&out_addr, sizeof(tor_addr_t)));
    /* We'll need to resolve ourselves again. */
    reset_last_resolved_addr();
    /* Okay, now change our keys. */
    ip_address_changed(1);
  }
}

/** Some systems have limited system buffers for recv and xmit on
 * sockets allocated in a virtual server or similar environment. For a Tor
 * server this can produce the "Error creating network socket: No buffer
 * space available" error once all available TCP buffer space is consumed.
 * This method will attempt to constrain the buffers allocated for the socket
 * to the desired size to stay below system TCP buffer limits.
 */
static void
set_constrained_socket_buffers(tor_socket_t sock, int size)
{
  void *sz = (void*)&size;
  socklen_t sz_sz = (socklen_t) sizeof(size);
  if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, sz, sz_sz) < 0) {
    int e = tor_socket_errno(sock);
    log_warn(LD_NET, "setsockopt() to constrain send "
             "buffer to %d bytes failed: %s", size, tor_socket_strerror(e));
  }
  if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, sz, sz_sz) < 0) {
    int e = tor_socket_errno(sock);
    log_warn(LD_NET, "setsockopt() to constrain recv "
             "buffer to %d bytes failed: %s", size, tor_socket_strerror(e));
  }
}

/** Process new bytes that have arrived on conn-\>inbuf.
 *
 * This function just passes conn to the connection-specific
 * connection_*_process_inbuf() function. It also passes in
 * package_partial if wanted.
 */
static int
connection_process_inbuf(connection_t *conn, int package_partial)
{
  tor_assert(conn);

  switch (conn->type) {
    case CONN_TYPE_OR:
      return connection_or_process_inbuf(TO_OR_CONN(conn));
    case CONN_TYPE_EXT_OR:
      return connection_ext_or_process_inbuf(TO_OR_CONN(conn));
    case CONN_TYPE_EXIT:
    case CONN_TYPE_AP:
      return connection_edge_process_inbuf(TO_EDGE_CONN(conn),
                                           package_partial);
    case CONN_TYPE_DIR:
      return connection_dir_process_inbuf(TO_DIR_CONN(conn));
    case CONN_TYPE_CONTROL:
      return connection_control_process_inbuf(TO_CONTROL_CONN(conn));
    default:
      log_err(LD_BUG,"got unexpected conn type %d.", conn->type);
      tor_fragile_assert();
      return -1;
  }
}

/** Called whenever we've written data on a connection. */
static int
connection_flushed_some(connection_t *conn)
{
  int r = 0;
  tor_assert(!conn->in_flushed_some);
  conn->in_flushed_some = 1;
  if (conn->type == CONN_TYPE_DIR &&
      conn->state == DIR_CONN_STATE_SERVER_WRITING) {
    r = connection_dirserv_flushed_some(TO_DIR_CONN(conn));
  } else if (conn->type == CONN_TYPE_OR) {
    r = connection_or_flushed_some(TO_OR_CONN(conn));
  } else if (CONN_IS_EDGE(conn)) {
    r = connection_edge_flushed_some(TO_EDGE_CONN(conn));
  }
  conn->in_flushed_some = 0;
  return r;
}

/** We just finished flushing bytes to the appropriately low network layer,
 * and there are no more bytes remaining in conn-\>outbuf or
 * conn-\>tls to be flushed.
 *
 * This function just passes conn to the connection-specific
 * connection_*_finished_flushing() function.
 */
static int
connection_finished_flushing(connection_t *conn)
{
  tor_assert(conn);

  /* If the connection is closed, don't try to do anything more here. */
  if (CONN_IS_CLOSED(conn)&& !(QUICSOCK_OK(conn->q_sock)))
    return 0;

//  log_fn(LOG_DEBUG,"entered. Socket %u.", conn->s);

  connection_stop_writing(conn);

  switch (conn->type) {
    case CONN_TYPE_OR:
      return connection_or_finished_flushing(TO_OR_CONN(conn));
    case CONN_TYPE_EXT_OR:
      return connection_ext_or_finished_flushing(TO_OR_CONN(conn));
    case CONN_TYPE_AP:
    case CONN_TYPE_EXIT:
      return connection_edge_finished_flushing(TO_EDGE_CONN(conn));
    case CONN_TYPE_DIR:
      return connection_dir_finished_flushing(TO_DIR_CONN(conn));
    case CONN_TYPE_CONTROL:
      return connection_control_finished_flushing(TO_CONTROL_CONN(conn));
    default:
      log_err(LD_BUG,"got unexpected conn type %d.", conn->type);
      tor_fragile_assert();
      return -1;
  }
}

/** Called when our attempt to connect() to another server has just
 * succeeded.
 *
 * This function just passes conn to the connection-specific
 * connection_*_finished_connecting() function.
 */
static int
connection_finished_connecting(connection_t *conn)
{
  tor_assert(conn);

  if (!server_mode(get_options())) {
    /* See whether getsockname() says our address changed.  We need to do this
     * now that the connection has finished, because getsockname() on Windows
     * won't work until then. */
    client_check_address_changed(conn->s);
  }

  switch (conn->type)
    {
    case CONN_TYPE_OR:
      return connection_or_finished_connecting(TO_OR_CONN(conn));
    case CONN_TYPE_EXIT:
      return connection_edge_finished_connecting(TO_EDGE_CONN(conn));
    case CONN_TYPE_DIR:
      return connection_dir_finished_connecting(TO_DIR_CONN(conn));
    default:
      log_err(LD_BUG,"got unexpected conn type %d.", conn->type);
      tor_fragile_assert();
      return -1;
  }
}

/** Callback: invoked when a connection reaches an EOF event. */
static int
connection_reached_eof(connection_t *conn)
{
  switch (conn->type) {
    case CONN_TYPE_OR:
    case CONN_TYPE_EXT_OR:
      return connection_or_reached_eof(TO_OR_CONN(conn));
    case CONN_TYPE_AP:
    case CONN_TYPE_EXIT:
      return connection_edge_reached_eof(TO_EDGE_CONN(conn));
    case CONN_TYPE_DIR:
      return connection_dir_reached_eof(TO_DIR_CONN(conn));
    case CONN_TYPE_CONTROL:
      return connection_control_reached_eof(TO_CONTROL_CONN(conn));
    default:
      log_err(LD_BUG,"got unexpected conn type %d.", conn->type);
      tor_fragile_assert();
      return -1;
  }
}

/** Comparator for the two-orconn case in OOS victim sort */
static int
oos_victim_comparator_for_orconns(or_connection_t *a, or_connection_t *b)
{
  int a_circs, b_circs;
  /* Fewer circuits == higher priority for OOS kill, sort earlier */

  a_circs = connection_or_get_num_circuits(a);
  b_circs = connection_or_get_num_circuits(b);

  if (a_circs < b_circs) return 1;
  else if (a_circs > b_circs) return -1;
  else return 0;
}

/** Sort comparator for OOS victims; better targets sort before worse
 * ones. */
static int
oos_victim_comparator(const void **a_v, const void **b_v)
{
  connection_t *a = NULL, *b = NULL;

  /* Get connection pointers out */

  a = (connection_t *)(*a_v);
  b = (connection_t *)(*b_v);

  tor_assert(a != NULL);
  tor_assert(b != NULL);

  /*
   * We always prefer orconns as victims currently; we won't even see
   * these non-orconn cases, but if we do, sort them after orconns.
   */
  if (a->type == CONN_TYPE_OR && b->type == CONN_TYPE_OR) {
    return oos_victim_comparator_for_orconns(TO_OR_CONN(a), TO_OR_CONN(b));
  } else {
    /*
     * One isn't an orconn; if one is, it goes first.  We currently have no
     * opinions about cases where neither is an orconn.
     */
    if (a->type == CONN_TYPE_OR) return -1;
    else if (b->type == CONN_TYPE_OR) return 1;
    else return 0;
  }
}

/** Pick n victim connections for the OOS handler and return them in a
 * smartlist.
 */
MOCK_IMPL(STATIC smartlist_t *,
pick_oos_victims, (int n))
{
  smartlist_t *eligible = NULL, *victims = NULL;
  smartlist_t *conns;
  int conn_counts_by_type[CONN_TYPE_MAX_ + 1], i;

  /*
   * Big damn assumption (someone improve this someday!):
   *
   * Socket exhaustion normally happens on high-volume relays, and so
   * most of the connections involved are orconns.  We should pick victims
   * by assembling a list of all orconns, and sorting them in order of
   * how much 'damage' by some metric we'd be doing by dropping them.
   *
   * If we move on from orconns, we should probably think about incoming
   * directory connections next, or exit connections.  Things we should
   * probably never kill are controller connections and listeners.
   *
   * This function will count how many connections of different types
   * exist and log it for purposes of gathering data on typical OOS
   * situations to guide future improvements.
   */

  /* First, get the connection array */
  conns = get_connection_array();
  /*
   * Iterate it and pick out eligible connection types, and log some stats
   * along the way.
   */
  eligible = smartlist_new();
  memset(conn_counts_by_type, 0, sizeof(conn_counts_by_type));
  SMARTLIST_FOREACH_BEGIN(conns, connection_t *, c) {
    /* Bump the counter */
    tor_assert(c->type <= CONN_TYPE_MAX_);
    ++(conn_counts_by_type[c->type]);

    /* Skip anything without a socket we can free */
    if (!(SOCKET_OK(c->s))) {
      continue;
    }

    /* Skip anything we would count as moribund */
    if (connection_is_moribund(c)) {
      continue;
    }

    switch (c->type) {
      case CONN_TYPE_OR:
        /* We've got an orconn, it's eligible to be OOSed */
        smartlist_add(eligible, c);
        break;
      default:
        /* We don't know what to do with it, ignore it */
        break;
    }
  } SMARTLIST_FOREACH_END(c);

  /* Log some stats */
  if (smartlist_len(conns) > 0) {
    /* At least one counter must be non-zero */
    log_info(LD_NET, "Some stats on conn types seen during OOS follow");
    for (i = CONN_TYPE_MIN_; i <= CONN_TYPE_MAX_; ++i) {
      /* Did we see any? */
      if (conn_counts_by_type[i] > 0) {
        log_info(LD_NET, "%s: %d conns",
                 conn_type_to_string(i),
                 conn_counts_by_type[i]);
      }
    }
    log_info(LD_NET, "Done with OOS conn type stats");
  }

  /* Did we find more eligible targets than we want to kill? */
  if (smartlist_len(eligible) > n) {
    /* Sort the list in order of target preference */
    smartlist_sort(eligible, oos_victim_comparator);
    /* Pick first n as victims */
    victims = smartlist_new();
    for (i = 0; i < n; ++i) {
      smartlist_add(victims, smartlist_get(eligible, i));
    }
    /* Free the original list */
    smartlist_free(eligible);
  } else {
    /* No, we can just call them all victims */
    victims = eligible;
  }

  return victims;
}

/** Kill a list of connections for the OOS handler. */
MOCK_IMPL(STATIC void,
kill_conn_list_for_oos, (smartlist_t *conns))
{
  if (!conns) return;

  SMARTLIST_FOREACH_BEGIN(conns, connection_t *, c) {
    /* Make sure the channel layer gets told about orconns */
    if (c->type == CONN_TYPE_OR) {
      connection_or_close_for_error(TO_OR_CONN(c), 1);
    } else {
      connection_mark_for_close(c);
    }
  } SMARTLIST_FOREACH_END(c);

  log_notice(LD_NET,
             "OOS handler marked %d connections",
             smartlist_len(conns));
}

/** Out-of-Sockets handler; n_socks is the current number of open
 * sockets, and failed is non-zero if a socket exhaustion related
 * error immediately preceded this call.  This is where to do
 * circuit-killing heuristics as needed.
 */
void
connection_check_oos(int n_socks, int failed)
{
  int target_n_socks = 0, moribund_socks, socks_to_kill;
  smartlist_t *conns;

  /* Early exit: is OOS checking disabled? */
  if (get_options()->DisableOOSCheck) {
    return;
  }

  /* Sanity-check args */
  tor_assert(n_socks >= 0);

  /*
   * Make some log noise; keep it at debug level since this gets a chance
   * to run on every connection attempt.
   */
  log_debug(LD_NET,
            "Running the OOS handler (%d open sockets, %s)",
            n_socks, (failed != 0) ? "exhaustion seen" : "no exhaustion");

  /*
   * Check if we're really handling an OOS condition, and if so decide how
   * many sockets we want to get down to.  Be sure we check if the threshold
   * is distinct from zero first; it's possible for this to be called a few
   * times before we've finished reading the config.
   */
  if (n_socks >= get_options()->ConnLimit_high_thresh &&
      get_options()->ConnLimit_high_thresh != 0 &&
      get_options()->ConnLimit_ != 0) {
    /* Try to get down to the low threshold */
    target_n_socks = get_options()->ConnLimit_low_thresh;
    log_notice(LD_NET,
               "Current number of sockets %d is greater than configured "
               "limit %d; OOS handler trying to get down to %d",
               n_socks, get_options()->ConnLimit_high_thresh,
               target_n_socks);
  } else if (failed) {
    /*
     * If we're not at the limit but we hit a socket exhaustion error, try to
     * drop some (but not as aggressively as ConnLimit_low_threshold, which is
     * 3/4 of ConnLimit_)
     */
    target_n_socks = (n_socks * 9) / 10;
    log_notice(LD_NET,
               "We saw socket exhaustion at %d open sockets; OOS handler "
               "trying to get down to %d",
               n_socks, target_n_socks);
  }

  if (target_n_socks > 0) {
    /*
     * It's an OOS!
     *
     * Count moribund sockets; it's be important that anything we decide
     * to get rid of here but don't immediately close get counted as moribund
     * on subsequent invocations so we don't try to kill too many things if
     * connection_check_oos() gets called multiple times.
     */
    moribund_socks = connection_count_moribund();

    if (moribund_socks < n_socks - target_n_socks) {
      socks_to_kill = n_socks - target_n_socks - moribund_socks;

      conns = pick_oos_victims(socks_to_kill);
      if (conns) {
        kill_conn_list_for_oos(conns);
        log_notice(LD_NET,
                   "OOS handler killed %d conns", smartlist_len(conns));
        smartlist_free(conns);
      } else {
        log_notice(LD_NET, "OOS handler failed to pick any victim conns");
      }
    } else {
      log_notice(LD_NET,
                 "Not killing any sockets for OOS because there are %d "
                 "already moribund, and we only want to eliminate %d",
                 moribund_socks, n_socks - target_n_socks);
    }
  }
}

/** Log how many bytes are used by buffers of different kinds and sizes. */
void
connection_dump_buffer_mem_stats(int severity)
{
  uint64_t used_by_type[CONN_TYPE_MAX_+1];
  uint64_t alloc_by_type[CONN_TYPE_MAX_+1];
  int n_conns_by_type[CONN_TYPE_MAX_+1];
  uint64_t total_alloc = 0;
  uint64_t total_used = 0;
  int i;
  smartlist_t *conns = get_connection_array();

  memset(used_by_type, 0, sizeof(used_by_type));
  memset(alloc_by_type, 0, sizeof(alloc_by_type));
  memset(n_conns_by_type, 0, sizeof(n_conns_by_type));

  SMARTLIST_FOREACH_BEGIN(conns, connection_t *, c) {
    int tp = c->type;
    ++n_conns_by_type[tp];
    if (c->inbuf) {
      used_by_type[tp] += buf_datalen(c->inbuf);
      alloc_by_type[tp] += buf_allocation(c->inbuf);
    }
    if (c->outbuf) {
      used_by_type[tp] += buf_datalen(c->outbuf);
      alloc_by_type[tp] += buf_allocation(c->outbuf);
    }
  } SMARTLIST_FOREACH_END(c);
  for (i=0; i <= CONN_TYPE_MAX_; ++i) {
    total_used += used_by_type[i];
    total_alloc += alloc_by_type[i];
  }

  tor_log(severity, LD_GENERAL,
     "In buffers for %d connections: "U64_FORMAT" used/"U64_FORMAT" allocated",
      smartlist_len(conns),
      U64_PRINTF_ARG(total_used), U64_PRINTF_ARG(total_alloc));
  for (i=CONN_TYPE_MIN_; i <= CONN_TYPE_MAX_; ++i) {
    if (!n_conns_by_type[i])
      continue;
    tor_log(severity, LD_GENERAL,
        "  For %d %s connections: "U64_FORMAT" used/"U64_FORMAT" allocated",
        n_conns_by_type[i], conn_type_to_string(i),
        U64_PRINTF_ARG(used_by_type[i]), U64_PRINTF_ARG(alloc_by_type[i]));
  }
}

/** Verify that connection <b>conn</b> has all of its invariants
 * correct. Trigger an assert if anything is invalid.
 */
void
assert_connection_ok(connection_t *conn, time_t now)
{
  (void) now; /* XXXX unused. */
  tor_assert(conn);
  tor_assert(conn->type >= CONN_TYPE_MIN_);
  tor_assert(conn->type <= CONN_TYPE_MAX_);

  switch (conn->type) {
    case CONN_TYPE_OR:
    case CONN_TYPE_EXT_OR:
      tor_assert(conn->magic == OR_CONNECTION_MAGIC);
      break;
    case CONN_TYPE_AP:
      tor_assert(conn->magic == ENTRY_CONNECTION_MAGIC);
      break;
    case CONN_TYPE_EXIT:
      tor_assert(conn->magic == EDGE_CONNECTION_MAGIC);
      break;
    case CONN_TYPE_DIR:
      tor_assert(conn->magic == DIR_CONNECTION_MAGIC);
      break;
    case CONN_TYPE_CONTROL:
      tor_assert(conn->magic == CONTROL_CONNECTION_MAGIC);
      break;
    CASE_ANY_LISTENER_TYPE:
      tor_assert(conn->magic == LISTENER_CONNECTION_MAGIC);
      break;
    default:
      tor_assert(conn->magic == BASE_CONNECTION_MAGIC);
      break;
  }

  if (conn->linked_conn) {
    tor_assert(conn->linked_conn->linked_conn == conn);
    tor_assert(conn->linked);
  }
  if (conn->linked)
    tor_assert(!SOCKET_OK(conn->s));

  if (conn->outbuf_flushlen > 0) {
    /* With optimistic data, we may have queued data in
     * EXIT_CONN_STATE_RESOLVING while the conn is not yet marked to writing.
     * */
    tor_assert((conn->type == CONN_TYPE_EXIT &&
                conn->state == EXIT_CONN_STATE_RESOLVING) ||
               connection_is_writing(conn) ||
               conn->write_blocked_on_bw ||
               (CONN_IS_EDGE(conn) &&
                TO_EDGE_CONN(conn)->edge_blocked_on_circ));
  }

  if (conn->hold_open_until_flushed)
    tor_assert(conn->marked_for_close);

  /* XXXX check: read_blocked_on_bw, write_blocked_on_bw, s, conn_array_index,
   * marked_for_close. */

  /* buffers */
  if (conn->inbuf)
    buf_assert_ok(conn->inbuf);
  if (conn->outbuf)
    buf_assert_ok(conn->outbuf);

  if (conn->type == CONN_TYPE_OR) {
    or_connection_t *or_conn = TO_OR_CONN(conn);
    if (conn->state == OR_CONN_STATE_OPEN) {
      /* tor_assert(conn->bandwidth > 0); */
      /* the above isn't necessarily true: if we just did a TLS
       * handshake but we didn't recognize the other peer, or it
       * gave a bad cert/etc, then we won't have assigned bandwidth,
       * yet it will be open. -RD
       */
//      tor_assert(conn->read_bucket >= 0);
    }
//    tor_assert(conn->addr && conn->port);
    tor_assert(conn->address);
    if (!conn->use_quic &&conn->state > OR_CONN_STATE_PROXY_HANDSHAKING)
      tor_assert(or_conn->tls);
  }

  if (CONN_IS_EDGE(conn)) {
    /* XXX unchecked: package window, deliver window. */
    if (conn->type == CONN_TYPE_AP) {
      entry_connection_t *entry_conn = TO_ENTRY_CONN(conn);
      if (entry_conn->chosen_exit_optional || entry_conn->chosen_exit_retries)
        tor_assert(entry_conn->chosen_exit_name);

      tor_assert(entry_conn->socks_request);
      if (conn->state == AP_CONN_STATE_OPEN) {
        tor_assert(entry_conn->socks_request->has_finished);
        if (!conn->marked_for_close) {
          tor_assert(ENTRY_TO_EDGE_CONN(entry_conn)->cpath_layer);
          assert_cpath_layer_ok(ENTRY_TO_EDGE_CONN(entry_conn)->cpath_layer);
        }
      }
    }
    if (conn->type == CONN_TYPE_EXIT) {
      tor_assert(conn->purpose == EXIT_PURPOSE_CONNECT ||
                 conn->purpose == EXIT_PURPOSE_RESOLVE);
    }
  } else if (conn->type == CONN_TYPE_DIR) {
  } else {
    /* Purpose is only used for dir and exit types currently */
    tor_assert(!conn->purpose);
  }

  switch (conn->type)
    {
    CASE_ANY_LISTENER_TYPE:
      tor_assert(conn->state == LISTENER_STATE_READY);
      break;
    case CONN_TYPE_OR:
      tor_assert(conn->state >= OR_CONN_STATE_MIN_);
      tor_assert(conn->state <= OR_CONN_STATE_MAX_);
      break;
    case CONN_TYPE_EXT_OR:
      tor_assert(conn->state >= EXT_OR_CONN_STATE_MIN_);
      tor_assert(conn->state <= EXT_OR_CONN_STATE_MAX_);
      break;
    case CONN_TYPE_EXIT:
      tor_assert(conn->state >= EXIT_CONN_STATE_MIN_);
      tor_assert(conn->state <= EXIT_CONN_STATE_MAX_);
      tor_assert(conn->purpose >= EXIT_PURPOSE_MIN_);
      tor_assert(conn->purpose <= EXIT_PURPOSE_MAX_);
      break;
    case CONN_TYPE_AP:
      tor_assert(conn->state >= AP_CONN_STATE_MIN_);
      tor_assert(conn->state <= AP_CONN_STATE_MAX_);
      tor_assert(TO_ENTRY_CONN(conn)->socks_request);
      break;
    case CONN_TYPE_DIR:
      tor_assert(conn->state >= DIR_CONN_STATE_MIN_);
      tor_assert(conn->state <= DIR_CONN_STATE_MAX_);
      tor_assert(conn->purpose >= DIR_PURPOSE_MIN_);
      tor_assert(conn->purpose <= DIR_PURPOSE_MAX_);
      break;
    case CONN_TYPE_CONTROL:
      tor_assert(conn->state >= CONTROL_CONN_STATE_MIN_);
      tor_assert(conn->state <= CONTROL_CONN_STATE_MAX_);
      break;
    default:
      tor_assert(0);
  }
}

/** Fills <b>addr</b> and <b>port</b> with the details of the global
 *  proxy server we are using.
 *  <b>conn</b> contains the connection we are using the proxy for.
 *
 *  Return 0 on success, -1 on failure.
 */
int
get_proxy_addrport(tor_addr_t *addr, uint16_t *port, int *proxy_type,
                   const connection_t *conn)
{
  const or_options_t *options = get_options();

  /* Client Transport Plugins can use another proxy, but that should be hidden
   * from the rest of tor (as the plugin is responsible for dealing with the
   * proxy), check it first, then check the rest of the proxy types to allow
   * the config to have unused ClientTransportPlugin entries.
   */
  if (options->ClientTransportPlugin) {
    const transport_t *transport = NULL;
    int r;
    r = get_transport_by_bridge_addrport(&conn->addr, conn->port, &transport);
    if (r<0)
      return -1;
    if (transport) { /* transport found */
      tor_addr_copy(addr, &transport->addr);
      *port = transport->port;
      *proxy_type = transport->socks_version;
      return 0;
    }

    /* Unused ClientTransportPlugin. */
  }

  if (options->HTTPSProxy) {
    tor_addr_copy(addr, &options->HTTPSProxyAddr);
    *port = options->HTTPSProxyPort;
    *proxy_type = PROXY_CONNECT;
    return 0;
  } else if (options->Socks4Proxy) {
    tor_addr_copy(addr, &options->Socks4ProxyAddr);
    *port = options->Socks4ProxyPort;
    *proxy_type = PROXY_SOCKS4;
    return 0;
  } else if (options->Socks5Proxy) {
    tor_addr_copy(addr, &options->Socks5ProxyAddr);
    *port = options->Socks5ProxyPort;
    *proxy_type = PROXY_SOCKS5;
    return 0;
  }

  tor_addr_make_unspec(addr);
  *port = 0;
  *proxy_type = PROXY_NONE;
  return 0;
}

/** Log a failed connection to a proxy server.
 *  <b>conn</b> is the connection we use the proxy server for. */
void
log_failed_proxy_connection(connection_t *conn)
{
  tor_addr_t proxy_addr;
  uint16_t proxy_port;
  int proxy_type;

  if (get_proxy_addrport(&proxy_addr, &proxy_port, &proxy_type, conn) != 0)
    return; /* if we have no proxy set up, leave this function. */

  log_warn(LD_NET,
           "The connection to the %s proxy server at %s just failed. "
           "Make sure that the proxy server is up and running.",
           proxy_type_to_string(proxy_type),
           fmt_addrport(&proxy_addr, proxy_port));
}

/** Return string representation of <b>proxy_type</b>. */
static const char *
proxy_type_to_string(int proxy_type)
{
  switch (proxy_type) {
  case PROXY_CONNECT:   return "HTTP";
  case PROXY_SOCKS4:    return "SOCKS4";
  case PROXY_SOCKS5:    return "SOCKS5";
  case PROXY_PLUGGABLE: return "pluggable transports SOCKS";
  case PROXY_NONE:      return "NULL";
  default:              tor_assert(0);
  }
  return NULL; /*Unreached*/
}

/** Call connection_free_minimal() on every connection in our array, and
 * release all storage held by connection.c.
 *
 * Don't do the checks in connection_free(), because they will
 * fail.
 */
void
connection_free_all(void)
{
  smartlist_t *conns = get_connection_array();

  /* We don't want to log any messages to controllers. */
  SMARTLIST_FOREACH(conns, connection_t *, conn,
    if (conn->type == CONN_TYPE_CONTROL)
      TO_CONTROL_CONN(conn)->event_mask = 0);

  control_update_global_event_mask();

  /* Unlink everything from the identity map. */
  connection_or_clear_identity_map();
  connection_or_clear_ext_or_id_map();

  /* Clear out our list of broken connections */
  clear_broken_connection_map(0);

  SMARTLIST_FOREACH(conns, connection_t *, conn,
                    connection_free_minimal(conn));

  if (outgoing_addrs) {
    SMARTLIST_FOREACH(outgoing_addrs, tor_addr_t *, addr, tor_free(addr));
    smartlist_free(outgoing_addrs);
    outgoing_addrs = NULL;
  }

  tor_free(last_interface_ipv4);
  tor_free(last_interface_ipv6);
}

/** Log a warning, and possibly emit a control event, that <b>received</b> came
 * at a skewed time.  <b>trusted</b> indicates that the <b>source</b> was one
 * that we had more faith in and therefore the warning level should have higher
 * severity.
 */
void
clock_skew_warning(const connection_t *conn, long apparent_skew, int trusted,
                   log_domain_mask_t domain, const char *received,
                   const char *source)
{
  char dbuf[64];
  char *ext_source = NULL, *warn = NULL;
  format_time_interval(dbuf, sizeof(dbuf), apparent_skew);
  if (conn)
    tor_asprintf(&ext_source, "%s:%s:%d", source, conn->address, conn->port);
  else
    ext_source = tor_strdup(source);
  log_fn(trusted ? LOG_WARN : LOG_INFO, domain,
         "Received %s with skewed time (%s): "
         "It seems that our clock is %s by %s, or that theirs is %s%s. "
         "Tor requires an accurate clock to work: please check your time, "
         "timezone, and date settings.", received, ext_source,
         apparent_skew > 0 ? "ahead" : "behind", dbuf,
         apparent_skew > 0 ? "behind" : "ahead",
         (!conn || trusted) ? "" : ", or they are sending us the wrong time");
  if (trusted) {
    control_event_general_status(LOG_WARN, "CLOCK_SKEW SKEW=%ld SOURCE=%s",
                                 apparent_skew, ext_source);
    tor_asprintf(&warn, "Clock skew %ld in %s from %s", apparent_skew,
                 received, source);
    control_event_bootstrap_problem(warn, "CLOCK_SKEW", conn, 1);
  }
  tor_free(warn);
  tor_free(ext_source);
}

