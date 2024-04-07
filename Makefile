CC = gcc
CFLAGS = -Wall -Wextra -g

SRCDIR = .
STACKDIR = ./stack
QUEUEDIR = ./queue
BUILDDIR = build
BINDIR = bin

# Collecting source files from multiple directories
SOURCES := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/stack/*.c) $(wildcard $(SRCDIR)/queue/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
EXECUTABLE = $(BINDIR)/main

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR) $(BINDIR)