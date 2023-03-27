#ifndef NONO_CODES_COMMON_FUNCTIONS_H
#define NONO_CODES_COMMON_FUNCTIONS_H

#include <stdint.h>

uint64_t compute_max_cardinality(const uint64_t *left, const uint64_t *right, int set_index);

int compute_conditions(const uint64_t *left, const uint64_t *right, int64_t **conditions,
                       int64_t **parameters, int index, int n);

int64_t **initialize_conditions(int n);

#endif //NONO_CODES_COMMON_FUNCTIONS_H
