#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

typedef struct opt_solution {
    uint64_t **left;
    uint64_t **right;
    struct opt_solution *next;
} opt_solution;

typedef struct subroutine_parameters {
    uint64_t **left;
    uint64_t **right;
    int n;
    int q;
    int current_level;
    int positive_letters;
    uint64_t *bnq;
    opt_solution *optimum;
} subroutine_parameters;

int max_level = 2;

void update_solution(uint64_t **left, uint64_t **right, int n,
                     uint64_t current_code_size, opt_solution *optimum, uint64_t *bnq) {
    if (current_code_size > bnq[0]) {
        bnq[0] = current_code_size;

        opt_solution *next = optimum->next;
        while (next != NULL) {
            for (int i = 1; i < n; i++) {
                free(next->left[i]);
                free(next->right[i]);
            }
            free(next->left);
            free(next->right);
            opt_solution *next_next = next->next;
            free(next);
            next = next_next;
        }

        optimum->next = NULL;
        for (int i = 1; i < n; i++) {
            for (int j = 0; j <= n / 2; j++) {
                optimum->left[i][j] = left[i][j];
                optimum->right[i][j] = right[i][j];
            }
        }
    } else if (current_code_size == bnq[0]) {
        opt_solution *new_optimum = (opt_solution *) malloc(sizeof(opt_solution));
        new_optimum->left = (uint64_t **) malloc(n * sizeof(uint64_t *));
        new_optimum->right = (uint64_t **) malloc(n * sizeof(uint64_t *));
        new_optimum->next = NULL;

        for (int i = 1; i < n; i++) {
            new_optimum->left[i] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
            new_optimum->right[i] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
            for (int j = 0; j <= n / 2; j++) {
                new_optimum->left[i][j] = left[i][j];
                new_optimum->right[i][j] = right[i][j];
            }
        }

        opt_solution *next = optimum;
        while (next->next != NULL) {
            next = next->next;
        }
        next->next = new_optimum;
    }
}

uint64_t maximum_partition_size(uint64_t **left, uint64_t **right, int i, int j, int q) {
    if (i == 1) {
        return q / 2;
    }
    uint64_t s = 0;
    for (int k = 1; k < i; k++) {
        for (int l = 0; l <= j; l++) {
            s += left[k][l] * right[i - k][j - l];
        }
    }
    return s;
}

void butlbrsk(uint64_t **left, uint64_t **right, int i, int j, int q, int n, opt_solution *optimum, uint64_t *bnq) {
    if (i == n) {
        uint64_t code_size = maximum_partition_size(left, right, i, n / 2, q);
        update_solution(left, right, n, code_size, optimum, bnq);
        return;
    }
    if (j > i || j > n / 2) {
        butlbrsk(left, right, i + 1, 0, q, n, optimum, bnq);
        return;
    }
    if (j < i - n / 2) {
        butlbrsk(left, right, i, i - n / 2, q, n, optimum, bnq);
        return;
    }

    uint64_t max_size = maximum_partition_size(left, right, i, j, q);
    if (i <= n / 2) {
        for (uint64_t s = 0; s <= max_size; s++) {
            left[i][j] = s;
            right[i][j] = max_size - s;
            butlbrsk(left, right, i, j + 1, q, n, optimum, bnq);
        }
    } else {
        left[i][j] = 0;
        right[i][j] = max_size;
        butlbrsk(left, right, i, j + 1, q, n, optimum, bnq);
        if (max_size > 0) {
            left[i][j] = max_size;
            right[i][j] = 0;
            butlbrsk(left, right, i, j + 1, q, n, optimum, bnq);
        }
    }
}

