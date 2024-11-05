CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow -Wno-discarded-qualifiers \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
			 -Wjump-misses-init -Wlogical-op -g -O3 -D_XOPEN_SOURCE=700

LFLAGS = -lssl -lcrypto
INCLUDE = -I./include
STATIC = -static

FILES = lib \
		  linked_list \
		  tree \
		  http_tree \
		  parsing \
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
MAIN_OUTPUT = http

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(SSL) $(CFLAGS) -c $< -o $@ $(INCLUDE)

$(BUILD_DIR)/%-no-ssl.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

http: dir $(OBJECTS_DIR)
	$(CC) $(SSL) $(CFLAGS) -o $(MAIN_OUTPUT) $(SRC_DIR)/main.c $(OBJECTS_DIR) $(INCLUDE) $(LFLAGS)

http-static: dir $(OBJECTS_DIR)
	$(CC) $(SSL) $(CFLAGS) -o $(MAIN_OUTPUT) $(SRC_DIR)/main.c $(OBJECTS_DIR) $(INCLUDE) $(LFLAGS) $(STATIC)

http-no-ssl: dir $(OBJECTS_DIR_NO_SSL)
	$(CC) $(CFLAGS) -o $(MAIN_OUTPUT) $(SRC_DIR)/main.c $(OBJECTS_DIR_NO_SSL) $(INCLUDE)

docker-image:
	docker build -t http-server .


TEST_FILES = test_main test_parsing
TEST_OBJECTS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(TEST_FILES)))
TEST_OUTPUT = tests_main
TEST_LFLAGS = $(LFLAGS) -lcheck -lsubunit -lm

$(BUILD_DIR)/test_%.o: tests/test_%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE) -lm -lsubunit

tests: dir $(TEST_OBJECTS) $(OBJECTS_DIR)
	$(CC) $(CFLAGS) -o $(TEST_OUTPUT) $(TEST_OBJECTS) $(OBJECTS_DIR) $(INCLUDE) $(TEST_LFLAGS)

run-tests: tests
	./$(TEST_OUTPUT)


clean:
	rm -rf logs $(MAIN_OUTPUT) $(TEST_OUTPUT) $(BUILD_DIR)