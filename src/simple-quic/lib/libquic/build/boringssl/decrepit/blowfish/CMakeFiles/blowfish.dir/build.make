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
include boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/depend.make

# Include the progress variables for this target.
include boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/progress.make

# Include the compile flags for this target's objects.
include boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/flags.make

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o: boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/flags.make
boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o: ../boringssl/decrepit/blowfish/blowfish.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/blowfish && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/blowfish.dir/blowfish.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/blowfish/blowfish.c

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/blowfish.dir/blowfish.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/blowfish && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/blowfish/blowfish.c > CMakeFiles/blowfish.dir/blowfish.c.i

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/blowfish.dir/blowfish.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/blowfish && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/blowfish/blowfish.c -o CMakeFiles/blowfish.dir/blowfish.c.s

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.requires:

.PHONY : boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.requires

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.provides: boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.requires
	$(MAKE) -f boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/build.make boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.provides.build
.PHONY : boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.provides

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.provides.build: boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o


blowfish: boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o
blowfish: boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/build.make

.PHONY : blowfish

# Rule to build all files generated by this target.
boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/build: blowfish

.PHONY : boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/build

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/requires: boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/blowfish.c.o.requires

.PHONY : boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/requires

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/clean:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/blowfish && $(CMAKE_COMMAND) -P CMakeFiles/blowfish.dir/cmake_clean.cmake
.PHONY : boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/clean

boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/depend:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/blowfish /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/blowfish /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : boringssl/decrepit/blowfish/CMakeFiles/blowfish.dir/depend

