CC     = gcc
CFLAGS = -Wall -Wextra -pedantic
 
# Arquivos fonte e objeto
SRCS = rush.c
OBJS = $(SRCS:.c=.o)
 
# Alvo padrão
all: rush
 
# Linka os objetos no binário final
rush: $(OBJS)
	$(CC) $(CFLAGS) -o rush $(OBJS)
 
# Compila cada .c em .o
%.o: %.c rush.h
	$(CC) $(CFLAGS) -c $< -o $@
 
# Remove arquivos gerados
clean:
	rm -f $(OBJS) rush
 
.PHONY: all clean