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
include boringssl/crypto/dh/CMakeFiles/dh_test.dir/depend.make

# Include the progress variables for this target.
include boringssl/crypto/dh/CMakeFiles/dh_test.dir/progress.make

# Include the compile flags for this target's objects.
include boringssl/crypto/dh/CMakeFiles/dh_test.dir/flags.make

boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o: boringssl/crypto/dh/CMakeFiles/dh_test.dir/flags.make
boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o: ../boringssl/crypto/dh/dh_test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/dh && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dh_test.dir/dh_test.cc.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/dh/dh_test.cc

boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dh_test.dir/dh_test.cc.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/dh && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/dh/dh_test.cc > CMakeFiles/dh_test.dir/dh_test.cc.i

boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dh_test.dir/dh_test.cc.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/dh && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/dh/dh_test.cc -o CMakeFiles/dh_test.dir/dh_test.cc.s

boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.requires:

.PHONY : boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.requires

boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.provides: boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.requires
	$(MAKE) -f boringssl/crypto/dh/CMakeFiles/dh_test.dir/build.make boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.provides.build
.PHONY : boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.provides

boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.provides.build: boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o


# Object files for target dh_test
dh_test_OBJECTS = \
"CMakeFiles/dh_test.dir/dh_test.cc.o"

# External object files for target dh_test
dh_test_EXTERNAL_OBJECTS = \
"/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/test/CMakeFiles/test_support.dir/file_test.cc.o" \
"/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/test/CMakeFiles/test_support.dir/malloc.cc.o" \
"/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/test/CMakeFiles/test_support.dir/test_util.cc.o"

boringssl/crypto/dh/dh_test: boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o
boringssl/crypto/dh/dh_test: boringssl/crypto/test/CMakeFiles/test_support.dir/file_test.cc.o
boringssl/crypto/dh/dh_test: boringssl/crypto/test/CMakeFiles/test_support.dir/malloc.cc.o
boringssl/crypto/dh/dh_test: boringssl/crypto/test/CMakeFiles/test_support.dir/test_util.cc.o
boringssl/crypto/dh/dh_test: boringssl/crypto/dh/CMakeFiles/dh_test.dir/build.make
boringssl/crypto/dh/dh_test: boringssl/crypto/libcrypto.a
boringssl/crypto/dh/dh_test: boringssl/crypto/dh/CMakeFiles/dh_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable dh_test"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/dh && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dh_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
boringssl/crypto/dh/CMakeFiles/dh_test.dir/build: boringssl/crypto/dh/dh_test

.PHONY : boringssl/crypto/dh/CMakeFiles/dh_test.dir/build

boringssl/crypto/dh/CMakeFiles/dh_test.dir/requires: boringssl/crypto/dh/CMakeFiles/dh_test.dir/dh_test.cc.o.requires

.PHONY : boringssl/crypto/dh/CMakeFiles/dh_test.dir/requires

boringssl/crypto/dh/CMakeFiles/dh_test.dir/clean:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/dh && $(CMAKE_COMMAND) -P CMakeFiles/dh_test.dir/cmake_clean.cmake
.PHONY : boringssl/crypto/dh/CMakeFiles/dh_test.dir/clean

boringssl/crypto/dh/CMakeFiles/dh_test.dir/depend:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/dh /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/dh /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/dh/CMakeFiles/dh_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : boringssl/crypto/dh/CMakeFiles/dh_test.dir/depend

