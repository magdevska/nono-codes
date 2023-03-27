#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common_functions.h"

uint64_t optimal_values[5][22] = {
        {1,  1,   2,   3,    5,     8,     14,     24,      44,      81,     149,     274, 504, 927, 1705, 3160, 5969, 11272, 21287, 40202, 76424, 147312},
        {4, 8, 17, 41, 99, 247, 656, 1792, 4896, 13376, 36544, 99840, 274384, 759847,                2150616},
        {9,  27,  81,  251,  829,   2753,  9805,   34921,   124373,  446496, 1619604, 5941181},
        {18, 64,  256, 1024, 4181,  17711, 76816,  341792,  1520800, 6817031},
        {32, 128, 625, 3125, 15625, 79244, 411481, 2188243, 11755857}
};


int print_optimal_solutions(uint64_t *left, uint64_t *right, int n, int q, int current_level, int64_t **conditions,
                            int64_t **parameters) {
    if (current_level == 0) {
        for (int i = 1; i <= q / 2; i++) {
            left[0] = i;
            right[0] = q - i;
            compute_conditions(left, right, conditions, parameters, current_level, n);
            print_optimal_solutions(left, right, n, q, 1, conditions, parameters);
        }
        return 0;
    }

    uint64_t current_code_size = compute_max_cardinality(left, right, current_level);
    if (current_level == n - 1) {
        if (current_code_size == optimal_values[q - 2][n - 3]) {
            printf("L: ");
            for (int i = 0; i < n - 1; i++) {
                printf("%ld ", left[i]);
            }
            printf("R: ");
            for (int i = 0; i < n - 1; i++) {
                printf("%ld ", right[i]);
            }
            printf("; Conditions: ");
            for (int i = n / 2; i < n - 1; i++) {
                printf("%ld ", conditions[0][i]);
            }
            printf("\n");
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
        print_optimal_solutions(left, right, n, q, current_level + 1, conditions, parameters);
    } else {
        for (int i = 0; i <= current_code_size; i++) {
            left[current_level] = i;
            right[current_level] = current_code_size - i;

            compute_conditions(left, right, conditions, parameters, current_level, n);
            print_optimal_solutions(left, right, n, q, current_level + 1, conditions, parameters);
        }
    }
    return 0;
}

int print_optimal_solutions_caller(uint64_t *left, uint64_t *right, int n, int q) {
    int64_t **conditions = initialize_conditions(n);

    int64_t **parameters = (int64_t **) malloc((n + 1) / 2 * sizeof(int64_t *));

    for (int i = 0; i < (n + 1) / 2; i++) {
        parameters[i] = (int64_t *) malloc((i + 1) * sizeof(int64_t));
        parameters[i][i] = 1;
    }

    print_optimal_solutions(left, right, n, q, 0, conditions, parameters);

    free(conditions[0]);
    free(conditions[1]);
    free(conditions);
    for (int i = 0; i < (n + 1) / 2; i++) {
        free(parameters[i]);
    }
    free(parameters);

    return 0;
}

int main(int argc, char **argv) {
    int q = (int) strtol(argv[1], NULL, 10);
    int n = (int) strtol(argv[2], NULL, 10);

    if (q < 2 || n < 3) {
        printf("invalid parameter values\n");
        return 0;
    }

    uint64_t *left = (uint64_t *) malloc(n * sizeof(uint64_t));
    uint64_t *right = (uint64_t *) malloc(n * sizeof(uint64_t));

    print_optimal_solutions_caller(left, right, n, q);

    free(left);
    free(right);

    return 0;
}

