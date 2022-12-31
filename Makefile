
SRC=$(wildcard src/*.c)
OBJ=$(sort $(SRC:.c=.o))
HEADER=include/creed.h
LIB=libcreed.a
BIN=creed
CC=clang
CFLAGS= \
	-Iinclude \
	-Wall \
	-g \
	-Wno-unused-label \
	-Wno-pointer-to-int-cast \
	-fsanitize=address
LD= -lm

.PHONY: all clean
all: $(BIN)

clean:
	@rm -rf $(OBJ) $(BIN) $(LIB)

%.o: %.c $(HEADER)
	@echo CC $@
	@$(CC) $(CFLAGS) -o $@ -c $<

$(LIB): $(OBJ)
	@echo LD $@
	@ar rcs $@ $(OBJ)

$(BIN): $(LIB)
	@echo LD $@
	@$(CC) $(CFLAGS) $(LD) -o $@ $(OBJ) main.c
