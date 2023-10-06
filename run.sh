#!/bin/bash
gcc main.cpp -lstdc++ -lX11 `pkg-config --cflags --libs gtk+-3.0 opencv4`
./a.out
