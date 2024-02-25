cmake_minimum_required(VERSION 3.26)
project(Adafruit_Display_Driver C)

set(CMAKE_C_STANDARD 11)

add_executable(Adafruit_Display_Driver main.c
)
add_executable(Adafruit_Display_Driver_1 Adafruit_HX8357.c Adafruit_HX8357.h)
