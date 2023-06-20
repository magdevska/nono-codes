CC=gcc
OBJECTS=\
	compute_sqn \
	optimal_solutions \
	sqn_lowerbound
SHARED_FILES=common_functions.c common_functions.h
CFLAGS=-std=c11 -Wall -Wextra -pedantic -pipe -lm -pthread
OPTIMIZATION_FLAGS=-Ofast
DEBUG_FLAGS=-g3 -Og

# the default is to build all binaries
all: $(OBJECTS)

# every binary: captures its name: its prerequisite is the .c file
# additional whitespace for readable visual output
$(OBJECTS): %: %.c
	$(CC) $(CFLAGS)     $(OPTIMIZATION_FLAGS)    -o $@.out           $(SHARED_FILES) $<
	$(CC) $(CFLAGS) $(DEBUG_FLAGS)       -o $@.out.debug     $(SHARED_FILES) $<

.PHONY: clean
clean:
	rm -vf *.out
	rm -vf *.out.debug

# a run-all consistency command
# for each obj in OBJECTS, add another prerequisite on consistency-obj
.PHONY: consistency
consistency: $(foreach obj, $(OBJECTS), consistency-$(obj))

# single consistency runs
# for each obj in OBJECTS, define a rule named consistency-obj (referenced above)
$(foreach obj, $(OBJECTS), consistency-$(obj)): consistency-%: %
	mkdir -p consistency/
	seq 2 6 | xargs -I XXX sh -c "seq 3 9 | xargs -I YYY ./$<.out 0 XXX YYY" | tee consistency/$<-serial.txt
	seq 2 6 | xargs -I XXX sh -c "seq 3 9 | xargs -I YYY ./$<.out 2 XXX YYY" | tee consistency/$<-maxlevel2.txt

# a prerequisite for performance runs (which benchmarks all binaries) is that they're compiled
.PHONY: performance
performance: $(OBJECTS)
	mkdir -p performance/
	hyperfine --warmup 2 --min-runs 10 --max-runs 500 --style full --time-unit millisecond --shell none --export-markdown "performance/$$(date --iso-8601=s).md" $(foreach obj, $(OBJECTS), "./$(obj).out 3 12 6")
