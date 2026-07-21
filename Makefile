CXX      = g++
CXXFLAGS = -O3 -std=c++17 -Wall
TARGET   = engine
SRC      = veloct.cpp
LIB      = libengine.a
HDR      = engine.hpp

all: $(TARGET)

$(TARGET): $(SRC) $(LIB) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) $(LIB) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
