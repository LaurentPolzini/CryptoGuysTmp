# Makefile

# Variables
CC=gcc
CFLAGS=-std=c99 -Wextra -Wall -Werror -pedantic
GENERATED_FILES=message.txt message_encrypted.txt message_decrypted.txt
LDFLAGS=-lm

ifeq ($(DEBUG),yes)
	CFLAGS += -g
	LDFLAGS +=
else
	CFLAGS += -O3 -DNDEBUG
	LDFLAGS +=
endif

SRC=encrypt_decrypt_xor.c
OBJ= $(SRC:.c=.o)
EXEC=crypt

# Règle par défaut
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

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJS) $(EXEC)
	rm -f $(GENERATED_FILES)

.PHONY: clean mrproper

mrproper: clean
	@rm -rf $(EXEC)


encrypt_decrypt_xor.o: encrypt_decrypt_xor.c mask.h xor.h
