CC = gcc
CFLAGS = -Wall -Wextra -g

SRCDIR = .
BUILDDIR = build
BINDIR = bin

# Collecting source files from multiple directories
SOURCES := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/stack/*.c) $(wildcard $(SRCDIR)/queue/*.c $(wildcard $(SRCDIR)/list/*.c))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
EXECUTABLE = $(BINDIR)/main

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@ -lm # lm links the math lib

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR) $(BINDIR)