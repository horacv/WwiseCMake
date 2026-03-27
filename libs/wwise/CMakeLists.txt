# Refer to https://www.audiokinetic.com/en/public-library/2024.1.8_8898/?source=SDK&id=goingfurther.html

###
## 1 - Define and add the Wwise Library
###

add_library(wwise INTERFACE)


###
## 2 - Set the paths to find all the Wwise dynamic and static libraries
###

# Define the path of the static and dynamic libraries for each platform and build configuration
# Add more target platforms here with an elseif ()
if (WIN32 AND MSVC)
    set(PLATFORM_CONFIG "x64_vc170")
    set(WWISE_DEBUG_DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Debug/bin")
    set(WWISE_DEBUG_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Debug/lib")
    set(WWISE_RELEASE_DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Release/bin")
    set(WWISE_RELEASE_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Release/lib")
    target_compile_definitions(wwise INTERFACE "AUDIO_PLATFORM=\"Windows\"")

    # Required for the Wwise IAKStreamManager
    # Refer to https://www.audiokinetic.com/en/library/edge/?source=SDK&id=projectsettings.html
    # Same as "Treat wchar_t as Built-in Type" or "/Zc:wchar_t" compile flag"
    target_compile_definitions(wwise INTERFACE -DUNICODE -D_UNICODE)

elseif (APPLE)
    set(PLATFORM_CONFIG "Mac_Xcode1600")
    set(WWISE_DEBUG_DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Debug/bin")
    set(WWISE_DEBUG_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Debug/lib")
    set(WWISE_RELEASE_DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Release/bin")
    set(WWISE_RELEASE_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_CONFIG}/Release/lib")
    target_compile_definitions(wwise INTERFACE "AUDIO_PLATFORM=\"Mac\"")
endif ()


###
## 3 - Find all the Wwise Core Libraries + Codecs + its dynamic library plugins
###

set(WWISE_CORE_CODEC_COMPONENTS # If not included only PCM will play
        "AkVorbisDecoder"
        "AkOpusDecoder")
set(WWISE_CORE_COMPONENTS # Required for all configurations
        "AkSoundEngine"
        "AkMemoryMgr"
        "AkStreamMgr"
        "AkMusicEngine"
        "AkSpatialAudio"
        ${WWISE_CORE_CODEC_COMPONENTS}
)
set(WWISE_CORE_DEBUG_COMPONENTS # Only for the debug config: Core + Codecs + The profiler component
        ${WWISE_CORE_COMPONENTS}
        "CommunicationCentral")
set(WWISE_CORE_LIBS)

# Find all the core Wwise static libraries (core components)
# Find all the dynamic library plugins (AkGain, AkRoomVerb, AkDelay, AkMasteringSuite..., or your custom plugins)
if (WIN32 AND MSVC)
    # For Debug configuration
    foreach(COMPONENT ${WWISE_CORE_DEBUG_COMPONENTS})
        list(APPEND WWISE_DEBUG_LIBS "${WWISE_DEBUG_LIB_PATH}/${COMPONENT}.lib")
    endforeach()
    file(GLOB DEBUG_PLUGIN_DLLS "${WWISE_DEBUG_DLL_PATH}/*.dll")

    # For Release configuration
    foreach(COMPONENT ${WWISE_CORE_COMPONENTS})
        list(APPEND WWISE_RELEASE_LIBS "${WWISE_RELEASE_LIB_PATH}/${COMPONENT}.lib")
    endforeach()
    file(GLOB RELEASE_PLUGIN_DLLS "${WWISE_RELEASE_DLL_PATH}/*.dll")

elseif (APPLE)
    # For Debug configuration
    foreach(COMPONENT ${WWISE_CORE_DEBUG_COMPONENTS})
        list(APPEND WWISE_DEBUG_LIBS "${WWISE_DEBUG_LIB_PATH}/lib${COMPONENT}.a")
    endforeach()
    file(GLOB DEBUG_PLUGIN_DLLS "${WWISE_DEBUG_DLL_PATH}/*.dylib")

    # For Release configuration
    foreach(COMPONENT ${WWISE_CORE_COMPONENTS})
        list(APPEND WWISE_RELEASE_LIBS "${WWISE_RELEASE_LIB_PATH}/lib${COMPONENT}.a")
    endforeach()
    file(GLOB RELEASE_PLUGIN_DLLS "${WWISE_RELEASE_DLL_PATH}/*.dylib")
endif ()

# Use generator expressions to select correct libraries based on configuration (Build Time)
set(WWISE_CORE_LIBS "$<IF:$<CONFIG:Debug>,${WWISE_DEBUG_LIBS},${WWISE_RELEASE_LIBS}>")


###
## 4 - Copy all the Wwise audio plugins to the build folder
###

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PLUGIN_DLLS ${DEBUG_PLUGIN_DLLS})
else()
    set(PLUGIN_DLLS ${RELEASE_PLUGIN_DLLS})
endif()

if (DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(WWISE_PLUGINS_DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins/wwise")
else ()
    set(WWISE_PLUGINS_DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/plugins/wwise")
endif ()

# Install all the dynamic library plugins in the build folder
file(INSTALL ${PLUGIN_DLLS} DESTINATION ${WWISE_PLUGINS_DESTINATION}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")

###
## 5 - Build and link the Default Wwise Streaming Manager
###

# Required! unless you create your own manager implementation
# Copy the "samples/SoundEngine" folder from the Wwise SDK
# https://www.audiokinetic.com/en/public-library/2024.1.8_8898/?source=SDK&id=streamingdevicemanager.html
set(WWISE_STRM_MNG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/samples/SoundEngine")

if (WIN32 AND MSVC)
    set(WWISE_STRM_MNG_PLATFORM_PATH "${WWISE_STRM_MNG_PATH}/Win32")
elseif (APPLE)
    set(WWISE_STRM_MNG_PLATFORM_PATH "${WWISE_STRM_MNG_PATH}/POSIX")
endif ()

file(GLOB WWISE_STRM_MNG_PLATFORM_SOURCES
        "${WWISE_STRM_MNG_PLATFORM_PATH}*.h"
        "${WWISE_STRM_MNG_PLATFORM_PATH}/*.cpp")

set(WWISE_STRM_MNG_INCLUDE_PATH "${WWISE_STRM_MNG_PATH}/Common")

file(GLOB WWISE_STRM_MNG_SOURCES
        "${WWISE_STRM_MNG_INCLUDE_PATH}/*.h"
        "${WWISE_STRM_MNG_INCLUDE_PATH}/*.cpp")

# Remove the stub file to avoid duplicate symbol definitions
list(REMOVE_ITEM WWISE_STRM_MNG_SOURCES
        "${WWISE_STRM_MNG_INCLUDE_PATH}/AkPlatformProfilerHookStubs.cpp")

add_library(wwise_streaming_manager STATIC
        ${WWISE_STRM_MNG_SOURCES}
        ${WWISE_STRM_MNG_PLATFORM_SOURCES})

target_include_directories(wwise_streaming_manager PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(wwise_streaming_manager PUBLIC wwise ${WWISE_CORE_LIBS})


###
## 6 - Prepare Wwise for linking with the main application
###

# The AK_OPTIMIZED definition is required to ignore unnecessary code on release builds
target_compile_definitions(wwise INTERFACE $<$<CONFIG:Release>:AK_OPTIMIZED>)

set_property(TARGET wwise PROPERTY INTERFACE_LINK_LIBRARIES ${WWISE_CORE_LIBS})
set_property(TARGET wwise APPEND PROPERTY INTERFACE_LINK_LIBRARIES wwise_streaming_manager)

target_include_directories(wwise INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include" # All the source files in /include (AK namespace)
        "${WWISE_STRM_MNG_INCLUDE_PATH}" # All the source files for the default streaming manager
        "${WWISE_STRM_MNG_PLATFORM_PATH}") # All the platform specific source files for the default streaming manager


###
## 7 - Link additional required platform SDKs and frameworks to Wwise:
###

# Refer to the " External Library Dependencies" section here:
# https://www.audiokinetic.com/en/public-library/2024.1.5_8803/?source=SDK&id=goingfurther_builds.html
if (WIN32 AND MSVC)
    set(PLATFORM_EXT_LIBS
            "dinput8"
            "winmm"
            "dsound"
            "dxguid"
            "xinput"
    )
    set(PLATFORM_EXT_LIBS_DEBUG
            ${PLATFORM_EXT_LIBS}
            "ws2_32"  # Winsock 2 - Required by CommunicationCentral in Debug builds
    )
    set(PLATFORM_EXT_LIBS_RELEASE ${PLATFORM_EXT_LIBS})
elseif (APPLE)
    set(PLATFORM_EXT_LIBS
            "-framework Foundation"
            "-framework AudioToolbox"
            "-framework AudioUnit"
            "-framework CoreAudio"
            "-framework AVFoundation"
    )
    set(PLATFORM_EXT_LIBS_DEBUG ${PLATFORM_EXT_LIBS})
    set(PLATFORM_EXT_LIBS_RELEASE ${PLATFORM_EXT_LIBS})
endif ()

target_link_libraries(wwise INTERFACE ${WWISE_CORE_LIBS} "$<IF:$<CONFIG:Debug>,${PLATFORM_EXT_LIBS_DEBUG},${PLATFORM_EXT_LIBS_RELEASE}>")
