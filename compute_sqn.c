#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common_functions.h"

static uint64_t snq;

typedef struct opt_solution {
    uint64_t *left;
    uint64_t *right;
    int64_t *conditions;
    struct opt_solution *next;
} opt_solution;

static opt_solution *optimum;

int fill_second_half(uint64_t *left, uint64_t *right, int n, int current_level, int64_t *conditions) {
    while (current_level < n - 1) {
        if (conditions[current_level] > 0) {
            left[current_level] = compute_max_cardinality(left, right, current_level);
            right[current_level] = 0;
        } else {
            left[current_level] = 0;
            right[current_level] = compute_max_cardinality(left, right, current_level);
        }
        current_level++;
    }
    uint64_t current_code_size = compute_max_cardinality(left, right, current_level);
    if (current_code_size > snq) {
        snq = current_code_size;

        if (optimum != NULL) {
            opt_solution *next = optimum->next;
            while(next != NULL) {
                free(next->left);
                free(next->right);
                free(next->conditions);
                next = next->next;
            }
        } else {
            optimum = (opt_solution *) malloc(sizeof(opt_solution));
            optimum->left = (uint64_t *) malloc(n * sizeof(uint64_t));
            optimum->right = (uint64_t *) malloc(n * sizeof(uint64_t));
            optimum->conditions = (int64_t *) malloc(n * sizeof(int64_t));
        }
        optimum->next = NULL;
        for (int i = 0; i < n; i++) {
            optimum->left[i] = left[i];
            optimum->right[i] = right[i];
            optimum->conditions[i] = conditions[i];
        }
    }
    else if (current_code_size == snq) {
        opt_solution *new_optimum = (opt_solution *) malloc(sizeof(opt_solution));
        new_optimum->left = (uint64_t *) malloc(n * sizeof(uint64_t));
        new_optimum->right = (uint64_t *) malloc(n * sizeof(uint64_t));
        new_optimum->conditions = (int64_t *) malloc(n * sizeof(int64_t));
        new_optimum->next = NULL;

        for (int i = 0; i < n; i++) {
            new_optimum->left[i] = left[i];
            new_optimum->right[i] = right[i];
            new_optimum->conditions[i] = conditions[i];
        }

        opt_solution *next = optimum;
        while (next->next != NULL) {
            next = next->next;
        }
        next->next = new_optimum;
    }
    return 0;
}

int solutions_iterator(uint64_t *left, uint64_t *right, int n, int q, int current_level, int64_t **conditions,
                       int64_t **parameters) {
    if (current_level == 0) {
        for (int i = 1; i <= q / 2; i++) {
            printf("size of L1: %d\n", i);
            left[0] = i;
            right[0] = q - i;
            compute_conditions(left, right, conditions, parameters, current_level, n);
            solutions_iterator(left, right, n, q, 1, conditions, parameters);
        }
        return 0;
    }

    if (current_level == n / 2) {
        fill_second_half(left, right, n, current_level, conditions[0]);
    } else {
        uint64_t current_code_size = compute_max_cardinality(left, right, current_level);
        int test_cut = 1;

        for (int i = 0; i < current_level; i++) {
            if (left[i] != right[i]) {
                test_cut = 0;
                break;
            }
        }

        for (uint64_t i = 0; i <= current_code_size; i++) {
            if (test_cut && i > current_code_size / 2) {
                break;
            }

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
    optimum = NULL;

    uint64_t *left = (uint64_t *) malloc(n * sizeof(uint64_t));
    uint64_t *right = (uint64_t *) malloc(n * sizeof(uint64_t));
    solutions_iterator_caller(left, right, n, q);

    free(left);
    free(right);

    printf("n: %d, q: %d, S(n,q): %ld\n", n, q, snq);

    while(optimum != NULL) {
        printf("L: ");
        for (int i = 0; i < n - 1; i++) {
            printf("%ld ", optimum->left[i]);
        }
        printf("R: ");
        for (int i = 0; i < n - 1; i++) {
            printf("%ld ", optimum->right[i]);
        }
        printf("; Conditions: ");
        for (int i = n / 2; i < n - 1; i++) {
            printf("%ld ", optimum->conditions[i]);
        }
        printf("\n");
        optimum = optimum->next;
    }
    return 0;
}

