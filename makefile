CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow -Wno-discarded-qualifiers \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
			 -Wjump-misses-init -Wlogical-op -O3 -D_XOPEN_SOURCE=700

LFLAGS = -lssl -lcrypto
INCLUDE = -I./include
STATIC = -L/usr/local/openssl/lib64 -I/usr/local/openssl/include -static

FILES = lib \
		  linked_list \
		  tree \
		  http_tree \
		  logger \
		  http

FILES_SSL = ssl \
				$(FILES)

OBJECTS = $(addsuffix .o, $(FILES_SSL))
OBJECTS_NO_SSL = $(addsuffix -no-ssl.o, $(FILES))

SRC_DIR = src
BUILD_DIR = build

OBJECTS_DIR = $(addprefix $(BUILD_DIR)/, $(OBJECTS))
OBJECTS_DIR_NO_SSL = $(addprefix $(BUILD_DIR)/, $(OBJECTS_NO_SSL))

SSL = -DUSE_SSL

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(SSL) $(CFLAGS) -c $< -o $@ $(INCLUDE)

$(BUILD_DIR)/%-no-ssl.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

http: dir $(OBJECTS_DIR)
	$(CC) $(SSL) $(CFLAGS) -o http $(SRC_DIR)/main.c $(OBJECTS_DIR) $(INCLUDE) $(LFLAGS)

http-no-ssl: dir $(OBJECTS_DIR_NO_SSL)
	$(CC) $(CFLAGS) -o http $(SRC_DIR)/main.c $(OBJECTS_DIR_NO_SSL) $(INCLUDE)

docker-image: dir $(OBJECTS_DIR_NO_SSL)
	$(CC) $(CFLAGS) -o http $(SRC_DIR)/main.c $(OBJECTS_DIR_NO_SSL) $(INCLUDE)
	docker build -t http-server .

clean:
	rm -rf *.o http logs build