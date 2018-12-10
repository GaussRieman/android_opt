# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

message(STATUS "******************************************************************")
message(STATUS "CMAKE_SOURCE_DIR->" ${CMAKE_SOURCE_DIR})
message(STATUS "PROJECT_SOURCE_DIR->)" ${PROJECT_SOURCE_DIR})
message(STATUS "******************************************************************")


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

 include_directories(src/main/include/ src/main/include/eigen-3.3.4/)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

#设置so库路径
set(my_lib_path ${CMAKE_SOURCE_DIR}/libs)

# 添加第三方动态库
add_library(qspower-lib
             SHARED
             IMPORTED)

# 设置第三方动态库属性（存储位置)
set_target_properties(qspower-lib
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/libs/arm64-v8a/libqspower-1.1.0.so)

#find_package(OpenMP REQUIRED)
#if(OPENMP_FOUND)
#message("OPENMP FOUND")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()

set(CMAKE_EXE_LINKER_FLAGS "-Wl --as-needed")



add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} qspower-lib ${OpenMP_LIBS})
