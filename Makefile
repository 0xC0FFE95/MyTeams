# Nom des fichiers sources et exécutables
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
SERVER_SRC = Server.c
CLIENT_SRC = client.c
SERVER_BIN = server
CLIENT_BIN = client

# Règle par défaut (compilation des deux programmes)
all: $(SERVER_BIN) $(CLIENT_BIN)

# Compilation du serveur
$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_SRC)

# Compilation du client
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC)

# Nettoyage des fichiers compilés
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

# Recompile tout proprement
re: clean all

