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
include boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/depend.make

# Include the progress variables for this target.
include boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/progress.make

# Include the compile flags for this target's objects.
include boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/flags.make

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o: boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/flags.make
boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o: ../boringssl/crypto/lhash/lhash_test.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/lhash && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/lhash_test.dir/lhash_test.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/lhash/lhash_test.c

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lhash_test.dir/lhash_test.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/lhash && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/lhash/lhash_test.c > CMakeFiles/lhash_test.dir/lhash_test.c.i

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lhash_test.dir/lhash_test.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/lhash && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/lhash/lhash_test.c -o CMakeFiles/lhash_test.dir/lhash_test.c.s

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.requires:

.PHONY : boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.requires

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.provides: boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.requires
	$(MAKE) -f boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/build.make boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.provides.build
.PHONY : boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.provides

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.provides.build: boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o


# Object files for target lhash_test
lhash_test_OBJECTS = \
"CMakeFiles/lhash_test.dir/lhash_test.c.o"

# External object files for target lhash_test
lhash_test_EXTERNAL_OBJECTS = \
"/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/test/CMakeFiles/test_support.dir/file_test.cc.o" \
"/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/test/CMakeFiles/test_support.dir/malloc.cc.o" \
"/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/test/CMakeFiles/test_support.dir/test_util.cc.o"

boringssl/crypto/lhash/lhash_test: boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o
boringssl/crypto/lhash/lhash_test: boringssl/crypto/test/CMakeFiles/test_support.dir/file_test.cc.o
boringssl/crypto/lhash/lhash_test: boringssl/crypto/test/CMakeFiles/test_support.dir/malloc.cc.o
boringssl/crypto/lhash/lhash_test: boringssl/crypto/test/CMakeFiles/test_support.dir/test_util.cc.o
boringssl/crypto/lhash/lhash_test: boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/build.make
boringssl/crypto/lhash/lhash_test: boringssl/crypto/libcrypto.a
boringssl/crypto/lhash/lhash_test: boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable lhash_test"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/lhash && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lhash_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/build: boringssl/crypto/lhash/lhash_test

.PHONY : boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/build

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/requires: boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/lhash_test.c.o.requires

.PHONY : boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/requires

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/clean:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/lhash && $(CMAKE_COMMAND) -P CMakeFiles/lhash_test.dir/cmake_clean.cmake
.PHONY : boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/clean

boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/depend:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/lhash /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/lhash /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : boringssl/crypto/lhash/CMakeFiles/lhash_test.dir/depend

