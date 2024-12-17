# Variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = encrypt_decrypt
SYMTARGET = sym_crypt
MAINTARGET = main_partie1
OBJS = encrypt_decrypt_xor.o sym_crypt.o main_partie1.o ../utilitaire/utiL.o
GENERATED_FILES = message.txt message_encrypted.txt message_decrypted.txt iv.txt message_encrypted.bin sym_crypt.o incorrect_key.txt incorrect_iv.txt

# Règle par défaut
all: $(TARGET)

.PHONY: asan
asan:
	$(MAKE) CFLAGS="$(CFLAGS) -fsanitize=address -g" CC=clang $(TARGET)

# Règle pour créer l'exécutable principal encrypt_decrypt
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Règle pour créer l'exécutable sym_crypt
sym_crypt: sym_crypt.o encrypt_decrypt_xor.o
	$(CC) $(CFLAGS) -o $(SYMTARGET) sym_crypt.o encrypt_decrypt_xor.o -lm

main_partie1: main_partie1.o encrypt_decrypt_xor.o
	$(CC) $(CFLAGS) -o $(MAINTARGET) main_partie1.o encrypt_decrypt_xor.o -lm

# Règle pour compiler les fichiers C en fichiers objets
encrypt_decrypt_xor.o: encrypt_decrypt_xor.c chiffrement.h ../utilitaire/utiL.h
	$(CC) $(CFLAGS) -c encrypt_decrypt_xor.c -lm

sym_crypt.o: sym_crypt.c chiffrement.h ../utilitaire/utiL.h
	$(CC) $(CFLAGS) -c sym_crypt.c -lm

../utilitaire/utiL.o: ../utilitaire/utiL.c ../utilitaire/utiL.h
	$(CC) $(CFLAGS) -c $< -o $@ -lm

# Nettoyage des fichiers générés
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(GENERATED_FILES)
