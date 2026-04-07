CC := gcc

TARGET := xsrend

CFLAGS := -std=gnu23 -march=native -O3
CFLAGS += -Wall -Wextra
CFLAGS += -Wconversion -Wsign-conversion -Wfloat-equal
CFLAGS += -Wwrite-strings -Wstrict-prototypes -Wold-style-definition
CFLAGS += -Wswitch-default -Winit-self -Wundef
CFLAGS += -Wpointer-arith -Wcast-align
CFLAGS += -Wshadow
CFLAGS += -Wno-format-truncation

DFLAGS := -g -O0 -DDEBUG
SFLAGS := -fsanitize=address -fno-omit-frame-pointer

LFLAGS := lib/libSDL2.a
LFLAGS += -lm

SOURCE := src/main.c src/backend_sdl.c src/camera.c src/text.c
OBJECT := $(SOURCE:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) $(OBJECT) $(LFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# debug build
debug: CFLAGS := $(CFLAGS) $(DFLAGS) $(SFLAGS)
debug: LFLAGS := $(LFLAGS) $(SFLAGS)
debug: rebuild

clean:
	rm -f $(OBJECT) && rm -f $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild debug
