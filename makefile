CC = gcc
CFLAGS = -Wall

OBJECTS = linked_list.o \
          tree.o \
          http_tree.o

http: http.o $(OBJECTS)
	$(CC) $(CFLAGS) -o http http.o $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

docker-image: http.o $(OBJECTS)
	$(CC) --static $(CFLAGS) -o http http.o $(OBJECTS)
	docker build -t http-server .

clean:
	rm -f *.o http