TARGET = emulator

OBJ = main.o maluch.o

DEP = $(OBJ:.o=.d)

CC ?= gcc
PYTHON ?= python3
CFLAGS = -std=c17 -Wall -Wextra -MMD -MP
LDLIBS = -lX11 -lGL -lm -lXrandr

ifeq ($(DEBUG), 1)
    CFLAGS += -Og -ggdb
else
    CFLAGS += -O2
endif

# Phony targets
.PHONY: all clean

all: $(TARGET) font.png

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

font.png: font.fnt font_to_png.py
	$(PYTHON) font_to_png.py

clean:
	rm -f $(OBJ) $(DEP) font.png $(TARGET)

# Include dependency files
-include $(DEP)
