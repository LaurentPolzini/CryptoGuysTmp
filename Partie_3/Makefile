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

EXEC=break_code
SRC= ../utilitaire/utiL.c ../Partie_1/encrypt_decrypt_xor.c $(wildcard *.c) $(wildcard autres_methodes/*.c)
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
# @$(MAKE) -C ../Partie_1

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)


# Special target for AddressSanitizer build
asan: CFLAGS += -fsanitize=address -g
asan: LDFLAGS += -fsanitize=address
asan: CC=clang
asan: $(EXEC)

clean:
	@rm -rf $(OBJ) $(EXEC)

.PHONY: clean mrproper

mrproper: clean
	@rm -rf $(EXEC)


autres_methodes/c1_stockingKeys.o : autres_methodes/c1_stockingKeys.h break_code_c1.h ../utilitaire/utiL.h caracteresCandidatsIndexKey.h autres_methodes/Tree.h
autres_methodes/ligne0.o : autres_methodes/ligne0.h break_code_c1.h ../utilitaire/utiL.h caracteresCandidatsIndexKey.h
autres_methodes/NTree.o : autres_methodes/NTree.h Pile.h ../utilitaire/utiL.h
autres_methodes/Tree.o : autres_methodes/Tree.h Pile.h break_code_c1.h ../utilitaire/utiL.h caracteresCandidatsIndexKey.h

caracteresCandidatsIndexKey.o : caracteresCandidatsIndexKey.h
ThreadSegmentationTableauxIndex.o : ThreadSegmentationTableauxIndex.h
Pile.o : Pile.h ../utilitaire/utiL.h
../utilitaire/utiL.o : ../utilitaire/utiL.h

break_code_c1.o : Pile.h crackage.h break_code_c1.h ../utilitaire/utiL.h ../Partie_1/chiffrement.h ThreadSegmentationTableauxIndex.h caracteresCandidatsIndexKey.h
break_code_c2.o : crackage.h ../utilitaire/utiL.h break_code_c1.h break_code_c2.h break_code_c2_c3.h
break_code_c3.o : crackage.h break_code_c3.h ../utilitaire/utiL.h break_code_c2_c3.h
break_code_c2_c3.o : break_code_c2_c3.h break_code_c1.h break_code_c2.h break_code_c3.h 
crack_mask.o : ../Partie_1/chiffrement.h

break_code_main.o : break_code_c1.h break_code_c2.h break_code_c3.h ../utilitaire/utiL.h ../Partie_1/chiffrement.h
