# Compiler and flags
CC = gcc
CFLAGS = -ggdb3 -Wall
LDLIBS = -lz -lpthread

# Targets
EXES = kvserver
TARGET = http_server

# Source files
HTTP_SRCS = http_server.c lab3.c
KV_SRCS = kvserver.c stats.c queue.c kvstore.c worker.c http_server.c lab3.c

# Object files (automatically inferred from sources)
HTTP_OBJS = $(HTTP_SRCS:.c=.o)
KV_OBJS = $(KV_SRCS:.c=.o)

# Rules
all: $(EXES) $(TARGET)

# Build the HTTP server
$(TARGET): $(HTTP_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(HTTP_OBJS) $(LDLIBS)

# Build kvserver
kvserver: $(KV_OBJS)
	$(CC) $(CFLAGS) -o kvserver $(KV_OBJS) $(LDLIBS)

# Clean up generated files
clean:
	rm -f $(EXES) $(TARGET) *.o data.[0-9]* *.trace
