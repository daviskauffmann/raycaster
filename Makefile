CC := gcc
CFLAGS := -ggdb -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter `pkg-config --cflags sdl2 sdl2_image sdl2_ttf`
LDFLAGS := `pkg-config --libs sdl2 sdl2_image sdl2_ttf` -mconsole

SRC	:= src
BUILD := build
BIN	:= bin

SOURCES	:= $(SRC)/main.c
OBJECTS := $(SOURCES:$(SRC)/%.c=$(BUILD)/%.o)
DEPENDENCIES := $(OBJECTS:%.o=%.d)
INCLUDE :=
LIB :=
LIBRARIES :=
TARGET := $(BIN)/raycaster

.PHONY: all
all: $(TARGET)
	@echo $(LDFLAGS)

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
