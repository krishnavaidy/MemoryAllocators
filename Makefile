BSD_SOURCE=bsd_allocator.cpp
       
INDIANA_SOURCE=indiana_allocator.cpp

OUTPUT=output.txt

B_TARGET=bsd

I_TARGET=indiana

all: bsd indiana

indiana: 
	g++ -std=c++11 -o $(I_TARGET) $(INDIANA_SOURCE) 

bsd: 
	g++ -std=c++11 -o $(B_TARGET) $(BSD_SOURCE) 

run:
	./$(B_TARGET) &> $(OUTPUT)
	./$(I_TARGET) >> $(OUTPUT)
