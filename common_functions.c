#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "common_functions.h"

uint128_t compute_max_cardinality(const uint128_t *left, const uint128_t *right, int set_index) {
    uint128_t size = 0;
    for (int i = 0; i < set_index; i++) {
        size += left[i] * right[set_index - 1 - i];
    }
    return size;
}

int compute_conditions(const uint128_t *left, const uint128_t *right, int128_t **conditions,
                       int128_t **parameters, int index, int n) {
    for (int j = 0; j < index; j++) {
        for (int k = j - 1; k > -1; k--) {
            parameters[j][k] = 0;
            for (int l = k; l < j; l++) {
//                parameters[j][k] += (conditions[1][n - index + l - 2] * right[j - l - 1] +
//                                     (1 - conditions[1][n - index + l - 2]) * left[j - l - 1]) * parameters[l][k];
                parameters[j][k] += (conditions[1][n - index + l - 1] * right[j - l - 1] +
                                     (1 - conditions[1][n - index + l - 1]) * left[j - l - 1]) * parameters[l][k];
            }
        }
    }

    conditions[0][n - index - 2] = right[index] - left[index];
    for (int j = 0; j < index; j++) {
        int128_t value = 0;
        for (int l = 0; l <= j; l++) {
            value += parameters[j][l] * (right[l] - left[l]);
        }
        value *= (conditions[1][n - index + j - 1] * right[index - j - 1] +
                  (1 - conditions[1][n - index + j - 1]) * left[index - j - 1]);
        conditions[0][n - index - 2] += value;
    }

    if (conditions[0][n - index - 2] < 0) {
        conditions[1][n - index - 2] = 0;
    } else {
        conditions[1][n - index - 2] = 1;
    }

    return 0;
}

int128_t **initialize_conditions(int n) {
    int128_t **conditions = (int128_t **) malloc(2 * sizeof(int128_t *));
    conditions[0] = (int128_t *) malloc(n * sizeof(int128_t));
    conditions[1] = (int128_t *) malloc(n * sizeof(int128_t));

    return conditions;
}

int print_u128_u(uint128_t u128) {
    int rc;
    if (u128 > UINT64_MAX) {
        uint128_t leading = u128 / P10_UINT64;
        uint64_t trailing = u128 % P10_UINT64;
        rc = print_u128_u(leading);
        rc += printf("%." TO_STRING(E10_UINT64) PRIu64, trailing);
    } else {
        uint64_t u64 = u128;
        rc = printf("%" PRIu64, u64);
    }
    return rc;
}