void *create_threads(void *subroutine_pointer) {
    subroutine_parameters *current_subroutine = (subroutine_parameters *) subroutine_pointer;

    uint64_t **left = current_subroutine->left;
    uint64_t **right = current_subroutine->right;
    int n = current_subroutine->n;
    int q = current_subroutine->q;
    int current_level = current_subroutine->current_level;
    int positive_letters = current_subroutine->positive_letters;
    uint64_t *bnq = current_subroutine->bnq;
    opt_solution *optimum = current_subroutine->optimum;

    if (current_level == max_level || current_level == n / 2) {
        butlbrsk(left, right, current_level, positive_letters, q, n, optimum, bnq);
        return NULL;
    }
    if (positive_letters > current_level || positive_letters > n / 2) {
        positive_letters = 0;
        current_level += 1;
    }
    if (positive_letters < current_level - n / 2) {
        positive_letters = current_level - n / 2;
    }


    uint64_t max_size = maximum_partition_size(left, right, current_level, positive_letters, q);

    uint64_t subproblems = max_size;

    uint64_t ***subleft = (uint64_t ***) malloc((subproblems + 1) * sizeof(uint64_t **));
    uint64_t ***subright = (uint64_t ***) malloc((subproblems + 1) * sizeof(uint64_t **));
    uint64_t *subbnq = (uint64_t *) malloc((subproblems + 1) * sizeof(uint64_t));
    opt_solution **suboptimum = (opt_solution **) malloc((subproblems + 1) * sizeof(opt_solution *));

    pthread_t *threads = (pthread_t *) malloc((subproblems + 1) * sizeof(pthread_t));

    subroutine_parameters **subroutine = (subroutine_parameters **) malloc(
            (subproblems + 1) * sizeof(subroutine_parameters *));

    for (uint64_t i = 0; i < (subproblems + 1); i++) {
        subleft[i] = (uint64_t **) malloc(n * sizeof(uint64_t *));
        subright[i] = (uint64_t **) malloc(n * sizeof(uint64_t *));
        suboptimum[i] = (opt_solution *) malloc(sizeof(opt_solution));
        suboptimum[i]->left = (uint64_t **) malloc(n * sizeof(uint64_t *));
        suboptimum[i]->right = (uint64_t **) malloc(n * sizeof(uint64_t *));

        suboptimum[i]->next = NULL;

        for (int j = 1; j < n; j++) {
            subleft[i][j] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
            subright[i][j] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
            suboptimum[i]->left[j] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
            suboptimum[i]->right[j] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
        }

        for (int j = 1; j < current_level; j++) {
            for (int k = 0; k <= n / 2; k++) {
                subleft[i][j][k] = left[j][k];
                subright[i][j][k] = right[j][k];
            }
        }
        for (int k = 0; k < positive_letters; k++) {
            subleft[i][current_level][k] = left[current_level][k];
            subright[i][current_level][k] = right[current_level][k];
        }
        subleft[i][current_level][positive_letters] = i;
        subright[i][current_level][positive_letters] = max_size - i;
        subbnq[i] = 0;

        subroutine[i] = (subroutine_parameters *) malloc(sizeof(subroutine_parameters));
        subroutine[i]->left = subleft[i];
        subroutine[i]->right = subright[i];
        subroutine[i]->n = n;
        subroutine[i]->q = q;
        subroutine[i]->current_level = current_level;
        subroutine[i]->positive_letters = positive_letters + 1;
        subroutine[i]->bnq = &(subbnq[i]);
        subroutine[i]->optimum = suboptimum[i];

        if (pthread_create(&threads[i], NULL, create_threads, subroutine[i]) != 0) {
            perror("pthread_create() error");
            exit(1);
        }
    }
    for (uint64_t i = 0; i < (subproblems + 1); i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join() error");
            exit(3);
        }
    }
    free(threads);
    for (uint64_t i = 0; i < (subproblems + 1); i++) {
        while (suboptimum[i] != NULL) {
            update_solution(suboptimum[i]->left, suboptimum[i]->right, n, subbnq[i], optimum, bnq);

            for (int j = 1; j < n; j++) {
                free(suboptimum[i]->left[j]);
                free(suboptimum[i]->right[j]);
            }
            free(suboptimum[i]->left);
            free(suboptimum[i]->right);
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
    free(subbnq);
    free(suboptimum);
    free(subroutine);

    return NULL;
}

int main(__attribute__((unused)) int argc, char **argv) {
    int level = (int) strtol(argv[1], NULL, 10);
    int q = (int) strtol(argv[2], NULL, 10);
    int n = (int) strtol(argv[3], NULL, 10);

    if (q < 2 || n < 3 || n % 2 == 1) {
        printf("invalid parameter values\n");
        return 0;
    }
    max_level = level;

    uint64_t **left = (uint64_t **) malloc(n * sizeof(uint64_t *));
    uint64_t **right = (uint64_t **) malloc(n * sizeof(uint64_t *));
    opt_solution *optimum = (opt_solution *) malloc(sizeof(opt_solution));
    optimum->left = (uint64_t **) malloc(n * sizeof(uint64_t *));
    optimum->right = (uint64_t **) malloc(n * sizeof(uint64_t *));
    optimum->next = NULL;

    for (int i = 1; i < n; i++) {
        left[i] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
        right[i] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
        optimum->left[i] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
        optimum->right[i] = (uint64_t *) malloc((n / 2 + 1) * sizeof(uint64_t));
        for (int j = 0; j <= n / 2; j++) {
            left[i][j] = 0;
            right[i][j] = 0;
        }
    }

    uint64_t bnq = 0;

    subroutine_parameters *subroutine = (subroutine_parameters *) malloc(sizeof(subroutine_parameters));
    subroutine->left = left;
    subroutine->right = right;
    subroutine->n = n;
    subroutine->q = q;
    subroutine->current_level = 1;
    subroutine->positive_letters = 0;
    subroutine->bnq = &bnq;
    subroutine->optimum = optimum;


    create_threads(subroutine);

    free(subroutine);

    int number_of_solutions = 0;
    while (optimum != NULL) {
        printf("L:");
        for (int i = 1; i < n; i++) {
            for (int j = 0; j <= n / 2; j++) {
                printf(" %ld", optimum->left[i][j]);
            }
            printf("\n");
            free(optimum->left[i]);
        }
        free(optimum->left);
        printf("R:");
        for (int i = 1; i < n; i++) {
            for (int j = 0; j <= n / 2; j++) {
                printf(" %ld", optimum->right[i][j]);
            }
            printf("\n");
            free(optimum->right[i]);
        }
        free(optimum->right);
        printf("end of solution\n");
        opt_solution *next = optimum->next;
        free(optimum);
        optimum = next;
        number_of_solutions += 1;
    }
    printf("q: %d n: %d code size: %ld\n", q, n, bnq);
    printf("number of solutions: %d\n", number_of_solutions);

    for (int i = 1; i < n; i++) {
        free(left[i]);
        free(right[i]);
    }

    free(left);
    free(right);

    return 0;
}
