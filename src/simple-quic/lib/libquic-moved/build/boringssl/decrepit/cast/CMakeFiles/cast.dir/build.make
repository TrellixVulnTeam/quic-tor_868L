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
include boringssl/decrepit/cast/CMakeFiles/cast.dir/depend.make

# Include the progress variables for this target.
include boringssl/decrepit/cast/CMakeFiles/cast.dir/progress.make

# Include the compile flags for this target's objects.
include boringssl/decrepit/cast/CMakeFiles/cast.dir/flags.make

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o: boringssl/decrepit/cast/CMakeFiles/cast.dir/flags.make
boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o: ../boringssl/decrepit/cast/cast.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cast.dir/cast.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/cast/cast.c

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cast.dir/cast.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/cast/cast.c > CMakeFiles/cast.dir/cast.c.i

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cast.dir/cast.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/cast/cast.c -o CMakeFiles/cast.dir/cast.c.s

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.requires:

.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.requires

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.provides: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.requires
	$(MAKE) -f boringssl/decrepit/cast/CMakeFiles/cast.dir/build.make boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.provides.build
.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.provides

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.provides.build: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o


boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o: boringssl/decrepit/cast/CMakeFiles/cast.dir/flags.make
boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o: ../boringssl/decrepit/cast/cast_tables.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cast.dir/cast_tables.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/cast/cast_tables.c

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cast.dir/cast_tables.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/cast/cast_tables.c > CMakeFiles/cast.dir/cast_tables.c.i

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cast.dir/cast_tables.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/cast/cast_tables.c -o CMakeFiles/cast.dir/cast_tables.c.s

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.requires:

.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.requires

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.provides: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.requires
	$(MAKE) -f boringssl/decrepit/cast/CMakeFiles/cast.dir/build.make boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.provides.build
.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.provides

boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.provides.build: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o


cast: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o
cast: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o
cast: boringssl/decrepit/cast/CMakeFiles/cast.dir/build.make

.PHONY : cast

# Rule to build all files generated by this target.
boringssl/decrepit/cast/CMakeFiles/cast.dir/build: cast

.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/build

boringssl/decrepit/cast/CMakeFiles/cast.dir/requires: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast.c.o.requires
boringssl/decrepit/cast/CMakeFiles/cast.dir/requires: boringssl/decrepit/cast/CMakeFiles/cast.dir/cast_tables.c.o.requires

.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/requires

boringssl/decrepit/cast/CMakeFiles/cast.dir/clean:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast && $(CMAKE_COMMAND) -P CMakeFiles/cast.dir/cmake_clean.cmake
.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/clean

boringssl/decrepit/cast/CMakeFiles/cast.dir/depend:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/decrepit/cast /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/decrepit/cast/CMakeFiles/cast.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : boringssl/decrepit/cast/CMakeFiles/cast.dir/depend

