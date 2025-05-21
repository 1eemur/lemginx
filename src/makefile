CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

SRCS = main.c logging.c file_utils.c http_utils.c client_handler.c
OBJS = $(SRCS:.c=.o)
TARGET = lemginx

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean