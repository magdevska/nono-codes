#include <stdio.h>
#include <stdlib.h>

#include "common_functions.h"

typedef struct opt_solution {
    uint128_t *left;
    uint128_t *right;
    struct opt_solution *next;
} opt_solution;

opt_solution *optimum;

static uint128_t snq;

void update_solution(const uint128_t *left, const uint128_t *right, int n,
                     uint128_t current_code_size) {
    if (current_code_size > snq) {
        snq = current_code_size;

        opt_solution *next = optimum->next;
        while (next != NULL) {
            free(next->left);
            free(next->right);
            opt_solution *next_next = next->next;
            free(next);
            next = next_next;
        }

        optimum->next = NULL;
        for (int i = 0; i < n; i++) {
            optimum->left[i] = left[i];
            optimum->right[i] = right[i];
        }
    } else if (current_code_size == snq) {
        opt_solution *new_optimum = (opt_solution *) malloc(sizeof(opt_solution));
        new_optimum->left = (uint128_t *) malloc(n * sizeof(uint128_t));
        new_optimum->right = (uint128_t *) malloc(n * sizeof(uint128_t));
        new_optimum->next = NULL;

        for (int i = 0; i < n; i++) {
            new_optimum->left[i] = left[i];
            new_optimum->right[i] = right[i];
        }

        opt_solution *next = optimum;
        while (next->next != NULL) {
            next = next->next;
        }
        next->next = new_optimum;
    }
}


int128_t compute_conditions_lowerbound(uint128_t *left, uint128_t *right, int128_t **parameters, int index) {
    for (int j = 0; j < index; j++) {
        for (int k = j - 1; k > -1; k--) {
            parameters[j][k] = 0;
            for (int l = k; l < j; l++) {
                parameters[j][k] += right[j - l - 1] * parameters[l][k];
            }
        }
    }

    int128_t conditions = compute_max_cardinality(left, right, index);
    for (int j = 0; j < index; j++) {
        int value = 0;
        for (int l = 0; l <= j; l++) {
            value += parameters[j][l] * (right[l] - left[l]);
        }
        value *= right[index - j - 1];
        conditions += value;
    }
    return conditions;
}


int lower_bound(uint128_t *left, uint128_t *right, int n, int q, int current_level, int128_t **parameters) {
    if (current_level == 0) {
        for (int i = 1; i <= q / 2; i++) {
            left[0] = i;
            right[0] = q - i;
            lower_bound(left, right, n, q, 1, parameters);
        }
        return 0;
    }

    uint128_t current_code_size = compute_max_cardinality(left, right, current_level);
    if (current_level == n - 1) {
        update_solution(left, right, n, current_code_size);
        return 0;
    }

    int n_bound = (int) (n / (q + 1)) + 1;
    if (current_level <= n_bound && current_level < n / 2) {
        int128_t cond = compute_conditions_lowerbound(left, right, parameters, current_level);
        for (int128_t i = 0; 2 * i <= cond && (uint128_t) i <= current_code_size; i++) {
            left[current_level] = i;
            right[current_level] = current_code_size - i;
            lower_bound(left, right, n, q, current_level + 1, parameters);
        }
    } else {
        left[current_level] = current_code_size;
        right[current_level] = 0;
        lower_bound(left, right, n, q, current_level + 1, parameters);
    }
    return 0;
}

int main(__attribute__((unused)) int argc, char **argv) {
    int q = (int) strtol(argv[2], NULL, 10);
    int n = (int) strtol(argv[3], NULL, 10);

    if (q < 2 || n < 3) {
        printf("invalid parameter values\n");
        return 0;
    }

    uint128_t *left = (uint128_t *) malloc(n * sizeof(uint128_t));
    uint128_t *right = (uint128_t *) malloc(n * sizeof(uint128_t));
    int128_t **parameters = (int128_t **) malloc(n * sizeof(int128_t *));
    for (int i = 0; i < n; i++) {
        parameters[i] = (int128_t *) malloc((i + 1) * sizeof(int128_t));
        parameters[i][i] = 1;
    }

    optimum = (opt_solution *) malloc(sizeof(opt_solution));
    optimum->left = (uint128_t *) malloc(n * sizeof(uint128_t));
    optimum->right = (uint128_t *) malloc(n * sizeof(uint128_t));
    optimum->next = NULL;

    lower_bound(left, right, n, q, 0, parameters);

    for (int i = 0; i < n; i++) {
        free(parameters[i]);
    }
    free(parameters);
    free(left);
    free(right);

    printf("n: %d, q: %d, S(n,q): ", n, q);
    print_u128_u(snq);
    printf("\n");

    while (optimum != NULL) {
        printf("L: ");
        for (int i = 0; i < n - 1; i++) {
            print_u128_u(optimum->left[i]);
            printf(" ");
        }
        printf(" R: ");
        for (int i = 0; i < n - 1; i++) {
            print_u128_u(optimum->right[i]);
            printf(" ");
        }
        printf("\n");
        free(optimum->left);
        free(optimum->right);
        opt_solution *next = optimum->next;
        free(optimum);
        optimum = next;
    }

    return 0;
}

