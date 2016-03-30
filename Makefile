BSD_SOURCE=bsd_allocator.cpp

INDIANA_SOURCE=indiana_allocator.cpp

LINUX_SOURCE=linux_buddy_system.cpp

OUTPUT=output.txt

B_TARGET=bsd

I_TARGET=indiana

L_TARGET=linux

all: bsd indiana linux

indiana:
	g++ -std=c++11 -o $(I_TARGET) $(INDIANA_SOURCE)

bsd:
	g++ -std=c++11 -o $(B_TARGET) $(BSD_SOURCE)

linux:
	g++ -std=c++11 -o $(L_TARGET) $(LINUX_SOURCE)

run:
	./$(B_TARGET) &> $(OUTPUT)
	./$(I_TARGET) >> $(OUTPUT)
	./$(L_TARGET) >> $(OUTPUT)
