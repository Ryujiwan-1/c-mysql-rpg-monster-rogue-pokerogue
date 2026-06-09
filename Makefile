CC = gcc
MYSQL_CFLAGS = $(shell mysql_config --cflags 2>/dev/null)
BREW_LIBS = -L/opt/homebrew/opt/zstd/lib -L/opt/homebrew/opt/openssl@3/lib
CFLAGS = -Wall -Wextra -std=c11 -DUSE_MYSQL -Iinclude $(MYSQL_CFLAGS)
TARGET = build/monster_rogue
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=build/%.o)
MYSQL57_LIB = lib/libmysqlclient.a

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(MYSQL57_LIB) $(BREW_LIBS) -lz -lzstd -lssl -lcrypto -lresolv -lc++

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build .build
