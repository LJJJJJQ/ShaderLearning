# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.26.4/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.26.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build

# Include any dependencies generated for this target.
include CMakeFiles/VulkanDemo.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/VulkanDemo.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/VulkanDemo.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/VulkanDemo.dir/flags.make

CMakeFiles/VulkanDemo.dir/main.cpp.o: CMakeFiles/VulkanDemo.dir/flags.make
CMakeFiles/VulkanDemo.dir/main.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/main.cpp
CMakeFiles/VulkanDemo.dir/main.cpp.o: CMakeFiles/VulkanDemo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/VulkanDemo.dir/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/VulkanDemo.dir/main.cpp.o -MF CMakeFiles/VulkanDemo.dir/main.cpp.o.d -o CMakeFiles/VulkanDemo.dir/main.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/main.cpp

CMakeFiles/VulkanDemo.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanDemo.dir/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/main.cpp > CMakeFiles/VulkanDemo.dir/main.cpp.i

CMakeFiles/VulkanDemo.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanDemo.dir/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/main.cpp -o CMakeFiles/VulkanDemo.dir/main.cpp.s

CMakeFiles/VulkanDemo.dir/src/Application.cpp.o: CMakeFiles/VulkanDemo.dir/flags.make
CMakeFiles/VulkanDemo.dir/src/Application.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Application.cpp
CMakeFiles/VulkanDemo.dir/src/Application.cpp.o: CMakeFiles/VulkanDemo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/VulkanDemo.dir/src/Application.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/VulkanDemo.dir/src/Application.cpp.o -MF CMakeFiles/VulkanDemo.dir/src/Application.cpp.o.d -o CMakeFiles/VulkanDemo.dir/src/Application.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Application.cpp

CMakeFiles/VulkanDemo.dir/src/Application.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanDemo.dir/src/Application.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Application.cpp > CMakeFiles/VulkanDemo.dir/src/Application.cpp.i

CMakeFiles/VulkanDemo.dir/src/Application.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanDemo.dir/src/Application.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Application.cpp -o CMakeFiles/VulkanDemo.dir/src/Application.cpp.s

CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o: CMakeFiles/VulkanDemo.dir/flags.make
CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Camera.cpp
CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o: CMakeFiles/VulkanDemo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o -MF CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o.d -o CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Camera.cpp

CMakeFiles/VulkanDemo.dir/src/Camera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanDemo.dir/src/Camera.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Camera.cpp > CMakeFiles/VulkanDemo.dir/src/Camera.cpp.i

CMakeFiles/VulkanDemo.dir/src/Camera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanDemo.dir/src/Camera.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Camera.cpp -o CMakeFiles/VulkanDemo.dir/src/Camera.cpp.s

CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o: CMakeFiles/VulkanDemo.dir/flags.make
CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Loader.cpp
CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o: CMakeFiles/VulkanDemo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o -MF CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o.d -o CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Loader.cpp

CMakeFiles/VulkanDemo.dir/src/Loader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanDemo.dir/src/Loader.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Loader.cpp > CMakeFiles/VulkanDemo.dir/src/Loader.cpp.i

CMakeFiles/VulkanDemo.dir/src/Loader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanDemo.dir/src/Loader.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/Loader.cpp -o CMakeFiles/VulkanDemo.dir/src/Loader.cpp.s

CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o: CMakeFiles/VulkanDemo.dir/flags.make
CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/ModelTransMat.cpp
CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o: CMakeFiles/VulkanDemo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o -MF CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o.d -o CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/ModelTransMat.cpp

CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/ModelTransMat.cpp > CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.i

CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/src/ModelTransMat.cpp -o CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.s

# Object files for target VulkanDemo
VulkanDemo_OBJECTS = \
"CMakeFiles/VulkanDemo.dir/main.cpp.o" \
"CMakeFiles/VulkanDemo.dir/src/Application.cpp.o" \
"CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o" \
"CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o" \
"CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o"

# External object files for target VulkanDemo
VulkanDemo_EXTERNAL_OBJECTS =

VulkanDemo: CMakeFiles/VulkanDemo.dir/main.cpp.o
VulkanDemo: CMakeFiles/VulkanDemo.dir/src/Application.cpp.o
VulkanDemo: CMakeFiles/VulkanDemo.dir/src/Camera.cpp.o
VulkanDemo: CMakeFiles/VulkanDemo.dir/src/Loader.cpp.o
VulkanDemo: CMakeFiles/VulkanDemo.dir/src/ModelTransMat.cpp.o
VulkanDemo: CMakeFiles/VulkanDemo.dir/build.make
VulkanDemo: /usr/local/lib/libvulkan.dylib
VulkanDemo: CMakeFiles/VulkanDemo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable VulkanDemo"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/VulkanDemo.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/VulkanDemo.dir/build: VulkanDemo
.PHONY : CMakeFiles/VulkanDemo.dir/build

CMakeFiles/VulkanDemo.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/VulkanDemo.dir/cmake_clean.cmake
.PHONY : CMakeFiles/VulkanDemo.dir/clean

CMakeFiles/VulkanDemo.dir/depend:
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build /Users/uncled/Documents/CFiles/VulkanEngine/RendingEngine/build/CMakeFiles/VulkanDemo.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/VulkanDemo.dir/depend

