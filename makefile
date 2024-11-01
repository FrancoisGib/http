CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow -Wno-discarded-qualifiers \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
			 -Wjump-misses-init -Wlogical-op -O3 -D_POSIX_C_SOURCE=200112L

OBJECTS = lib.o \
			 linked_list.o \
          tree.o \
          http_tree.o \
			 logger.o \
			 http.o

DIR = objects

OBJECTS_DIR = $(addprefix $(DIR)/, $(OBJECTS))

dir:
	mkdir -p objects

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $(DIR)/$@

http: dir $(OBJECTS)
	$(CC) $(CFLAGS) -o http main.c $(OBJECTS_DIR)

docker-image: http.o $(OBJECTS)
	$(CC) --static $(CFLAGS) -o http main.c $(OBJECTS_DIR)
	docker build -t http-server .

clean:
	rm -rf *.o http logs objects