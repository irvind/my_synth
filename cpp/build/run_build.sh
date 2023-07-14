#!/bin/bash

rm -rf CMake* cmake_* Makefile PulseAudioWavDemo
cmake ../src
cmake --build .
