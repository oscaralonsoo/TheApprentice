cmake_minimum_required(VERSION 3.10)
project(PlatformGame VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(PlatformGame ${SOURCES})

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(PlatformGame PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(SDL2_image CONFIG REQUIRED)
target_link_libraries(PlatformGame PRIVATE
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
)

find_package(SDL2_mixer CONFIG REQUIRED)
target_link_libraries(PlatformGame PRIVATE
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED
  libavcodec
  libavformat
  libavutil
  libswscale
  libswresample
)

include_directories(${FFMPEG_INCLUDE_DIRS})
link_directories(${FFMPEG_LIBRARY_DIRS})

add_executable(main main.cpp)
target_link_libraries(main ${FFMPEG_LIBRARIES})
target_include_directories(main PRIVATE ${FFMPEG_INCLUDE_DIRS})
