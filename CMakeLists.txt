cmake_minimum_required(VERSION 3.16)

project(OrderBookArchiver VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS 
    Core
    WebSockets)
    
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(OrderBookArchiver
    main.cpp
    OrderBookManager.cpp
)

target_link_libraries(OrderBookArchiver PRIVATE 
    Qt6::Core
    Qt6::WebSockets
)