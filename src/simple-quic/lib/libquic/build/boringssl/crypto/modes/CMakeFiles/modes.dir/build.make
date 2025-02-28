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
include boringssl/crypto/modes/CMakeFiles/modes.dir/depend.make

# Include the progress variables for this target.
include boringssl/crypto/modes/CMakeFiles/modes.dir/progress.make

# Include the compile flags for this target's objects.
include boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make

boringssl/crypto/modes/aesni-gcm-x86_64.S: ../boringssl/crypto/modes/asm/aesni-gcm-x86_64.pl
boringssl/crypto/modes/aesni-gcm-x86_64.S: ../boringssl/crypto/perlasm/arm-xlate.pl
boringssl/crypto/modes/aesni-gcm-x86_64.S: ../boringssl/crypto/perlasm/x86_64-xlate.pl
boringssl/crypto/modes/aesni-gcm-x86_64.S: ../boringssl/crypto/perlasm/x86asm.pl
boringssl/crypto/modes/aesni-gcm-x86_64.S: ../boringssl/crypto/perlasm/x86gas.pl
boringssl/crypto/modes/aesni-gcm-x86_64.S: ../boringssl/crypto/perlasm/x86masm.pl
boringssl/crypto/modes/aesni-gcm-x86_64.S: ../boringssl/crypto/perlasm/x86nasm.pl
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating aesni-gcm-x86_64.S"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/perl /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/asm/aesni-gcm-x86_64.pl elf > aesni-gcm-x86_64.S

boringssl/crypto/modes/ghash-x86_64.S: ../boringssl/crypto/modes/asm/ghash-x86_64.pl
boringssl/crypto/modes/ghash-x86_64.S: ../boringssl/crypto/perlasm/arm-xlate.pl
boringssl/crypto/modes/ghash-x86_64.S: ../boringssl/crypto/perlasm/x86_64-xlate.pl
boringssl/crypto/modes/ghash-x86_64.S: ../boringssl/crypto/perlasm/x86asm.pl
boringssl/crypto/modes/ghash-x86_64.S: ../boringssl/crypto/perlasm/x86gas.pl
boringssl/crypto/modes/ghash-x86_64.S: ../boringssl/crypto/perlasm/x86masm.pl
boringssl/crypto/modes/ghash-x86_64.S: ../boringssl/crypto/perlasm/x86nasm.pl
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating ghash-x86_64.S"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/perl /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/asm/ghash-x86_64.pl elf > ghash-x86_64.S

boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o: boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make
boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o: ../boringssl/crypto/modes/cbc.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/modes.dir/cbc.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/cbc.c

boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/modes.dir/cbc.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/cbc.c > CMakeFiles/modes.dir/cbc.c.i

boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/modes.dir/cbc.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/cbc.c -o CMakeFiles/modes.dir/cbc.c.s

boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.requires:

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.requires

boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.provides: boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.requires
	$(MAKE) -f boringssl/crypto/modes/CMakeFiles/modes.dir/build.make boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.provides.build
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.provides

boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.provides.build: boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o


boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o: boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make
boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o: ../boringssl/crypto/modes/ctr.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/modes.dir/ctr.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/ctr.c

boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/modes.dir/ctr.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/ctr.c > CMakeFiles/modes.dir/ctr.c.i

boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/modes.dir/ctr.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/ctr.c -o CMakeFiles/modes.dir/ctr.c.s

boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.requires:

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.requires

boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.provides: boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.requires
	$(MAKE) -f boringssl/crypto/modes/CMakeFiles/modes.dir/build.make boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.provides.build
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.provides

boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.provides.build: boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o


boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o: boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make
boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o: ../boringssl/crypto/modes/ofb.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/modes.dir/ofb.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/ofb.c

boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/modes.dir/ofb.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/ofb.c > CMakeFiles/modes.dir/ofb.c.i

boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/modes.dir/ofb.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/ofb.c -o CMakeFiles/modes.dir/ofb.c.s

boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.requires:

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.requires

boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.provides: boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.requires
	$(MAKE) -f boringssl/crypto/modes/CMakeFiles/modes.dir/build.make boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.provides.build
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.provides

boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.provides.build: boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o


boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o: boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make
boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o: ../boringssl/crypto/modes/cfb.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/modes.dir/cfb.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/cfb.c

boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/modes.dir/cfb.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/cfb.c > CMakeFiles/modes.dir/cfb.c.i

boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/modes.dir/cfb.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/cfb.c -o CMakeFiles/modes.dir/cfb.c.s

boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.requires:

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.requires

boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.provides: boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.requires
	$(MAKE) -f boringssl/crypto/modes/CMakeFiles/modes.dir/build.make boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.provides.build
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.provides

boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.provides.build: boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o


boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o: boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make
boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o: ../boringssl/crypto/modes/gcm.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/modes.dir/gcm.c.o   -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/gcm.c

boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/modes.dir/gcm.c.i"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/gcm.c > CMakeFiles/modes.dir/gcm.c.i

boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/modes.dir/gcm.c.s"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes/gcm.c -o CMakeFiles/modes.dir/gcm.c.s

boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.requires:

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.requires

boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.provides: boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.requires
	$(MAKE) -f boringssl/crypto/modes/CMakeFiles/modes.dir/build.make boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.provides.build
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.provides

boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.provides.build: boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o


boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o: boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make
boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o: boringssl/crypto/modes/aesni-gcm-x86_64.S
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building ASM object boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(ASM_DEFINES) $(ASM_INCLUDES) $(ASM_FLAGS) -o CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes/aesni-gcm-x86_64.S

boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.requires:

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.requires

boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.provides: boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.requires
	$(MAKE) -f boringssl/crypto/modes/CMakeFiles/modes.dir/build.make boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.provides.build
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.provides

boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.provides.build: boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o


boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o: boringssl/crypto/modes/CMakeFiles/modes.dir/flags.make
boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o: boringssl/crypto/modes/ghash-x86_64.S
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building ASM object boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o"
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && /usr/bin/cc  $(ASM_DEFINES) $(ASM_INCLUDES) $(ASM_FLAGS) -o CMakeFiles/modes.dir/ghash-x86_64.S.o -c /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes/ghash-x86_64.S

boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.requires:

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.requires

boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.provides: boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.requires
	$(MAKE) -f boringssl/crypto/modes/CMakeFiles/modes.dir/build.make boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.provides.build
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.provides

boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.provides.build: boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o


modes: boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o
modes: boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o
modes: boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o
modes: boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o
modes: boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o
modes: boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o
modes: boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o
modes: boringssl/crypto/modes/CMakeFiles/modes.dir/build.make

.PHONY : modes

# Rule to build all files generated by this target.
boringssl/crypto/modes/CMakeFiles/modes.dir/build: modes

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/build

boringssl/crypto/modes/CMakeFiles/modes.dir/requires: boringssl/crypto/modes/CMakeFiles/modes.dir/cbc.c.o.requires
boringssl/crypto/modes/CMakeFiles/modes.dir/requires: boringssl/crypto/modes/CMakeFiles/modes.dir/ctr.c.o.requires
boringssl/crypto/modes/CMakeFiles/modes.dir/requires: boringssl/crypto/modes/CMakeFiles/modes.dir/ofb.c.o.requires
boringssl/crypto/modes/CMakeFiles/modes.dir/requires: boringssl/crypto/modes/CMakeFiles/modes.dir/cfb.c.o.requires
boringssl/crypto/modes/CMakeFiles/modes.dir/requires: boringssl/crypto/modes/CMakeFiles/modes.dir/gcm.c.o.requires
boringssl/crypto/modes/CMakeFiles/modes.dir/requires: boringssl/crypto/modes/CMakeFiles/modes.dir/aesni-gcm-x86_64.S.o.requires
boringssl/crypto/modes/CMakeFiles/modes.dir/requires: boringssl/crypto/modes/CMakeFiles/modes.dir/ghash-x86_64.S.o.requires

.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/requires

boringssl/crypto/modes/CMakeFiles/modes.dir/clean:
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes && $(CMAKE_COMMAND) -P CMakeFiles/modes.dir/cmake_clean.cmake
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/clean

boringssl/crypto/modes/CMakeFiles/modes.dir/depend: boringssl/crypto/modes/aesni-gcm-x86_64.S
boringssl/crypto/modes/CMakeFiles/modes.dir/depend: boringssl/crypto/modes/ghash-x86_64.S
	cd /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/boringssl/crypto/modes /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes /home/malsabah/Desktop/QuicTor/quictor/src/simple-quic/lib/libquic/build/boringssl/crypto/modes/CMakeFiles/modes.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : boringssl/crypto/modes/CMakeFiles/modes.dir/depend

