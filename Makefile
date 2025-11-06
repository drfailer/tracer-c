CC=gcc
CFLAGS=-Wall -Wextra -Wuninitialized -Wno-sign-compare -fdiagnostics-color=auto \
	   -fPIC -g -O3
LDFLAGS=
SRC=$(wildcard tracer/*.c)
OBJ=$(addprefix build/,$(SRC:tracer/%.c=%.o))
DEP=$(addprefix build/,$(SRC:tracer/%.c=%.d))

.PHONY all: build/lib/libtracer.so build/lib/libtracer.a

build/lib/libtracer.so: $(OBJ)
	@mkdir -p build/lib
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -shared

build/lib/libtracer.a: $(OBJ)
	@mkdir -p build/lib
	ar rcs $@ $^

build/%.o: tracer/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

clean:
	rm -rf build

-include $(DEP)
