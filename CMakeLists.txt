cmake_minimum_required(VERSION 2.6)

project(running-cat)

set(CMAKE_BUILD_PARALLEL_LEVEL 4)

set(CMAKE_CXX_FLAGS "-s -O3")

set(LIBC_LIBRARY -static-libgcc -static-libstdc++)

include_directories("/usr/local/include")
add_definitions(-D_REENTRANT)

set(SDL2_LIBRARY -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lSDL2_image)

add_executable(
    running-cat
    image.c
    main.c
)

target_link_libraries(
    running-cat
    ${LIBC_LIBRARY}
    ${SDL2_LIBRARY}
)
