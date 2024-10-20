cmake_minimum_required(VERSION 3.30)

project(post_event_handler LANGUAGES CXX)

add_executable(post_event_handler
    src/post_event_handler.cpp
    main.cpp)

target_include_directories(post_event_handler PUBLIC
    inc/)

target_compile_options(post_event_handler PUBLIC -g)