project(imgui)

file(GLOB SOURCES "*.cpp")

add_library(imgui STATIC ${SOURCES} "backends/imgui_impl_sdl3.cpp" "backends/imgui_impl_sdlrenderer3.cpp")

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/backends)

target_link_libraries(imgui PUBLIC SDL)
