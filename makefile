CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow -Wno-discarded-qualifiers \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
			 -Wjump-misses-init -Wlogical-op -O3 -D_POSIX_C_SOURCE=200112L

LFLAGS = -lssl -lcrypto
INCLUDE = -I./include
STATIC = -L/usr/local/openssl/lib64 -I/usr/local/openssl/include -static

OBJECTS = lib.o \
			 linked_list.o \
          tree.o \
          http_tree.o \
			 logger.o \
			 ssl.o \
			 http.o

SRC_DIR = src
BUILD_DIR = build

OBJECTS_DIR = $(addprefix $(BUILD_DIR)/, $(OBJECTS))

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

http: dir $(OBJECTS_DIR)
	$(CC) $(CFLAGS) -o http $(SRC_DIR)/main.c $(OBJECTS_DIR) $(INCLUDE) $(LFLAGS)

docker-image: dir $(OBJECTS_DIR)
	$(CC) -o http $(SRC_DIR)/main.c $(OBJECTS_DIR) $(INCLUDE) $(STATIC) $(LFLAGS)
	docker build -t http-server .

clean:
	rm -rf *.o http logs build