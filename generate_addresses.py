import itertools
from collections.abc import Generator
from typing import Any
import sys
from Bio.SeqUtils import gc_fraction, MeltingTemp


def generate_pre_suf_fixes(left: list[list[str]], right: list[list[str]], i: int) -> list[str]:
    if i == 0:
        return list("CGTA")

    span_end = i - 1

    result: list[str] = []
    for j in range(i):
        result.extend(f"{left}{right}" for left, right in itertools.product(left[j], right[span_end - j]))
    
    return result


def generate_partitions(
        pre_suf_fixes: list[str], left_partition_size: int, i: int
) -> Generator[Any, tuple[list[str], list[str]], Any]:
    all_indexes = set(range(len(pre_suf_fixes)))
    
    left_index_variants = [set(items) for items in itertools.combinations(all_indexes, left_partition_size)]
    right_index_variants = [all_indexes - left_variant for left_variant in left_index_variants]

    for left_indices, right_indices in zip(left_index_variants, right_index_variants, strict=True):
        retval_left = [pre_suf_fixes[li] for li in left_indices]
        retval_right = [pre_suf_fixes[ri] for ri in right_indices]
        yield retval_left, retval_right
    return


def find_addresses(
        left: list[list[str]], right: list[list[str]], left_partition_size: list[int], i: int
) -> None: #list[list[str]]:
    pre_suf_fixes = generate_pre_suf_fixes(left, right, i)

    if i >= len(left):
        print("solution:", )
        for j in range(3):
            print("L[",j,"]", ", ".join(left[j]))
        
        valid_addresses_gc = 0
        valid_addresses_t = 0
        valid_addresses = 0
        
        valid_addresses_homopolymer = 0
        for address in pre_suf_fixes:
            fraction = gc_fraction(address)
            temperature = MeltingTemp.Tm_NN(address)
#            temperature = MeltingTemp.Tm_GC(address)
            if fraction <= 0.6 and fraction >= 0.4:
                valid_addresses_gc += 1           
            
            if temperature <= 65.0 and temperature >= 30.0:
                valid_addresses_t += 1
                
            homopolymer_run = (max(len(list(v)) for k, v in itertools.groupby(address)))
            if homopolymer_run <= 5:
                valid_addresses_homopolymer += 1
                
            if fraction <= 0.6 and fraction >= 0.4 and temperature <= 65.0 and temperature >= 30.0 and homopolymer_run <= 5:
                valid_addresses += 1
            
        print("gc-fraction:", valid_addresses_gc, valid_addresses_gc / len(pre_suf_fixes))
        print("melting temperature:", valid_addresses_t, valid_addresses_t / len(pre_suf_fixes))
        print("all", valid_addresses, valid_addresses / len(pre_suf_fixes))
        print("homopolymer run", valid_addresses_homopolymer, valid_addresses_homopolymer / len(pre_suf_fixes))
        return #[pre_suf_fixes]

    #result: list[list[str]] = []
    for left_addendum, right_addendum in generate_partitions(pre_suf_fixes, left_partition_size[i], i):
        left[i] = left_addendum
        right[i] = right_addendum
        
#        solutions = 
        find_addresses(left, right, left_partition_size, i+1)
#        for solution in solutions:
#            print("solution:", " ".join(solution))
#        result.extend(find_addresses(left, right, left_partition_size, i + 1))
    return #result


def main(n: int) -> None:
#    left_partition_sizes = [3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   
#    left_partition_sizes_from_3_to_5 = [3, 0, 0]
#    left_partition_sizes_from_6_to_7 = [3, 1, 0]
#    left_partition_sizes_from_8_to_11 = [3, 2, 0]
#    left_partition_sizes_from_12_to_13 = [3, 2, 1]
#    left_partition_sizes_from_14_to_15 = [3, 2, 2]
#    left_partition_sizes_from_16_to_16 = [3, 3, 0]
    
#    n = len(left_partition_sizes)

    if (n < 3):
        print("Argument too small.")
        return
    if (n > 16):
        print("Argument too big.")
        return
    left_partition_sizes: list[int]
    if (n < 6):
        left_partition_sizes = [3, 0, 0]
    elif (n < 8):
        left_partition_sizes = [3, 1, 0]
    elif (n < 12):
        left_partition_sizes = [3, 2, 0]
    elif (n < 14):
        left_partition_sizes = [3, 2, 1]
    elif (n < 16):
        left_partition_sizes = [3, 2, 2]
    elif (n < 23):
        left_partition_sizes = [3, 3, 0]
    else:
    	left_partition_sizes = [3, 3, 0, 1]
    
    for i in range(3,n):
        left_partition_sizes.append(0)
    print(len(left_partition_sizes))

    left: list[list[str]] = [[] for _ in range(n)]
    right: list[list[str]] = [[] for _ in range(n)]

    solutions = find_addresses(left, right, left_partition_sizes, 0)
#    for solution in solutions:
#        print("solution:", " ".join(solution))
    print("done")


if __name__ == "__main__":
    main(int(sys.argv[1]))

