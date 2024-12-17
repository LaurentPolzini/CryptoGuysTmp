CC=gcc
CFLAGS=-std=c99 -Wextra -Wall -Werror -pedantic
LDFLAGS=-lm -pthread

ifeq ($(DEBUG),yes)
	CFLAGS += -g
	LDFLAGS +=
else
	CFLAGS += -O3 -DNDEBUG
	LDFLAGS +=
endif

EXEC=crypto
SRC_ALL= $(wildcard ./Partie_1/*.c) $(wildcard ./Partie_3/*.c)
SRC= ./utilitaire/utiL.c crypto.c $(filter-out ./Partie_1/sym_crypt.c ./Partie_3/break_code_main.c ./Partie_3/crack_mask_main.c, $(SRC_ALL))
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
# i don't have valgrind on mac...
# and the one that i had doesn't work anymore..
asan: CFLAGS += -fsanitize=address -g
asan: LDFLAGS += -fsanitize=address
asan: CC=clang
asan: $(EXEC)

clean:
	@rm -rf $(OBJ) $(EXEC)

.PHONY: clean mrproper

mrproper: clean
	@rm -rf $(EXEC)


# part 1 links
./Partie_1/encrypt_decrypt_xor.o: ./Partie_1/encrypt_decrypt_xor.c ./Partie_1/chiffrement.h ./utilitaire/utiL.h

# part 2 links
./Partie_2/dh_gen_group.o : ./Partie_2/dh_gen_group.h ./Partie_2/Code_C/dh_prime.h ./utilitaire/utiL.h
./Partie_2/Code_C/dh_prime.o : ./Partie_2/Code_C/dh_prime.h

# part 3 links
./Partie_3/mutex.o : ./Partie_3/mutex.h
./Partie_3/Pile.o : ./Partie_3/Pile.h ./utilitaire/utiL.h
./Partie_3/break_code_c1.o : ./Partie_3/Pile.h ./Partie_3/crackage.h ./Partie_3/break_code_c1.h ./utilitaire/utiL.h ./Partie_1/chiffrement.h ./Partie_3/ThreadSegmentationTableauxIndex.h ./Partie_3/caracteresCandidatsIndexKey.h ./Partie_3/tests_crackage.h ./Partie_3/mutex.h
./Partie_3/break_code_c2.o : ./Partie_3/crackage.h ./utilitaire/utiL.h ./Partie_3/break_code_c1.h ./Partie_3/break_code_c2.h ./Partie_3/break_code_c2_c3.h ./Partie_3/mutex.h
./Partie_3/break_code_c3.o : ./Partie_3/crackage.h ./Partie_3/break_code_c3.h ./utilitaire/utiL.h ./Partie_3/break_code_c2_c3.h ./Partie_3/mutex.h
./Partie_3/break_code_c2_c3.o : ./Partie_3/break_code_c2_c3.h ./Partie_3/break_code_c1.h ./Partie_3/break_code_c2.h ./Partie_3/break_code_c3.h ./Partie_3/mutex.h
./Partie_3/crack_mask.o : ./Partie_1/chiffrement.h ./Partie_3/crackage.h
./Partie_3/c1_stockingKeys.o : ./Partie_3/c1_stockingKeys.h ./Partie_3/break_code_c1.h ./utilitaire/utiL.h


# links for the main of all the mains
crypto.o : ./Partie_1/chiffrement.h ./Partie_3/crackage.h ./utilitaire/utiL.h
./utilitaire/utiL.o : ./utilitaire/utiL.h
