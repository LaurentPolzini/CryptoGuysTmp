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

EXEC=break_code
SRC= ../utilitaire/utiL.c ../Partie_1/encrypt_decrypt_xor.c $(wildcard *.c)
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

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)

# Special target for AddressSanitizer build
asan: CFLAGS += -fsanitize=address -g
asan: LDFLAGS += -fsanitize=address
asan: CC=clang
asan: $(EXEC)

clean:
	@rm -rf *.o ../utilitaire/utiL.o ../Partie_1/encrypt_decrypt_xor.o $(EXEC)

.PHONY: clean mrproper

mrproper: clean
	@rm -rf $(EXEC)

Queue.o : Queue.h
NTree.o : NTree.h Queue.h
break_code_c1.o : NTree.h Queue.h crackage.h break_code_c1.h
break_code_c2.o : crackage.h
break_code_c3.o : crackage.h
crack_mask.o : ../Partie_1/xor.h ../Partie_1/mask.h
../utilitaire/utiL.o : ../utilitaire/utiL.h
../Partie_1/encrypt_decrypt_xor.o : ../Partie_1/xor.h ../Partie_1/mask.h
break_code_main.o : break_code_c1.h ../utilitaire/utiL.h ../Partie_1/xor.h