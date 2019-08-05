CC := gcc
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := bin/jvscore

SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -std=gnu99
DEFINES :=
LIB := -pthread
INC := -I include

$(TARGET): $(OBJECTS)
	@echo " Linking:"
	@echo "  $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo " Building:"
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(BINDIR)
	@echo "  $(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(DEFINES) $(INC) -c -o $@ $<

install:
	@echo " Installing"
	@echo "  cp $(TARGET) /usr/bin/jvscore"; cp $(TARGET) /usr/$(TARGET)
	@echo "  cp -n docs/jvscore.conf /etc/jvscore.conf"; cp -n docs/jvscore.conf /etc/jvscore.conf

uninstall:
	@echo " Removing"
	@echo "  rm -f /usr/bin/jvscore"; rm -f /usr/bin/jvscore
	@echo "  rm -f /etc/jvscore.conf"; rm -f /etc/jvscore.conf

clean:
	@echo " Cleaning";
	@echo "  $(RM) -r $(BUILDDIR) $(TARGET) $(BINDIR)"; $(RM) -r $(BUILDDIR) $(TARGET) $(BINDIR)

.PHONY: clean
