SRC = party.c party_main.c
OBJ = $(SRC:.c=.o)
BIN = party

CC = gcc
CFLAGS = -Wall -Wextra -Werror -fsanitize=address

all: $(BIN)

run: $(BIN)
	./$(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c party.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
