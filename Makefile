
SRC=$(wildcard src/*.c)
OBJ=$(sort $(SRC:.c=.o))
LIB=libcreed.a
BIN=creed
HEADER=include/creed.h
CC=clang
CFLAGS= \
	-Iinclude \
	-Wall \
	-g \
	-Wno-unused-label \
	-Wno-pointer-to-int-cast \
	-fsanitize=address
LD= -lm -lreadline -lpcre2-32

TEST_SRC=$(wildcard tests/*.c)
TESTS=$(sort $(TEST_SRC:.c=.test))

.PHONY: all clean tests
all: $(BIN)

clean:
	@rm -rf $(OBJ) $(BIN) $(LIB) $(TESTS)

%.o: %.c $(HEADER)
	@echo CC $@
	@$(CC) $(CFLAGS) -o $@ -c $<

%.test: %.c $(LIB)
	@echo CC $@
	@$(CC) $(CFLAGS) $(LD) -o $@ $< $(LIB)

tests: $(TESTS) ;

$(LIB): $(OBJ)
	@echo LD $@
	@ar rcs $@ $(OBJ)

$(BIN): $(LIB) main.c
	@echo LD $@
	@$(CC) $(CFLAGS) $(LD) -o $@ $(OBJ) main.c
