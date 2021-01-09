CC = gcc
CFLAGS = -Wall -I./include
OBJS = src/main.o src/encode.o src/decode.o
TARGET = bin/huffman
DATA = *.zipdat expanded/*

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

.PHONY: tmpclean clean

tmpclean:
	find . -name *~ -exec rm -vf {} \;
clean: tmpclean
	rm -f $(OBJS) $(TARGET) $(DATA)
