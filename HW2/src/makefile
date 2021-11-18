TARGET=../bin/hw2
TARGET_PARA=../bin/hw2_parallel
main:
	g++ -o $(TARGET) -Ofast -std=c++17 main.cpp
	g++ -o $(TARGET_PARA) -Ofast -std=c++17 -lpthread main_parallel.cpp
clean :
	rm $(TARGET) $(TARGET_PARA)