CC = g++
CCFLAGS = -std=c++0x -lglut -lGLU
OBJECTS = bitmap.o main.o
TARGET = GLRenderer
GLRenderer: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(CCFLAGS)
main.o : main.cpp bitmap.hpp
	$(CC) -c $< $(CCFLAGS)
bitmap.o : bitmap.cpp bitmap.hpp
	$(CC) -c $< $(CCFLAGS)
clean : 
	rm $(TARGET) $(OBJECTS)
