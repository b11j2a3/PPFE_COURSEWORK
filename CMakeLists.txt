cmake_minimum_required(VERSION 3.10)
project(PPFE_COURSEWORK C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(waveform_analyser
    main.c
    waveform.c
    io.c
)

if(UNIX)
    target_link_libraries(waveform_analyser m)
endif()
