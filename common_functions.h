#ifndef NONO_CODES_COMMON_FUNCTIONS_H
#define NONO_CODES_COMMON_FUNCTIONS_H

#include <stdint.h>
#include <inttypes.h>


__extension__ typedef unsigned __int128 uint128_t;
__extension__ typedef __int128 int128_t;

uint128_t compute_max_cardinality(const uint128_t *left, const uint128_t *right, int set_index);

int compute_conditions(const uint128_t *left, const uint128_t *right, int128_t **conditions,
                       int128_t **parameters, int index, int n);

int128_t **initialize_conditions(int n);



/*      UINT64_MAX 18446744073709551615ULL */
#define P10_UINT64 10000000000000000000ULL   /* 19 zeroes */
#define E10_UINT64 19

#define STRINGIZER(x)   # x
#define TO_STRING(x)    STRINGIZER(x)

int print_u128_u(uint128_t u128);


#endif //NONO_CODES_COMMON_FUNCTIONS_H
