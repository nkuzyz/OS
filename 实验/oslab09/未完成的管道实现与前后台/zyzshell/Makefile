.PHONY:clean
CC=gcc
CFLAGS=-Wall -g
BIN=zyzshell
OBJS=main.o parse.o init.o
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)

