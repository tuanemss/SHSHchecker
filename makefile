# Output executable name
TARGET = SHSHchecker

# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -O2

# Libraries to link
LDLIBS = -lcurl

# Source files
SRC = main.cpp

# Default rule to build the target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDLIBS)

# Clean up build files
clean:
	rm -f $(TARGET)
