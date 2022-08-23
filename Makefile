CC:=gcc
BIN:=ibm_dev
LDFLAGS:=-liotp-as-device -lubus -lubox
CPPFLAGS:=-I$(CURDIR)
SOURCES:=$(wildcard *.c)
OBJS:=$(SOURCES:.c=.o)

.PHONY: all clean

all:$(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	
clean:
	rm -rf $(BIN) $(OBJS)
