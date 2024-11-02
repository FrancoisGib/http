CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow -Wno-discarded-qualifiers \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
			 -Wjump-misses-init -Wlogical-op -O3 -D_POSIX_C_SOURCE=200112L

LFLAGS = -lssl -lcrypto

OBJECTS = lib.o \
			 linked_list.o \
          tree.o \
          http_tree.o \
			 logger.o \
			 ssl.o \
			 http.o

DIR = objects

OBJECTS_DIR = $(addprefix $(DIR)/, $(OBJECTS))

dir:
	mkdir -p objects

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $(DIR)/$@

http: dir $(OBJECTS)
	$(CC) $(CFLAGS) -o http main.c $(OBJECTS_DIR) $(LFLAGS)

docker-image: dir $(OBJECTS)
	gcc -o http main.c $(OBJECTS_DIR) -L/usr/local/openssl/lib -I/usr/local/openssl/include $(LFLAGS) -static
	docker build -t http-server .

clean:
	rm -rf *.o http logs objects