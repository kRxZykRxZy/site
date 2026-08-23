CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS=-pthread -lcurl -lssl -lcrypto
SRC=src/main.c src/http.c src/monitor.c src/services.c src/ai.c src/notify.c src/push.c
OBJ=$(SRC:.c=.o)

all: pi-guardian

pi-guardian: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) pi-guardian

.PHONY: all clean
