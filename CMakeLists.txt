cmake_minimum_required(VERSION 3.10)
project(ndipush VERSION 0.1.0)

add_executable(ndipush main.cpp 
        src/ndiReceiverWrapper.cpp
        src/AbletonPushManager.cpp)

# Set C++ standard for the target
target_compile_features(ndipush PRIVATE cxx_std_17)


set(ProjectName "${PROJECT_NAME}")
message(STATUS "ProjectName: ${ProjectName}")

set(ProjectPath "${CMAKE_SOURCE_DIR}")
message(STATUS "ProjectPath: ${ProjectPath}")
include_directories(${ProjectPath}/include)
link_directories(${ProjectPath}/src)

message(STATUS "ProjectPath: ${ProjectPath}")

# NDI Related linking 
set (NdiFrameworkPath "/Library/NDI SDK for Apple/lib/macOS")
set (NdiLibFile "libndi.dylib")
message(STATUS "NDI Framework: ${NdiFrameworkPath}")
message(STATUS "NDI Framework includes : ${NdiFrameworkPath}/include")
message(STATUS "NDI Framework lib : ${NdiFrameworkPath}/lib")

include_directories(${NdiFrameworkPath}/include)
link_directories(${NdiFrameworkPath}/lib)
target_link_libraries(${PROJECT_NAME} ${NdiFrameworkPath}/${NdiLibFile})
target_link_libraries(ndipush ${NdiFrameworkPath}/${NdiLibFile})


target_link_libraries( ${PROJECT_NAME} -lpthread)


find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
include_directories(${LIBUSB_INCLUDE_DIRS})
target_link_libraries(ndipush /opt/homebrew/Cellar/libusb/1.0.29/lib/libusb-1.0.0.dylib)

message(STATUS "libusb includes: ${LIBUSB_INCLUDE_DIRS}")
message(STATUS "libusb libraries: ${LIBUSB_LIBRARIES}")


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
