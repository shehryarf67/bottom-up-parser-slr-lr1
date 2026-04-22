CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
SRC = $(wildcard src/*.cpp)
TARGET = parser

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
