#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common_functions.h"

static uint64_t snq;

int64_t compute_conditions_lowerbound(uint64_t* left, uint64_t* right, int64_t** parameters, int index, int n) {
    for (int j = 0; j < index; j++) {
        for (int k = j - 1; k > - 1; k--) {
            parameters[j][k] = 0;
            for (int l = k; l < j; l++) {
                parameters[j][k] += right[j-l - 1] * parameters[l][k];
            }
        }
    }
    
    int64_t conditions = compute_max_cardinality(left, right, index);
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

int lower_bound(uint64_t* left, uint64_t* right, int n, int q, int current_level, int64_t **parameters) {
    if (current_level == 0) {  
        int64_t** parameters = (int64_t**) malloc(n * sizeof(int64_t *));
    
        for (int i = 0; i < n; i++) {
            parameters[i] = (int64_t*) malloc((i + 1) * sizeof(int64_t));
            parameters[i][i] = 1;
        }
        
        for (int i = 1; i <= q/2; i++) {
            left[0] = i;
            right[0] = q - i;
            lower_bound(left, right, n, q, 1, parameters);
        }
        for (int i = 0; i < n; i++) {
            free(parameters[i]);
        }
        free(parameters);
        return 0;
    }

    uint64_t current_code_size = compute_max_cardinality(left, right, current_level);
    if (current_level == n - 1) {    
        if (current_code_size > snq) {
            snq = current_code_size;
        }
        return 0;
    }

    int n_bound = (int) (n / (q + 1)) + 1;
    if (current_level <= n_bound) {
        int64_t cond = compute_conditions_lowerbound(left, right, parameters, current_level, n);
        for (int i = 0; 2*i <= cond && i <= current_code_size; i++) {
            left[current_level] = i;
            right[current_level] = current_code_size - i;
            lower_bound(left, right, n, q, current_level + 1, parameters);
        }
    }
    else {
        left[current_level] = current_code_size;
        right[current_level] = 0;
        lower_bound(left, right, n, q, current_level + 1, parameters);
    }
    return 0;
}

int main(int argc, char **argv) {
    int q = (int) strtol(argv[1], NULL, 10);
    int n = (int) strtol(argv[2], NULL, 10);
    
    if (q < 2 || n < 3) {
        printf("invalid parameter values\n");
        return 0;
    }
    
    uint64_t* left = (uint64_t*) malloc(n * sizeof(uint64_t));
    uint64_t* right = (uint64_t*) malloc(n * sizeof(uint64_t));
    
    lower_bound(left, right, n, q, 0, NULL);
        
    free(left);
    free(right);
    
    printf("n: %d, q: %d, S(n,q): %ld\n", n, q, snq);
    return 0;
}

