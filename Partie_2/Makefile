CC=gcc
CFLAGS=-std=c99 -Wextra -Wall -Werror -pedantic
LDFLAGS=-lm

ifeq ($(DEBUG),yes)
	CFLAGS += -g
	LDFLAGS +=
else
	CFLAGS += -O3 -DNDEBUG
	LDFLAGS +=
endif

EXEC=dh_gen_group
SRC= ../utilitaire/utiL.c ./Code_C/dh_prime.c $(wildcard *.c)
OBJ= $(SRC:.c=.o)

all:
ifeq ($(DEBUG),yes)
	@echo "Generating in debug mode"
else
	@echo "Generating in release mode"
endif
	@$(MAKE) $(EXEC)

$(EXEC): $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)

asan: CFLAGS += -fsanitize=address -g
asan: LDFLAGS += -fsanitize=address
asan: CC=clang
asan: $(EXEC)

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)

clean:
	@rm -rf $(OBJ) $(EXEC)

.PHONY: clean mrproper

mrproper: clean
	@rm -rf $(EXEC)


dh_gen_group.o : dh_gen_group.h ./Code_C/dh_prime.h ../utilitaire/utiL.h
./Code_C/dh_prime.o : ./Code_C/dh_prime.h ../utilitaire/utiL.h
