SOURCE=bsd_allocator.cpp

TARGETS=bsd_allocator

all:
	g++ -std=c++11 $(SOURCE) -o bsd

