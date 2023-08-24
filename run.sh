#!/bin/bash
gcc main.cpp -lstdc++ `pkg-config --cflags --libs gtk+-3.0`
./a.out
