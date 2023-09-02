SRC = $(shell find src/ -name '*.c')
RESOURCE_FILES = $(shell find resources/ -name '*.png')

OBJ = $(patsubst src/%.c, bin/obj/%.o, $(SRC))
RESOURCE_HEADERS = $(patsubst resources/%.png, resources/output/%.h, $(RESOURCE_FILES))

CC = tcc
CFLAGS +=	-Wall -Werror -Wno-pedantic -Wno-macro-redefined -Wno-unused-function -Wno-unused-parameter \
		-Wno-unused-variable -D_XOPEN_SOURCE=500 -I resources/output
LFLAGS += -lSDL2 -lSDL2_image -lm -lpthread

TARGET = bin/game

ifndef RELEASE
	CFLAGS+=-D_DEBUG -ggdb
else
	CFLAGS+=-Oz
endif

ifdef WIN_RELEASE
ifdef WIN32_RELEASE
	CC=i686-w64-mingw32-gcc
	LFLAGS+=-Lwindows/32/lib
	CFLAGS+=-Iwindows/32/include
	TARGET=bin/game32.exe
else
	CC=x86_64-w64-mingw32-gcc
	LFLAGS+=-Lwindows/64/lib
	CFLAGS+=-Iwindows/64/include
	TARGET=bin/game64.exe
endif
endif

all: clean_resources resources build

clean_resources:
	rm -rf $(RESOURCE_HEADERS)

resources: $(RESOURCE_HEADERS)
	@echo "Compiled all resources"

build: $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(LFLAGS)

run:
	./$(TARGET)

clean:
	@rm -rf $(OBJ) $(TARGET) $(RESOURCE_HEADERS)

resources/output/%.h: resources/%.png
	p2c -o $@ -i $< -f yx

bin/obj/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)
