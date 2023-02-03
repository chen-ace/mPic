cmake_minimum_required(VERSION 3.20)
project(mPic)

set(CMAKE_CXX_STANDARD 17)

include (cmake/arch.cmake)
include (cmake/target.cmake)

set(OPENSSL_USE_STATIC_LIBS TRUE)
include(FindOpenSSL)
if(OPENSSL_FOUND)
    message("OPENSSL_FOUND : " ${OPENSSL_ROOT_DIR})
endif()
find_package(OpenSSL REQUIRED)

include(FindCURL)
if(CURL_FOUND)
    message("CURL_FOUND : " ${CURL_LIBRARIES})
endif()
add_definitions(-DCURL_STATICLIB)
find_package(CURL REQUIRED)

find_package(Boost COMPONENTS program_options filesystem)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "BOOST未找到")
endif()

# 阿里云OSS SDK
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations") #关闭警告，否则会在Linux中报错
option(BUILD_SAMPLE "NOT BUILD SAMPLE" OFF) #不编译样例
add_subdirectory (contribs/aliyun-oss-cpp-sdk)
include_directories(contribs/aliyun-oss-cpp-sdk/sdk/include)

include(cmake/simdjson.cmake)

add_executable(mPic main.cpp Config.cpp Config.h)

target_link_libraries(mPic ${OPENSSL_LIBRARIES} cpp-sdk
        ${CURL_LIBRARIES} ${Boost_LIBRARIES} contrib::simdjson)

if(OS_DARWIN)
    message("macos系统")
endif()

if(OS_LINUX)
    message("Linux需要额外链接两个库")
    find_package(Threads)
    find_package(ZLIB)
    target_link_libraries(mPic ${CMAKE_THREAD_LIBS_INIT} ZLIB::ZLIB)
endif()

install(TARGETS mPic
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )