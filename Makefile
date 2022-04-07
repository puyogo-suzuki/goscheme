CFLAGS=-I. -g -Wall -Werror
INCS=token.h string_t.h
OBJS=main.o token.o string_t.o
LIBS=
TARGET=main

%.o: %.c $(INCS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(TARGET) *.o