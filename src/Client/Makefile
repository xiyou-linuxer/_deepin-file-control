CC=g++
CFLAGS = -g -lpthread -w 
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %cpp, %o, $(SRC))
BIN = MainClient
all:$(OBJ)
	@$(CC) $(OBJ) -o $(BIN) $(CFLAGS)
 
%.o:%.cpp
	@$(CC) $(STD) $(CFLAGS) -c $< -o $@
clean:
	@rm  $(BIN) $(OBJ)
