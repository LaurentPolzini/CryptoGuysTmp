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

SRC_UTILS=../utilitaire/utiL.c

EXEC_BREAK_CODE=break_code
EXEC_CRACK_MASK=crack_mask

SRC_UTILS=../utilitaire/utiL.c
SRC_ALL=$(wildcard *.c) $(SRC_UTILS)

# Filtrer crack_mask.c pour break_code
SRC_BREAK_CODE=../Partie_1/encrypt_decrypt_xor.c $(filter-out crack_mask.c, $(SRC_ALL))
SRC_CRACK_MASK=crack_mask.c $(SRC_UTILS)

OBJ_BREAK= $(SRC_BREAK_CODE:.c=.o)
OBJ_MASK= $(SRC_CRACK_MASK:.c=.o)

all:
ifeq ($(DEBUG),yes)
	@echo "Generating in debug mode"
else
	@echo "Generating in release mode"
endif
	@$(MAKE) $(EXEC_BREAK_CODE)
	@$(MAKE) $(EXEC_CRACK_MASK)

$(EXEC_BREAK_CODE): $(OBJ_BREAK)
	@$(CC) -o $@ $^ $(LDFLAGS)
# @$(MAKE) -C ../Partie_1

$(EXEC_CRACK_MASK): $(OBJ_MASK)
	@$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)


# Special target for AddressSanitizer build
asan: CFLAGS += -fsanitize=address -g
asan: LDFLAGS += -fsanitize=address
asan: CC=clang
asan: $(EXEC_BREAK_CODE)

clean:
	@rm -rf $(OBJ_BREAK) $(OBJ_MASK) $(EXEC_BREAK_CODE) $(EXEC_CRACK_MASK)

.PHONY: clean mrproper

mrproper: clean
	@rm -rf $(EXEC_BREAK_CODE) $(EXEC_CRACK_MASK)


# autres_methodes/c1_stockingKeys.o : autres_methodes/c1_stockingKeys.h break_code_c1.h ../utilitaire/utiL.h caracteresCandidatsIndexKey.h autres_methodes/Tree.h
# autres_methodes/ligne0.o : autres_methodes/ligne0.h break_code_c1.h ../utilitaire/utiL.h caracteresCandidatsIndexKey.h
# autres_methodes/NTree.o : autres_methodes/NTree.h Pile.h ../utilitaire/utiL.h
# autres_methodes/Tree.o : autres_methodes/Tree.h Pile.h break_code_c1.h ../utilitaire/utiL.h caracteresCandidatsIndexKey.h

caracteresCandidatsIndexKey.o : caracteresCandidatsIndexKey.h
ThreadSegmentationTableauxIndex.o : ThreadSegmentationTableauxIndex.h
Pile.o : Pile.h ../utilitaire/utiL.h
../utilitaire/utiL.o : ../utilitaire/utiL.h

break_code_c1.o : Pile.h crackage.h break_code_c1.h ../utilitaire/utiL.h ../Partie_1/chiffrement.h ThreadSegmentationTableauxIndex.h caracteresCandidatsIndexKey.h tests_crackage.h mutex.h
break_code_c2.o : crackage.h ../utilitaire/utiL.h break_code_c1.h break_code_c2.h break_code_c2_c3.h mutex.h
break_code_c3.o : crackage.h break_code_c3.h ../utilitaire/utiL.h break_code_c2_c3.h mutex.h
break_code_c2_c3.o : break_code_c2_c3.h break_code_c1.h break_code_c2.h break_code_c3.h mutex.h
crack_mask.o : ../Partie_1/chiffrement.h crackage.h

mutex.o : mutex.h
tests_crackage.o : tests_crackage.h ../utilitaire/uthash.h ../utilitaire/utiL.h mutex.h

break_code_main.o : break_code_c1.h break_code_c2.h break_code_c3.h ../utilitaire/utiL.h ../Partie_1/chiffrement.h tests_crackage.h
