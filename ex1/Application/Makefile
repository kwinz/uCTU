# Author Markus Krainz
# Date 2018
# Some ideas from https://stackoverflow.com/questions/7004702/how-can-i-create-a-makefile-for-c-projects-with-src-obj-and-bin-subdirectories

CC := avr-gcc
LD := $(CC)
AVRPROG2 := avrprog2

SRCDIR   = src
OBJDIR   = build
BINDIR   = bin

EXEC   = build.elf

CFLAGS   := -mmcu=atmega1280
CCFLAGS  := -std=gnu99 -Wall -Wno-main -O2 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -pedantic -Wno-unused-but-set-variable #-Werror
CPPFLAGS := -I./includes -I./src
LDFLAGS  := -L./libs -lglcd -lwiimote -lsdcard -lmp3

SOURCES  := $(wildcard $(SRCDIR)/*.c)
#INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all clean install

all: $(EXEC) install


$(EXEC): $(OBJECTS)
	@$(LD) $(OBJECTS) libs/lcd.o $(CFLAGS) $(LDFLAGS) -o $(BINDIR)/$@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) $(CCFLAGS) $(CPPFLAGS)  -c $< -o $@
	@echo "Compiled "$<" successfully!"

install: $(EXEC)
	$(AVRPROG2) --flash w:$(BINDIR)/$<

clean:
	rm -f $(BINDIR)/* $(OBJDIR)/*






