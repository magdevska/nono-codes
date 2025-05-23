#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "common_functions.h"

typedef struct opt_solution {
    uint128_t *left;
    uint128_t *right;
    int128_t *conditions;
    struct opt_solution *next;
} opt_solution;

typedef struct subroutine_parameters {
    uint128_t *left;
    uint128_t *right;
    int n;
    int q;
    int current_level;
    uint128_t *snq;
    opt_solution *optimum;
} subroutine_parameters;

int max_level = 0;

int test_repetitions(const uint128_t *left, const uint128_t *right, int current_level) {
    int test_cut = 1;

    for (int i = 0; i < current_level; i++) {
        if (left[i] != right[i]) {
            test_cut = 0;
            break;
        }
    }
    return test_cut;
}

void update_solution(const uint128_t *left, const uint128_t *right, int n, const int128_t *conditions, uint128_t *snq,
                     opt_solution *optimum,
                     uint128_t current_code_size) {
    if (current_code_size > snq[0]) {
        snq[0] = current_code_size;

        opt_solution *next = optimum->next;
        while (next != NULL) {
            free(next->left);
            free(next->right);
            free(next->conditions);
            opt_solution *next_next = next->next;
            free(next);
            next = next_next;
        }

        optimum->next = NULL;
        for (int i = 0; i < n; i++) {
            optimum->left[i] = left[i];
            optimum->right[i] = right[i];
            optimum->conditions[i] = conditions[i];
        }
    } else if (current_code_size == snq[0]) {
        opt_solution *new_optimum = (opt_solution *) malloc(sizeof(opt_solution));
        new_optimum->left = (uint128_t *) malloc(n * sizeof(uint128_t));
        new_optimum->right = (uint128_t *) malloc(n * sizeof(uint128_t));
        new_optimum->conditions = (int128_t *) malloc(n * sizeof(int128_t));
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
}

int fill_second_half(uint128_t *left, uint128_t *right, int n, int current_level, int128_t *conditions, uint128_t *snq,
                     opt_solution *optimum) {
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
    uint128_t current_code_size = compute_max_cardinality(left, right, current_level);
    update_solution(left, right, n, conditions, snq, optimum, current_code_size);

    return 0;
}

int solutions_iterator(uint128_t *left, uint128_t *right, int n, int q, int current_level, int128_t **conditions,
                       int128_t **parameters, uint128_t *snq, opt_solution *optimum) {
    if (current_level == n / 2) {
        fill_second_half(left, right, n, current_level, conditions[0], snq, optimum);
    } else {
        uint128_t min_size = (current_level == 0) ? 1 : 0;
        uint128_t max_size = (current_level == 0) ? (uint128_t) q : compute_max_cardinality(left, right, current_level);
        int test_cut = test_repetitions(left, right, current_level);

        for (uint128_t i = min_size; i <= max_size; i++) {
            if (test_cut && i > max_size / 2) {
                break;
            }

            left[current_level] = i;
            right[current_level] = max_size - i;

            compute_conditions(left, right, conditions, parameters, current_level, n);
            solutions_iterator(left, right, n, q, current_level + 1, conditions, parameters, snq, optimum);
        }
    }
    return 0;
}

int solutions_iterator_caller(uint128_t *left, uint128_t *right, int n, int q, int current_level, uint128_t *snq,
                              opt_solution *optimum) {
    int128_t **conditions = initialize_conditions(n);

    int128_t **parameters = (int128_t **) malloc((n + 1) / 2 * sizeof(int128_t *));

    for (int i = 0; i < (n + 1) / 2; i++) {
        parameters[i] = (int128_t *) malloc((i + 1) * sizeof(int128_t));
        parameters[i][i] = 1;
    }

    for (int i = 0; i < current_level; i++) {
        compute_conditions(left, right, conditions, parameters, i, n);
    }

    solutions_iterator(left, right, n, q, current_level, conditions, parameters, snq, optimum);

    free(conditions[0]);
    free(conditions[1]);
    free(conditions);
    for (int i = 0; i < (n + 1) / 2; i++) {
        free(parameters[i]);
    }
    free(parameters);

    return 0;
}


void *create_threads(void *subroutine_pointer) {
    subroutine_parameters *current_subroutine = (subroutine_parameters *) subroutine_pointer;

    uint128_t *left = current_subroutine->left;
    uint128_t *right = current_subroutine->right;
    int n = current_subroutine->n;
    int q = current_subroutine->q;
    int current_level = current_subroutine->current_level;
    uint128_t *snq = current_subroutine->snq;
    opt_solution *optimum = current_subroutine->optimum;

    if (current_level == max_level || current_level == n / 2) {
        solutions_iterator_caller(left, right, n, q, current_level, snq, optimum);
        return 0;
    }
    uint128_t max_size = (current_level == 0) ? (uint128_t) q : compute_max_cardinality(left, right, current_level);
    uint128_t min_size = (current_level == 0) ? 1 : 0;

    uint128_t subproblems = max_size;
    int test_cut = test_repetitions(left, right, current_level);

    if (test_cut == 1) {
        subproblems /= 2;
    }


    uint128_t **subleft = (uint128_t **) malloc((subproblems + 1 - min_size) * sizeof(uint128_t *));
    uint128_t **subright = (uint128_t **) malloc((subproblems + 1 - min_size) * sizeof(uint128_t *));
    uint128_t *subsnq = (uint128_t *) malloc((subproblems + 1 - min_size) * sizeof(uint128_t));
    opt_solution **suboptimum = (opt_solution **) malloc((subproblems + 1 - min_size) * sizeof(opt_solution *));

    pthread_t *threads = (pthread_t *) malloc((subproblems + 1 - min_size) * sizeof(pthread_t));

    subroutine_parameters **subroutine = (subroutine_parameters **) malloc(
            (subproblems + 1 - min_size) * sizeof(subroutine_parameters *));

    for (uint128_t i = 0; i < (subproblems + 1 - min_size); i++) {
        subleft[i] = (uint128_t *) malloc(n * sizeof(uint128_t));
        subright[i] = (uint128_t *) malloc(n * sizeof(uint128_t));
        suboptimum[i] = (opt_solution *) malloc(sizeof(opt_solution));
        suboptimum[i]->left = (uint128_t *) malloc(n * sizeof(uint128_t));
        suboptimum[i]->right = (uint128_t *) malloc(n * sizeof(uint128_t));
        suboptimum[i]->conditions = (int128_t *) malloc(n * sizeof(int128_t));
        suboptimum[i]->next = NULL;

        for (int j = 0; j < current_level; j++) {
            subleft[i][j] = left[j];
            subright[i][j] = right[j];
        }
        subleft[i][current_level] = i + min_size;
        subright[i][current_level] = max_size - i - min_size;
        subsnq[i] = 0;

        subroutine[i] = (subroutine_parameters *) malloc(sizeof(subroutine_parameters));
        subroutine[i]->left = subleft[i];
        subroutine[i]->right = subright[i];
        subroutine[i]->n = n;
        subroutine[i]->q = q;
        subroutine[i]->current_level = current_level + 1;
        subroutine[i]->snq = &(subsnq[i]);
        subroutine[i]->optimum = suboptimum[i];

        if (pthread_create(&threads[i], NULL, create_threads, subroutine[i]) != 0) {
            perror("pthread_create() error");
            exit(1);
        }
    }
    for (uint128_t i = 0; i < (subproblems + 1 - min_size); i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join() error");
            exit(3);
        }
    }
    free(threads);
    for (uint128_t i = 0; i < (subproblems + 1 - min_size); i++) {
        while (suboptimum[i] != NULL) {
            update_solution(suboptimum[i]->left, suboptimum[i]->right, n, suboptimum[i]->conditions, snq, optimum,
                            subsnq[i]);
            free(suboptimum[i]->left);
            free(suboptimum[i]->right);
            free(suboptimum[i]->conditions);
            opt_solution *next = suboptimum[i]->next;
            free(suboptimum[i]);
            suboptimum[i] = next;
        }

        free(subleft[i]);
        free(subright[i]);
        free(suboptimum[i]);
        free(subroutine[i]);
    }

    free(subleft);
    free(subright);
    free(subsnq);
    free(suboptimum);
    free(subroutine);

    return 0;
}


