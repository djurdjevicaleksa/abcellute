LIBS = -lm
CC = gcc
CFLAGS =  
INC_F = -I./include/ 
SRC_F = src/

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:.c=.o)

BIN = abcellute


all: $(BIN)
	make clean

$(BIN) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN) $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_F) 

clean:
	rm $(OBJ)