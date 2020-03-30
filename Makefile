PKGS = sdl2 sdl2_image sdl2_ttf

CC = gcc
CFLAGS = -ggdb -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-type-limits `pkg-config --cflags $(PKGS)`
CPPFLAGS =
LDFLAGS = `pkg-config --libs $(PKGS)` -mconsole
LDLIBS =

SRC	= src/main.c
TARGET = bin/raycaster

.PHONY: all
all: $(TARGET)

$(TARGET): $(SRC:src/%.c=obj/%.o)
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

obj/%.o: src/%.c
	@mkdir -p $(@D)
	@mkdir -p $(@D:obj%=dep%)
	$(CC) -c $< -o $@ -MMD -MF $(@:obj/%.o=dep/%.d) $(CFLAGS) $(CPPFLAGS)

-include $(SRC:src/%.c=dep/%.d)

.PHONY: run
run: all
	./$(TARGET)

.PHONY: clean
clean:
	rm -rf bin obj dep
