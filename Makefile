CC=cc
CFLAGS=-Wall -O2
LFLAGS=-lm -s

OBJS=frontend.o core.o

TARGET=emulator

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -fv -- $(OBJS) $(TARGET)
