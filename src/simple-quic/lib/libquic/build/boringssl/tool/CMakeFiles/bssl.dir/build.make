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
include boringssl/tool/CMakeFiles/bssl.dir/depend.make

# Include the progress variables for this target.
include boringssl/tool/CMakeFiles/bssl.dir/progress.make

# Include the compile flags for this target's objects.
include boringssl/tool/CMakeFiles/bssl.dir/flags.make

boringssl/tool/CMakeFiles/bssl.dir/args.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/args.cc.o: ../boringssl/tool/args.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/args.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/args.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/args.cc

boringssl/tool/CMakeFiles/bssl.dir/args.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/args.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/args.cc > CMakeFiles/bssl.dir/args.cc.i

boringssl/tool/CMakeFiles/bssl.dir/args.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/args.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/args.cc -o CMakeFiles/bssl.dir/args.cc.s

boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/args.cc.o


boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o: ../boringssl/tool/ciphers.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/ciphers.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/ciphers.cc

boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/ciphers.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/ciphers.cc > CMakeFiles/bssl.dir/ciphers.cc.i

boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/ciphers.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/ciphers.cc -o CMakeFiles/bssl.dir/ciphers.cc.s

boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o


boringssl/tool/CMakeFiles/bssl.dir/client.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/client.cc.o: ../boringssl/tool/client.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/client.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/client.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/client.cc

boringssl/tool/CMakeFiles/bssl.dir/client.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/client.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/client.cc > CMakeFiles/bssl.dir/client.cc.i

boringssl/tool/CMakeFiles/bssl.dir/client.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/client.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/client.cc -o CMakeFiles/bssl.dir/client.cc.s

boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/client.cc.o


boringssl/tool/CMakeFiles/bssl.dir/const.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/const.cc.o: ../boringssl/tool/const.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/const.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/const.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/const.cc

boringssl/tool/CMakeFiles/bssl.dir/const.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/const.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/const.cc > CMakeFiles/bssl.dir/const.cc.i

boringssl/tool/CMakeFiles/bssl.dir/const.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/const.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/const.cc -o CMakeFiles/bssl.dir/const.cc.s

boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/const.cc.o


boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o: ../boringssl/tool/digest.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/digest.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/digest.cc

boringssl/tool/CMakeFiles/bssl.dir/digest.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/digest.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/digest.cc > CMakeFiles/bssl.dir/digest.cc.i

boringssl/tool/CMakeFiles/bssl.dir/digest.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/digest.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/digest.cc -o CMakeFiles/bssl.dir/digest.cc.s

boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o


boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o: ../boringssl/tool/genrsa.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/genrsa.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/genrsa.cc

boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/genrsa.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/genrsa.cc > CMakeFiles/bssl.dir/genrsa.cc.i

boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/genrsa.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/genrsa.cc -o CMakeFiles/bssl.dir/genrsa.cc.s

boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o


boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o: ../boringssl/tool/pkcs12.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/pkcs12.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/pkcs12.cc

boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/pkcs12.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/pkcs12.cc > CMakeFiles/bssl.dir/pkcs12.cc.i

boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/pkcs12.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/pkcs12.cc -o CMakeFiles/bssl.dir/pkcs12.cc.s

boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o


boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o: ../boringssl/tool/rand.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/rand.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/rand.cc

boringssl/tool/CMakeFiles/bssl.dir/rand.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/rand.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/rand.cc > CMakeFiles/bssl.dir/rand.cc.i

boringssl/tool/CMakeFiles/bssl.dir/rand.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/rand.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/rand.cc -o CMakeFiles/bssl.dir/rand.cc.s

boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o


boringssl/tool/CMakeFiles/bssl.dir/server.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/server.cc.o: ../boringssl/tool/server.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/server.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/server.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/server.cc

boringssl/tool/CMakeFiles/bssl.dir/server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/server.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/server.cc > CMakeFiles/bssl.dir/server.cc.i

boringssl/tool/CMakeFiles/bssl.dir/server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/server.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/server.cc -o CMakeFiles/bssl.dir/server.cc.s

boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/server.cc.o


boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o: ../boringssl/tool/speed.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/speed.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/speed.cc

boringssl/tool/CMakeFiles/bssl.dir/speed.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/speed.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/speed.cc > CMakeFiles/bssl.dir/speed.cc.i

boringssl/tool/CMakeFiles/bssl.dir/speed.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/speed.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/speed.cc -o CMakeFiles/bssl.dir/speed.cc.s

boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o


boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o: ../boringssl/tool/tool.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/tool.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/tool.cc

boringssl/tool/CMakeFiles/bssl.dir/tool.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/tool.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/tool.cc > CMakeFiles/bssl.dir/tool.cc.i

boringssl/tool/CMakeFiles/bssl.dir/tool.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/tool.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/tool.cc -o CMakeFiles/bssl.dir/tool.cc.s

boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o


boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o: boringssl/tool/CMakeFiles/bssl.dir/flags.make
boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o: ../boringssl/tool/transport_common.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bssl.dir/transport_common.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/transport_common.cc

boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bssl.dir/transport_common.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/transport_common.cc > CMakeFiles/bssl.dir/transport_common.cc.i

boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bssl.dir/transport_common.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool/transport_common.cc -o CMakeFiles/bssl.dir/transport_common.cc.s

boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.requires:

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.requires

boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.provides: boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.requires
	$(MAKE) -f boringssl/tool/CMakeFiles/bssl.dir/build.make boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.provides.build
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.provides

boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.provides.build: boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o


# Object files for target bssl
bssl_OBJECTS = \
"CMakeFiles/bssl.dir/args.cc.o" \
"CMakeFiles/bssl.dir/ciphers.cc.o" \
"CMakeFiles/bssl.dir/client.cc.o" \
"CMakeFiles/bssl.dir/const.cc.o" \
"CMakeFiles/bssl.dir/digest.cc.o" \
"CMakeFiles/bssl.dir/genrsa.cc.o" \
"CMakeFiles/bssl.dir/pkcs12.cc.o" \
"CMakeFiles/bssl.dir/rand.cc.o" \
"CMakeFiles/bssl.dir/server.cc.o" \
"CMakeFiles/bssl.dir/speed.cc.o" \
"CMakeFiles/bssl.dir/tool.cc.o" \
"CMakeFiles/bssl.dir/transport_common.cc.o"

# External object files for target bssl
bssl_EXTERNAL_OBJECTS =

boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/args.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/client.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/const.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/server.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/build.make
boringssl/tool/bssl: boringssl/ssl/libssl.a
boringssl/tool/bssl: boringssl/crypto/libcrypto.a
boringssl/tool/bssl: boringssl/tool/CMakeFiles/bssl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Linking CXX executable bssl"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bssl.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
boringssl/tool/CMakeFiles/bssl.dir/build: boringssl/tool/bssl

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/build

boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/args.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/ciphers.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/client.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/const.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/digest.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/genrsa.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/pkcs12.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/rand.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/server.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/speed.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/tool.cc.o.requires
boringssl/tool/CMakeFiles/bssl.dir/requires: boringssl/tool/CMakeFiles/bssl.dir/transport_common.cc.o.requires

.PHONY : boringssl/tool/CMakeFiles/bssl.dir/requires

boringssl/tool/CMakeFiles/bssl.dir/clean:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool && $(CMAKE_COMMAND) -P CMakeFiles/bssl.dir/cmake_clean.cmake
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/clean

boringssl/tool/CMakeFiles/bssl.dir/depend:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/tool /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/tool/CMakeFiles/bssl.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : boringssl/tool/CMakeFiles/bssl.dir/depend

