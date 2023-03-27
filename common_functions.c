#include <stdlib.h>
#include <stdint.h>
#include "common_functions.h"

uint64_t compute_max_cardinality(uint64_t *left, uint64_t *right, int set_index) {
    uint64_t size = 0;
    for (int i = 0; i < set_index; i++) {
        size += left[i] * right[set_index - 1 - i];
    }
    return size;
}

int compute_conditions(uint64_t *left, uint64_t *right, int64_t **conditions, int64_t **parameters, int index, int n) {
    for (int j = 0; j < index; j++) {
        for (int k = j - 1; k > -1; k--) {
            parameters[j][k] = 0;
            for (int l = k; l < j; l++) {
                parameters[j][k] += (conditions[1][n - index + l - 2] * right[j - l - 1] +
                                     (1 - conditions[1][n - index + l - 2]) * left[j - l - 1]) * parameters[l][k];
            }
        }
    }

    conditions[0][n - index - 2] = right[index] - left[index];
    for (int j = 0; j < index; j++) {
        int value = 0;
        for (int l = 0; l <= j; l++) {
            value += parameters[j][l] * (right[l] - left[l]);
        }
        value *= (conditions[1][n - index + j - 2] * right[index - j - 1] +
                  (1 - conditions[1][n - index + j - 2]) * left[index - j - 1]);
        conditions[0][n - index - 2] += value;
    }

    if (conditions[0][n - index - 2] < 0) {
        conditions[1][n - index - 2] = 0;
    } else {
        conditions[1][n - index - 2] = 1;
    }

    return 0;
}

int64_t **initialize_conditions(int n) {
    int64_t **conditions = (int64_t **) malloc(2 * sizeof(int64_t *));
    conditions[0] = (int64_t *) malloc(n * sizeof(int64_t));
    conditions[1] = (int64_t *) malloc(n * sizeof(int64_t));

    return conditions;
}
