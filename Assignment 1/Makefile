CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = assignment2

SRCS = main_pa2.cpp comment_stripper.cpp lexer.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)