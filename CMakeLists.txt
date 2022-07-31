cmake_minimum_required(VERSION 3.16)
project(zmq_pass_through)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

find_package(cppzmq REQUIRED)
find_package(Boost COMPONENTS program_options log REQUIRED)
find_package(Catch2 3 REQUIRED)

add_library(subscriber_lib src/PassThrough.cpp src/PassThrough.h)
add_library(publisher_lib src/GenerateMessages.cpp src/GenerateMessages.h)
add_library(common_lib src/logging_abstraction.cpp src/logging_abstraction.h)

target_link_libraries(subscriber_lib common_lib ${Boost_LIBRARIES})
target_link_libraries(publisher_lib common_lib ${Boost_LIBRARIES})

add_executable(zmq_publisher src/generate_messages_main.cpp)

add_executable(zmq_subscriber src/pass_through_main.cpp src/logging_abstraction.cpp src/logging_abstraction.h)

add_executable(tests test/test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain PUBLIC)

target_link_libraries(zmq_publisher publisher_lib cppzmq)
target_link_libraries(zmq_subscriber subscriber_lib cppzmq)