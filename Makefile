CC := gcc
CFLAGS := -ggdb -std=c99 -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-type-limits `pkg-config --cflags sdl2 sdl2_image sdl2_ttf`
CPPFLAGS :=
LDFLAGS := `pkg-config --libs sdl2 sdl2_image sdl2_ttf` -mconsole
LDLIBS :=

SRC	:= src/main.c
OBJ := $(SRC:src/%.c=build/%.o)
DEP := $(OBJ:%.o=%.d)
TGT := bin/raycaster

.PHONY: all
all: $(TGT)

$(TGT): $(OBJ)
	mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) -c $< -o $@ -MMD -MF $(@:.o=.d) $(CFLAGS) $(CPPFLAGS)

-include $(DEP)

.PHONY: run
run: all
	./$(TGT)

.PHONY: clean
clean:
	rm -rf bin build
