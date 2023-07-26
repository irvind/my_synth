cmake_minimum_required(VERSION 3.10)
find_package(PkgConfig REQUIRED)

project(PulseAudioWavDemo)

pkg_check_modules(GTKMM40 REQUIRED gtkmm-4.0)

add_executable(PulseAudioWavDemo main.cpp yswavfile.cpp PulsePlayer.cpp MainWindow.cpp)

target_include_directories(PulseAudioWavDemo PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(PulseAudioWavDemo PUBLIC ${GTKMM40_INCLUDE_DIRS})

target_link_libraries(PulseAudioWavDemo pulse)
target_link_libraries(PulseAudioWavDemo ${GTKMM40_LIBRARIES})

target_compile_options(PulseAudioWavDemo PUBLIC ${GTKMM40_CFLAGS_OTHER})
