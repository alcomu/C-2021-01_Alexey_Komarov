cmake_minimum_required(VERSION 3.00)
project(pingpong C)

set(CMAKE_C_STANDARD 99)

include_directories(./include)

add_executable(pingpong src/main.c src/ball.c src/draw.c 
    src/frames.c src/init.c src/input.c src/sound.c
    src/stage.c src/util.c src/collision.c) 
 
target_link_libraries(pingpong SDL2_mixer SDL2_ttf SDL2 m)
