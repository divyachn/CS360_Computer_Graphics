#!/bin/bash

echo "Compiling the code"

echo "g++ -ggdb -std=c++11 -c -o shader_utils.o shader_utils.cpp"
g++ -ggdb -std=c++11 -c -o shader_utils.o shader_utils.cpp

echo "g++ -ggdb -std=c++11 -c -o texture.o texture.cpp"
g++ -ggdb -std=c++11 -c -o texture.o texture.cpp

echo "g++ -ggdb -std=c++11 main.cpp Camera.cpp shader_utils.o texture.o -lglut -lGLEW -lGL -lGLU -lm -o room"
g++ -ggdb -std=c++11 main.cpp Camera.cpp shader_utils.o texture.o -lglut -lGLEW -lGL -lGLU -lm -o room
