CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: strip_comments

strip_comments: strip_comments.cpp
	$(CXX) $(CXXFLAGS) -o strip_comments strip_comments.cpp

clean:
	rm -f strip_comments