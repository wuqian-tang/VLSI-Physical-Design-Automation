#!/bin/bash
g++ -std=c++17 main.cpp -Ofast -lpthread
./a.out ../testcases/p2-$1.nets ../testcases/p2-$1.cells ../output/p2-$1.out
cd ../verifier
./verify ../testcases/p2-$1.nets ../testcases/p2-$1.cells ../output/p2-$1.out