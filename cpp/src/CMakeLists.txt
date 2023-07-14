cmake_minimum_required(VERSION 3.10)

project(PulseAudioWavDemo)

add_executable(PulseAudioWavDemo main.cpp yswavfile.cpp)

target_include_directories(PulseAudioWavDemo PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(PulseAudioWavDemo pulse)
