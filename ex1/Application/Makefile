# Author Markus Krainz
# Date 2018

CC := avr-gcc
LD := $(CC)
AVRPROG2 := avrprog2

CFLAGS   := -mmcu=atmega1280
CCFLAGS  := -std=gnu99 -Wall -Wno-main -O2 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -pedantic -Werror
CPPFLAGS := -I.
LDFLAGS  := -L.

.PHONY: all clean install

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
EXEC = main.elf

all: $(EXEC) install

%.o: %.cpp
	$(CC) $(CFLAGS) $(CCFLAGS) $(CPPFLAGS) -c -o $@ $<

$(EXEC): $(OBJECTS)
	$(LD) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

#$(LD) $(CFLAGS) $(LDFLAGS) -o $@ rand.o -lrandtest

install: $(EXEC)
	$(AVRPROG2) --flash w:$<

clean:
	rm -f main.elf rand.o






