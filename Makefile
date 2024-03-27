CC = gcc
GDP = gdb
BIN = a.out

CPP_FLAGS   = -std=gnu2x -ggdb

WARNINGS    = -Wall \
              -Werror \
              -Wextra \
              -Wcast-align \
              -Wconversion \
              -Wpointer-arith \
              -Wshadow \
              -Wundef \
              -Wunreachable-code \
              -Wwrite-strings

LIBS        = -lm \
              -l wiringPi \
              -lGL \
              -lglfw \
              -lGLU \
              -lX11
              #-lGLEW
              #-lz
              #-lSOIL

SRCS = mpu.c mpu_i2c.c test.c mpu_integral.c mpu_graphics.c
OBJS = $(SRCS:.c=.o)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LIBS) $(CPP_FLAGS)
	@echo comiled successfully

%.o: %.c
	$(CC) $(CPP_FLAGS) $(WARNINGS) -c $< -o $@
	@echo comiled successfully

.PHONY: clean

clean:
	rm *.o
