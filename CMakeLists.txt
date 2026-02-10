cmake_minimum_required(VERSION 3.30)
project(WwiseCmake VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available configurations: Debug, Release" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

add_executable(WwiseCmake
        src/audio/audio_config.h
        src/audio/audio_engine.cpp
        src/audio/audio_engine.h
        src/core/game.cpp
        src/core/game.h
        src/gui/gui.cpp
        src/gui/gui.h
        src/gui/gui_events.h
        src/gui/gui_widget.h
        src/gui/widgets/main_menu.cpp
        src/gui/widgets/main_menu.h
        src/gui/widgets/settings_menu_volume.cpp

        src/main.cpp
        src/pch.h
)

# Pre-Compiled headers section
if(CMAKE_GENERATOR STREQUAL "Xcode")
    set(CMAKE_XCODE_GENERATE_SCHEME ON)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER YES
            XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h"
    )
else()
    target_precompile_headers(${PROJECT_NAME} PRIVATE "src/pch.h")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

function(CopyLibraryToTarget LIB_NAME TARGET_NAME)
    add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${LIB_NAME}>
            $<TARGET_FILE_DIR:${TARGET_NAME}>
    )
endfunction()

# Search path for additional libraries
add_subdirectory(libs)

# LIBRARIES

# imgui
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

# sdl
target_link_libraries(${PROJECT_NAME} PUBLIC SDL)
CopyLibraryToTarget(SDL ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC SDL_TTF)
CopyLibraryToTarget(SDL_TTF ${PROJECT_NAME})

# Wwise
target_link_libraries(${PROJECT_NAME} PUBLIC wwise)

# Add more libraries here:
#.............................


# ............................

# CONTENT AND CONFIG

function(CopyFolderToTarget DIR_NAME SOURCE_DIR)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND
            ${CMAKE_COMMAND} -E copy_directory
            ${SOURCE_DIR}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DIR_NAME}
    )
endfunction()

# Define the assets folder path and copy it to the build directory
set(ASSETS_FOLDER_NAME "assets")
set(ASSETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}")
CopyFolderToTarget(${ASSETS_FOLDER_NAME} ${ASSETS_SOURCE_DIR})

# Define the config folder path and copy it to the build directory
set(CONFIG_FOLDER_NAME "config")
set(CONFIG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_FOLDER_NAME}")
CopyFolderToTarget(${CONFIG_FOLDER_NAME} ${CONFIG_SOURCE_DIR})