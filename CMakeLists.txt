cmake_minimum_required(VERSION 3.30)
project(WwiseCmake VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
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
        src/main.cpp
        src/audio/audio_engine.h
        src/audio/audio_engine.cpp
        src/core/game.h
        src/core/game.cpp
)

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

# ASSETS AND CONTENT

# Define the assets folder path
set(ASSETS_FOLDER_NAME "assets")
set(ASSETS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}")

# Copy the "assets" folder content to the build
add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_SOURCE_DIR}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_FOLDER_NAME}
)