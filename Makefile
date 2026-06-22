CC = gcc
CFLAGS = -Wall -Wextra -pedantic -pthread

# Arquivos fonte e objeto
SRCS = formigopolis.c
OBJS = $(SRCS:.c=.o)

# Alvo padrão
all: formigopolis

# Linka os objetos no binário final
formigopolis: $(OBJS)
	$(CC) $(CFLAGS) -o formigopolis $(OBJS)

# Compila cada .c em .o
%.o: %.c formigopolis.h
	$(CC) $(CFLAGS) -c $< -o $@

# Remove arquivos gerados
clean:
	rm -f $(OBJS) formigopolis

.PHONY: all clean