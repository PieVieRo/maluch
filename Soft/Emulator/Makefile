OBJ = main.o

CFLAGS=-std=c17 -Wall -Wextra

ifeq ($(DEBUG), 1)
	CFLAGS += -Og -ggdb
endif

# LDFLAGS=-lX11 -lGL -lm -lXrandr

all: emulator

emulator: $(OBJ)
	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) -o $@ $^

# $(OBJ): $(OBJ_DIR)/%.o: %.c
# 	@mkdir -p $(OBJ_DIR)
# 	$(CC) -c $^ $(CFLAGS) -o $@

clean: $(OBJ)
	rm $(OBJ)
