CC=gcc
OBJECTS=\
	compute_sqn \
	optimal_solutions \
	sqn_lowerbound
SHARED_FILES=common_functions.c common_functions.h
CFLAGS=-std=c11 -Wall -Wextra -pedantic -Ofast -lm

# the default is to build all binaries
all: $(OBJECTS)

# every binary: captures its name: its prerequisite is the .c file
$(OBJECTS): %: %.c
	$(CC) $(CFLAGS) -o $@.out $(SHARED_FILES) $<

.PHONY: clean
clean:
	rm -vf *.out
