# Makefile for LRU Cache Test

# Compiler
CXX = g++

# Compiler flags
# -std=c++11: Use C++11 standard
# -Wall: Enable all warnings
# -g: Generate debugging information
CXXFLAGS = -std=c++14 -Wall -g

# Target executable
TARGET = test_lru

# Source files
# Since LRU.h is a template-only library, we only need to compile the test file.
SOURCES = test_lru.cpp

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Rule to run the tests
run: all
	./$(TARGET)

# Clean up the build files
clean:
	rm -f $(TARGET)
