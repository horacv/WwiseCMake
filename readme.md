
# WwiseCMake

A CMake template project that demonstrates integration of Audiokinetic's Wwise audio engine with any software project.

## Overview

This project showcases a basic integration between Wwise and SDL, featuring:
- A simple SDL window application
- Wwise audio engine integration
- Background music playback by posting an audio event on a soundbank
- A basic audio engine class to interface between Wwise and the rest of the codebase
- Detailed instructions on how to build and link all Wwise dependencies.

## Prerequisites

- CMake 3.x or higher
- Wwise SDK 
- SDL3 and SDL3_TTF development libraries (included)
- C++17 compatible compiler

## Building the Project

1. Clone or download the repository
2. Copy the Wwise SDK and the libraries for your target platform
3. Configue the project by using the command line or an IDE
4. Build using your favorite IDE and compiler

### CMake build commands:

Open the project directory and write one of these command examples based on your platform and required configuration.
Note that some IDEs can configure and build your project without using the command line.

- cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022"
- cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -G Xcode
- cmake -B build/release -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles"

## Project Structure

- `CMakeLists.txt` - Main CMake configuration file
- `assets/` - Contains soundbanks and general assets
  - `fonts/` - arial.ttf
  - `soundbanks/` - Init.bnk, Music.bnk + wem files
- `libs/` - Contains all "third-party" libraries for the project
  - `sdl/`
    - `include` - SDL header files
    - `lib` - SDL dynamic libraries
    - `CMakeLists.txt` - CMake configuration file for SDL
  - `wwise/`
    - `include` - Wwise SDK headers and source files
    - `lib` - Wwise static and dynamic libraries
    - `samples` - Wwise default streaming manager files
    - `CMakeLists.txt` - CMake configuration file for Wwise
  - `CMakeLists.txt` - File that exposes the libraries to the main configuration file
- `src/` - Source code directory

## Official Documentation and Helpful Links

Wwise - Sound Engine Integration Walkthrough\
https://www.audiokinetic.com/en/public-library/2024.1.5_8803/?source=SDK&id=workingwithsdks.html

SDL and SDL_TTF\
https://github.com/libsdl-org/SDL \
https://github.com/libsdl-org/SDL_ttf

CMake\
https://cmake.org/

CMake Projects in Visual Studio\
https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170

Create a CMake hello world project in VS Code\
https://code.visualstudio.com/docs/cpp/cmake-quickstart

Xcode generator in CMake\
https://cmake.org/cmake/help/latest/generator/Xcode.html

CLion - Create/open CMake projects\
https://www.jetbrains.com/help/clion/creating-new-project-from-scratch.html#