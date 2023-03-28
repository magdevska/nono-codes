CC=gcc
OBJECTS=\
	compute_sqn \
	optimal_solutions \
	sqn_lowerbound
SHARED_FILES=common_functions.c common_functions.h
CFLAGS=-std=c11 -Wall -Wextra -pedantic -pipe -Ofast -lm

# the default is to build all binaries
all: $(OBJECTS)

# every binary: captures its name: its prerequisite is the .c file
$(OBJECTS): %: %.c
	$(CC) $(CFLAGS) -o $@.out $(SHARED_FILES) $<

.PHONY: clean
clean:
	rm -vf *.out

# a run-all consistency command
# for each obj in OBJECTS, add another prerequisite on consistency-obj
.PHONY: consistency
consistency: $(foreach obj, $(OBJECTS), consistency-$(obj))

# single consistency runs
# for each obj in OBJECTS, define a rule named consistency-obj (referenced above)
$(foreach obj, $(OBJECTS), consistency-$(obj)): consistency-%: %
	mkdir -p consistency/
	seq 2 6 | xargs -I XXX sh -c "seq 3 9 | xargs -I YYY ./$<.out XXX YYY" | tee consistency/$<.txt

# a prerequisite for performance runs (which benchmarks all binaries) is that they're compiled
.PHONY: performance
performance: $(OBJECTS)
	mkdir -p performance/
	hyperfine --warmup 2 --min-runs 10 --max-runs 500 --style full --time-unit millisecond --shell none --export-markdown "performance/$$(date --iso-8601=s).md" $(foreach obj, $(OBJECTS), "./$(obj).out 12 6")
