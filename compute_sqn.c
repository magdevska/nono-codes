#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

static uint64_t snq;

uint64_t compute_max_cardinality(uint64_t* left, uint64_t* right, int set_index) {
    uint64_t size = 0;
    for (int i = 0; i < set_index; i++) {
        size += left[i] * right[set_index - 1 - i];
    }
    return size;
}

int compute_conditions(uint64_t* left, uint64_t* right, int64_t** conditions, int64_t** parameters, int index, int n) {
    for (int j = 0; j < index; j++) {
        for (int k = j - 1; k > - 1; k--) {
            parameters[j][k] = 0;
            for (int l = k; l < j; l++) {
                parameters[j][k] += (conditions[1][n-index+l - 2] * right[j-l - 1] + (1 - conditions[1][n-index+l - 2]) * left[j-l -1 ]) * parameters[l][k];
            }
        }
    }
    
    conditions[0][n - index - 2] = right[index] - left[index];
    for (int j = 0; j < index; j++) {
        int value = 0;
        for (int l = 0; l <= j; l++) {
            value += parameters[j][l] * (right[l] - left[l]);
        }
        value *= (conditions[1][n - index + j - 2] * right[index - j - 1] + (1 - conditions[1][n - index + j - 2]) * left[index - j - 1]);
        conditions[0][n - index - 2] += value;
    }
    
    if (conditions[0][n - index - 2] < 0) {
        conditions[1][n - index - 2] = 0;
    }
    else {
        conditions[1][n - index - 2] = 1;
    }
    
    return 0;
}

int64_t **initialize_conditions(int n) {
    int64_t** conditions = (int64_t**) malloc(2 * sizeof(int64_t *));
    conditions[0] = (int64_t*) malloc(n * sizeof(int64_t));
    conditions[1] = (int64_t*) malloc(n * sizeof(int64_t));
    
    return conditions;
}

int solutions_iterator(uint64_t* left, uint64_t* right, int n, int q, int current_level, int64_t **conditions, int64_t **parameters) {
    if (current_level == 0) {
        for (int i = 1; i <= q/2; i++) {
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
    
    if (current_level > n/2 && current_level < n - 1) {
        if (conditions[1][current_level] == 1) {
            left[current_level] = current_code_size;
            right[current_level] = 0;
        } else {
            left[current_level] = 0;
            right[current_level] = current_code_size;
        }
        solutions_iterator(left, right, n, q, current_level + 1, conditions, parameters);
    }
    else {
        for (int i = 0; i <= current_code_size; i++) {
            left[current_level] = i;
            right[current_level] = current_code_size - i;
            
            compute_conditions(left, right, conditions, parameters, current_level, n);            
            solutions_iterator(left, right, n, q, current_level + 1, conditions, parameters);
        }
    } 
    return 0;
}

int solutions_iterator_caller(uint64_t* left, uint64_t* right, int n, int q) {
    int64_t **conditions = initialize_conditions(n);
    
    int64_t** parameters = (int64_t**) malloc((n + 1)/2 * sizeof(int64_t *));
    
    for (int i = 0; i < (n + 1)/2; i++) {
        parameters[i] = (int64_t*) malloc((i + 1) * sizeof(int64_t));
        parameters[i][i] = 1;
    }
    
    solutions_iterator(left, right, n, q, 0, conditions, parameters);
    
    free(conditions[0]);
    free(conditions[1]);
    free(conditions);
    for (int i = 0; i < (n + 1)/2; i++) {
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
    
    snq = 0;
    
    uint64_t* left = (uint64_t*) malloc(n * sizeof(uint64_t));
    uint64_t* right = (uint64_t*) malloc(n * sizeof(uint64_t));
    solutions_iterator_caller(left, right, n, q);
    
    free(left);
    free(right);

    printf("n: %d, q: %d, S(n,q): %ld\n", n, q, snq);
    return 0;
}

