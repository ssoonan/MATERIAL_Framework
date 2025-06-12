CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c99 -D_GNU_SOURCE
LDFLAGS=-lrt -lpthread -lm

# Source directory
SRCDIR=src

# Build directory
BUILDDIR=build

# Sources
SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Target executable
TARGET=$(BUILDDIR)/Linux_EdgeSys_Base

.PHONY: all clean

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(BUILDDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)
	@echo "Clean complete"

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Install complete"

.PHONY: debug
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: release
release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)