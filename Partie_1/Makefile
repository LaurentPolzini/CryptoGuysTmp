# Makefile

# Variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = encrypt_decrypt
OBJS = encrypt_decrypt_xor.o
GENERATED_FILES = message.txt message_encrypted.txt message_decrypted.txt

# Règle par défaut
all: $(TARGET)

# Règle pour créer l'exécutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Règle pour compiler le fichier C en fichier objet
encrypt_decrypt_xor.o: encrypt_decrypt_xor.c mask.h xor.h
	$(CC) $(CFLAGS) -c encrypt_decrypt_xor.c

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(GENERATED_FILES)
