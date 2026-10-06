cmake_minimum_required(VERSION 3.16)
project(chroma_key LANGUAGES CXX)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)
add_executable(chroma_key main.cpp)
target_compile_features(chroma_key PRIVATE cxx_std_17)
target_include_directories(chroma_key PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(chroma_key PRIVATE ${OpenCV_LIBS})
