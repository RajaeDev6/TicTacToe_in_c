cmake_minimum_required(VERSION 3.28.2)
project(Tictoc
    LANGUAGES C
    VERSION 0.1
    DESCRIPTION "tictac toe game"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Tictoc PRIVATE
        -Wextra    
        -std=c23
    )
endif()


set(CMAKE_PREFIX_PATH "/usr/lib/" CACHE PATH "/usr/local/lib/")


set(SDL2_TTF_LIBRARIES "/usr/lib/libSDL2_ttf.so")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED PATHS "/usr/lib/")



message(STATUS "SDL2_ttf libraries: ${SDL2_TTF_LIBRARIES}")
message(STATUS "SDL2_ttf include dirs: ${SDL2_TTF_INCLUDE_DIRS}")

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})


add_executable(Tictoc ./src/main.c)

target_link_libraries(Tictoc ${SDL2_LIBRARIES})
target_link_libraries(Tictoc ${SDL2_TTF_LIBRARIES})

