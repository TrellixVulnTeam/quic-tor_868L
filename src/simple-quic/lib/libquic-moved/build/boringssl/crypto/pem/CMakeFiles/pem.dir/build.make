# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build

# Include any dependencies generated for this target.
include boringssl/crypto/pem/CMakeFiles/pem.dir/depend.make

# Include the progress variables for this target.
include boringssl/crypto/pem/CMakeFiles/pem.dir/progress.make

# Include the compile flags for this target's objects.
include boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o: ../boringssl/crypto/pem/pem_all.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_all.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_all.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_all.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_all.c > CMakeFiles/pem.dir/pem_all.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_all.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_all.c -o CMakeFiles/pem.dir/pem_all.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o


boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o: ../boringssl/crypto/pem/pem_info.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_info.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_info.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_info.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_info.c > CMakeFiles/pem.dir/pem_info.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_info.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_info.c -o CMakeFiles/pem.dir/pem_info.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o


boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o: ../boringssl/crypto/pem/pem_lib.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_lib.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_lib.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_lib.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_lib.c > CMakeFiles/pem.dir/pem_lib.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_lib.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_lib.c -o CMakeFiles/pem.dir/pem_lib.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o


boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o: ../boringssl/crypto/pem/pem_oth.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_oth.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_oth.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_oth.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_oth.c > CMakeFiles/pem.dir/pem_oth.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_oth.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_oth.c -o CMakeFiles/pem.dir/pem_oth.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o


boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o: ../boringssl/crypto/pem/pem_pk8.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_pk8.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_pk8.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_pk8.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_pk8.c > CMakeFiles/pem.dir/pem_pk8.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_pk8.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_pk8.c -o CMakeFiles/pem.dir/pem_pk8.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o


boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o: ../boringssl/crypto/pem/pem_pkey.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_pkey.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_pkey.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_pkey.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_pkey.c > CMakeFiles/pem.dir/pem_pkey.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_pkey.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_pkey.c -o CMakeFiles/pem.dir/pem_pkey.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o


boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o: ../boringssl/crypto/pem/pem_x509.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_x509.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_x509.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_x509.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_x509.c > CMakeFiles/pem.dir/pem_x509.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_x509.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_x509.c -o CMakeFiles/pem.dir/pem_x509.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o


boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o: boringssl/crypto/pem/CMakeFiles/pem.dir/flags.make
boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o: ../boringssl/crypto/pem/pem_xaux.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pem.dir/pem_xaux.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_xaux.c

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pem.dir/pem_xaux.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_xaux.c > CMakeFiles/pem.dir/pem_xaux.c.i

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pem.dir/pem_xaux.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem/pem_xaux.c -o CMakeFiles/pem.dir/pem_xaux.c.s

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.requires:

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.requires

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.provides: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.requires
	$(MAKE) -f boringssl/crypto/pem/CMakeFiles/pem.dir/build.make boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.provides.build
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.provides

boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.provides.build: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o


pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o
pem: boringssl/crypto/pem/CMakeFiles/pem.dir/build.make

.PHONY : pem

# Rule to build all files generated by this target.
boringssl/crypto/pem/CMakeFiles/pem.dir/build: pem

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/build

boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_all.c.o.requires
boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_info.c.o.requires
boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_lib.c.o.requires
boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_oth.c.o.requires
boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pk8.c.o.requires
boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_pkey.c.o.requires
boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_x509.c.o.requires
boringssl/crypto/pem/CMakeFiles/pem.dir/requires: boringssl/crypto/pem/CMakeFiles/pem.dir/pem_xaux.c.o.requires

.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/requires

boringssl/crypto/pem/CMakeFiles/pem.dir/clean:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem && $(CMAKE_COMMAND) -P CMakeFiles/pem.dir/cmake_clean.cmake
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/clean

boringssl/crypto/pem/CMakeFiles/pem.dir/depend:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/pem /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/pem/CMakeFiles/pem.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : boringssl/crypto/pem/CMakeFiles/pem.dir/depend

