CC := gcc
CFLAGS := -ggdb -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-type-limits `pkg-config --cflags sdl2 sdl2_image sdl2_ttf`
CPPFLAGS :=
LDFLAGS := `pkg-config --libs sdl2 sdl2_image sdl2_ttf` -mconsole
LDLIBS :=

SRC	:= src/main.c
TARGET := bin/raycaster

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC:src/%.c=build/%.o)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ -MMD -MF $(@:.o=.d) $(CFLAGS) $(CPPFLAGS)

-include $(SRC:src/%.c=build/%.d)

.PHONY: run
run: all
	./$(TARGET)

.PHONY: clean
clean:
	rm -rf bin build
