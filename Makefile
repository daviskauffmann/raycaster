CC := gcc
CFLAGS := -ggdb -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter
LDFLAGS :=

SRC	:= src
DEPS := deps
BUILD := build
BIN	:= bin
EXE := raycaster

SOURCES	:= $(SRC)/main.c
OBJECTS := $(patsubst $(SRC)/%,$(BUILD)/%,$(SOURCES:.c=.o))
DEPENDENCIES := $(OBJECTS:.o=.d)
INCLUDE :=
LIB :=
LIBRARIES := -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
TARGET := $(BIN)/$(EXE)

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIB) $(LIBRARIES)

$(BUILD)/%.o: $(SRC)/%.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ -MMD -MF $(@:.o=.d) $(CFLAGS) $(INCLUDE)

-include $(DEPENDENCIES)

.PHONY: run
run: all
	./$(TARGET)

.PHONY: clean
clean:
	rm -rf $(BIN) $(BUILD)
