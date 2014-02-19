CC = g++
CCFLAGS = -std=c++11 -lglut -lGLU
OBJECTS = bitmap.o main.o
TARGET = GLRenderer
GLRenderer: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(CCFLAGS)
main.o : main.cpp bitmap.hpp
	$(CC) -c $<
bitmap.o : bitmap.cpp bitmap.hpp
	$(CC) -c $<
clean : 
	rm $(TARGET) $(OBJECTS)
