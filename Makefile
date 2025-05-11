SRCDIR := src
OBJDIR := obj
BINDIR := bin

SHELL := /bin/bash

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.c=.o)))
EXE := $(BINDIR)/nes

CC := gcc
CFLAGS := -DBUILD_DEBUG \
		  -lm -lSDL3 \
		  -std=c23 \
		  -g3 \
		  -pedantic \
		  -Wall \
		  -Werror \
		  -Wextra \
		  -Wconversion \
		  -Wno-unused-parameter \
		  -Wno-unused-function \
		  -Wno-sign-conversion

all: clean build

$(BINDIR):
	mkdir -p "$(@)"

$(OBJDIR):
	mkdir -p "$(@)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJDIR)
	$(CC) -c -o "$(@)" "$(<)" $(CFLAGS)

build: $(BINDIR) $(OBJ)
	$(CC) -o "$(EXE)" $(OBJ) $(CFLAGS)

clean:
	rm -f $(OBJDIR)/*.o $(EXE)

.PHONY: all clean build
