import itertools
from collections.abc import Generator
from typing import Any


def generate_pre_suf_fixes(left: list[list[str]], right: list[list[str]], i: int) -> list[str]:
    if i == 0:
        return list("CGTA")

    span_end = i - 1

    result: list[str] = []
    for j in range(i):
        result.extend(f"{left}{right}" for left, right in itertools.product(left[j], right[span_end - j]))

    return result


def generate_partitions(
        pre_suf_fixes: list[str], left_partition_size: int
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
) -> list[list[str]]:
    pre_suf_fixes = generate_pre_suf_fixes(left, right, i)

    if i >= len(left):
        return [pre_suf_fixes]

    result: list[list[str]] = []
    for left_addendum, right_addendum in generate_partitions(pre_suf_fixes, left_partition_size[i]):
        left[i] = left_addendum
        right[i] = right_addendum
        result.extend(find_addresses(left, right, left_partition_size, i + 1))
    return result


def main() -> None:
    left_partition_sizes = [3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    n = len(left_partition_sizes)

    left: list[list[str]] = [[] for _ in range(n)]
    right: list[list[str]] = [[] for _ in range(n)]

    solutions = find_addresses(left, right, left_partition_sizes, 0)
    for solution in solutions:
        print("solution:", " ".join(solution))
    print("done")


if __name__ == "__main__":
    main()
