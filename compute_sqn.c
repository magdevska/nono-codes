#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common_functions.h"

static uint64_t snq;

int solutions_iterator(uint64_t *left, uint64_t *right, int n, int q, int current_level, int64_t **conditions,
                       int64_t **parameters) {
    if (current_level == 0) {
        for (int i = 1; i <= q / 2; i++) {
            left[0] = i;
            right[0] = q - i;
            compute_conditions(left, right, conditions, parameters, current_level, n);
            solutions_iterator(left, right, n, q, 1, conditions, parameters);
        }
        return 0;
    }

    uint64_t current_code_size = compute_max_cardinality(left, right, current_level);
    if (current_level == n - 1) {
        if (current_code_size > snq) {
            snq = current_code_size;
        }

        return 0;
    }

    if (current_level > n / 2) {
        if (conditions[1][current_level] == 1) {
            left[current_level] = current_code_size;
            right[current_level] = 0;
        } else {
            left[current_level] = 0;
            right[current_level] = current_code_size;
        }
        solutions_iterator(left, right, n, q, current_level + 1, conditions, parameters);
    } else {
        for (uint64_t i = 0; i <= current_code_size; i++) {
            left[current_level] = i;
            right[current_level] = current_code_size - i;

            compute_conditions(left, right, conditions, parameters, current_level, n);
            solutions_iterator(left, right, n, q, current_level + 1, conditions, parameters);
        }
    }
    return 0;
}

int solutions_iterator_caller(uint64_t *left, uint64_t *right, int n, int q) {
    int64_t **conditions = initialize_conditions(n);

    int64_t **parameters = (int64_t **) malloc((n + 1) / 2 * sizeof(int64_t *));

    for (int i = 0; i < (n + 1) / 2; i++) {
        parameters[i] = (int64_t *) malloc((i + 1) * sizeof(int64_t));
        parameters[i][i] = 1;
    }

    solutions_iterator(left, right, n, q, 0, conditions, parameters);

    free(conditions[0]);
    free(conditions[1]);
    free(conditions);
    for (int i = 0; i < (n + 1) / 2; i++) {
        free(parameters[i]);
    }
    free(parameters);

    return 0;
}

int main(__attribute__((unused)) int argc, char **argv) {
    int q = (int) strtol(argv[1], NULL, 10);
    int n = (int) strtol(argv[2], NULL, 10);

    if (q < 2 || n < 3) {
        printf("invalid parameter values\n");
        return 0;
    }

    snq = 0;

    uint64_t *left = (uint64_t *) malloc(n * sizeof(uint64_t));
    uint64_t *right = (uint64_t *) malloc(n * sizeof(uint64_t));
    solutions_iterator_caller(left, right, n, q);

    free(left);
    free(right);

    printf("n: %d, q: %d, S(n,q): %ld\n", n, q, snq);
    return 0;
}

