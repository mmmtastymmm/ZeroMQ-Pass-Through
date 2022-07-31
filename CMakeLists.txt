cmake_minimum_required(VERSION 3.16)
project(zmq_pass_through)

set(CMAKE_CXX_STANDARD 20)

find_package(cppzmq REQUIRED)
find_package(Boost COMPONENTS program_options log REQUIRED)

add_executable(zmq_publisher generate_messages_main.cpp)

add_executable(zmq_subscriber pass_through_main.cpp)

target_link_libraries(zmq_publisher cppzmq ${Boost_LIBRARIES})
target_link_libraries(zmq_subscriber cppzmq ${Boost_LIBRARIES})