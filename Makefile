CXX := clang++
CXXFLAGS := -Wall -Wextra -std=c++23 -ggdb
CC := clang
CCFLAGS := -Wall -Wextra -ggdb
LDFLAGS := -fsanitize=address,undefined

SRC := src/main.cpp src/parser.cpp src/traverser.cpp src/ir.cpp src/utf.cpp src/make_c.cpp src/builder.cpp
OBJ := $(SRC:.cpp=.o)

CORE_SRC := core/core.c
CORE_OBJ := $(CORE_SRC:.c=.o)

all: core charta

charta: $(OBJ)
	$(CXX) $(LDFLAGS) -o charta $^

core: $(CORE_OBJ)
	ar rcs libcore.a $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) charta $(CORE_OBJ) libcore.a