int main(__attribute__((unused)) int argc, char **argv) {
    if (argc != 4) {
        printf("invalid number of arguments");
        return 1;
    }

    int level = (int) strtol(argv[1], NULL, 10);
    int q = (int) strtol(argv[2], NULL, 10);
    int n = (int) strtol(argv[3], NULL, 10);

    if (level < 0) {
        printf("invalid maxlevel value\n");
        return 2;
    }

    if (q < 2 || n < 3) {
        printf("invalid parameter values\n");
        return 3;
    }

    max_level = level;

    uint128_t snq = 0;

    opt_solution *optimum = (opt_solution *) malloc(sizeof(opt_solution));
    optimum->left = (uint128_t *) malloc(n * sizeof(uint128_t));
    optimum->right = (uint128_t *) malloc(n * sizeof(uint128_t));
    optimum->conditions = (int128_t *) malloc(n * sizeof(int128_t));
    optimum->next = NULL;

    uint128_t *left = (uint128_t *) malloc(n * sizeof(uint128_t));
    uint128_t *right = (uint128_t *) malloc(n * sizeof(uint128_t));

    subroutine_parameters *subroutine = (subroutine_parameters *) malloc(sizeof(subroutine_parameters));
    subroutine->left = left;
    subroutine->right = right;
    subroutine->n = n;
    subroutine->q = q;
    subroutine->current_level = 0;
    subroutine->snq = &snq;
    subroutine->optimum = optimum;

    create_threads(subroutine);

    free(left);
    free(right);
    free(subroutine);

    printf("n: %d, q: %d, S(n,q): ", n, q);
    print_u128_u(snq);
    printf("\n");

    while (optimum != NULL) {
        printf("L: ");
        for (int i = 0; i < n - 1; i++) {
            print_u128_u(optimum->left[i]);
            printf(" ");
        }
        printf("R: ");
        for (int i = 0; i < n - 1; i++) {
            print_u128_u(optimum->right[i]);
            printf(" ");
        }

        printf("; Conditions: ");
        for (int i = n / 2; i < n - 1; i++) {
            print_u128_u(optimum->conditions[i]);
            printf(" ");
        }
        printf("\n");
        free(optimum->left);
        free(optimum->right);
        free(optimum->conditions);
        opt_solution *next = optimum->next;
        free(optimum);
        optimum = next;
    }

    return 0;
}

