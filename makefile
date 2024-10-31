CC = gcc
CFLAGS = -Wall -Wextra -std=c11 \
          -Wformat=2 -Wno-unused-parameter -Wshadow -Wno-discarded-qualifiers \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
			 -Wjump-misses-init -Wlogical-op -O3 -D_POSIX_C_SOURCE=200112L

OBJECTS = linked_list.o \
          tree.o \
          http_tree.o \
			 logger.o

http: http.o $(OBJECTS)
	$(CC) $(CFLAGS) -o http http.o $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

docker-image: http.o $(OBJECTS)
	$(CC) --static $(CFLAGS) -o http http.o $(OBJECTS)
	docker build -t http-server .

clean:
	rm -f *.o http