# Author Markus Krainz
# Date 2018
# Some ideas from https://stackoverflow.com/questions/9178285/how-can-makefile-use-separate-directories-for-source-code-and-binaries

CC := avr-gcc
LD := $(CC)
AVRPROG2 := avrprog2

CFLAGS   := -mmcu=atmega1280
CCFLAGS  := -std=gnu99 -Wall -Wno-main -O2 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -pedantic -Werror
CPPFLAGS := -I./includes
LDFLAGS  := -L./libs

SRC_DIR     = ./src
BUILD_DIR   = ./build
BIN_DIR     = ./bin
SRC_LIST = $(wildcard $(SRC_DIR)/*.c)
OBJ_LIST = $(BUILD_DIR)/$(notdir $(SRC_LIST:.c=.o))

.PHONY: all clean install

# edit next line to name build.elf target name
EXEC = main.elf
# and this if I want to install automatically on all
all: $(EXEC) install

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(CCFLAGS) $(CPPFLAGS) -c -o $@ $<

$(EXEC): $(OBJ_LIST)
	$(LD) $(CFLAGS) $(LDFLAGS) $(OBJ_LIST) -o $(BIN_DIR)/$@

#$(LD) $(CFLAGS) $(LDFLAGS) -o $@ rand.o -lrandtest

install: $(EXEC)
	$(AVRPROG2) --flash w:$(BIN_DIR)/$<

clean:
	rm -f $(BIN_DIR)/* $(BUILD_DIR)/*






