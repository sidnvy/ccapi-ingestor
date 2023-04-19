# cmake_minimum_required(VERSION 3.15)
#
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
# # Add the submodules
# add_subdirectory(ccapi/example)
# add_subdirectory(clickhouse-cpp)
# include_directories(ccapi/include)
#
# add_executable(${PROJECT_NAME} main.cpp)
#
# target_include_directories(${PROJECT_NAME} PRIVATE clickhouse-cpp/ clickhouse-cpp/contrib/absl)
#
# # Link against the libraries
# target_link_libraries(${PROJECT_NAME} ccapi clickhouse-cpp-lib)


cmake_minimum_required(VERSION 3.14)
project(ccapi-ingestor)

set(CMAKE_CXX_STANDARD 14)
if(NOT APPLE AND NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
if (WIN32)
  set(CMAKE_BUILD_TYPE "Release")
endif()
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# Add ClickHouse client library
add_subdirectory(clickhouse-cpp)

# Setup CCAPI
set(CCAPI_PROJECT_DIR "${CMAKE_SOURCE_DIR}/ccapi")
message(STATUS "CCAPI_PROJECT_DIR: ${CCAPI_PROJECT_DIR}")
set(RAPIDJSON_INCLUDE_DIR ${CCAPI_PROJECT_DIR}/dependency/rapidjson/include)
set(WEBSOCKETPP_INCLUDE_DIR ${CCAPI_PROJECT_DIR}/dependency/websocketpp)
set(BOOST_INCLUDE_DIR ${CCAPI_PROJECT_DIR}/dependency/boost)
find_package(OpenSSL REQUIRED)
include_directories(${CCAPI_PROJECT_DIR}/include ${WEBSOCKETPP_INCLUDE_DIR} ${BOOST_INCLUDE_DIR} ${RAPIDJSON_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
set(ADDITIONAL_LINK_LIBRARIES "")
if (WIN32)
  set(ADDITIONAL_LINK_LIBRARIES ws2_32)
endif()
link_libraries(OpenSSL::Crypto OpenSSL::SSL ${ADDITIONAL_LINK_LIBRARIES})

# Add your executable and link libraries
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} clickhouse-cpp-lib)

