NAME := nparser

SRC := main.cpp
OBJ := $(SRC:.cpp=.o)

CXX      := clang++
CXXFLAGS := -g -std=c++17

RM := rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(NAME)

.PHONY: all clean rebuild
rebuild: clean all
