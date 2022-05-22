CFLAGS=-g -Wall -Werror -Wno-parentheses -pthread -D _SYSV -D DEBUG
INC=./inc
SRCS=$(wildcard src/*.c)
SRCS2=$(addprefix src/,$(SRCS))
OBJS=$(addprefix obj/,$(notdir $(SRCS:.c=.o)))
LIBS=
TARGET=gs.out

all: $(TARGET)

obj/%.o: src/%.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c -I $(INC) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -I $(INC) -o $@ $^

clean:
	rm -rf $(TARGET) obj/*.o
