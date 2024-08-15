import argparse
import itertools
import multiprocessing
import time
from asyncio import Future
from collections.abc import Generator
from concurrent.futures.process import ProcessPoolExecutor
from dataclasses import dataclass
from textwrap import dedent
from typing import Any, Self

from Bio.SeqUtils import MeltingTemp


@dataclass
class PrecomputationData:
    codeword_length: int
    left_partition_sizes: list[int]
    right_partition_sizes: list[int]
    left: list[list[str]]
    right: list[list[str]]

    @classmethod
    def _generate_pre_suf_fixes(cls, left: list[list[str]], right: list[list[str]], i: int) -> list[str]:
        if i == 0:
            return list("CGTA")

        span_end = i - 1

        result: list[str] = []
        for j in range(i):
            result.extend(f"{left}{right}" for left, right in itertools.product(left[j], right[span_end - j]))

        return result

    @classmethod
    def _generate_partitions(
        cls, pre_suf_fixes: list[str], left_partition_size: int, i: int
    ) -> Generator[Any, tuple[list[str], list[str]], Any]:
        all_indexes = set(range(len(pre_suf_fixes)))

        left_index_variants = [set(items) for items in itertools.combinations(all_indexes, left_partition_size)]
        right_index_variants = [all_indexes - left_variant for left_variant in left_index_variants]

        for left_indices, right_indices in zip(left_index_variants, right_index_variants, strict=True):
            retval_left = [pre_suf_fixes[li] for li in left_indices]
            retval_right = [pre_suf_fixes[ri] for ri in right_indices]
            yield retval_left, retval_right
        return

    @classmethod
    def _generate_small_afixes(
        cls,
        left_partition_sizes: list[int],
        right_partition_sizes: list[int],
        left: list[list[str]],
        right: list[list[str]],
        i: int,
    ) -> None:
        if i >= 2:
            n = len(left_partition_sizes)
            for x in range(right_partition_sizes[n - 1]):
                w = find_codeword(left_partition_sizes, right_partition_sizes, left, right, n - 1, x)
            return

        pre_suf_fixes = cls._generate_pre_suf_fixes(left, right, i)
        for left_addendum, right_addendum in cls._generate_partitions(pre_suf_fixes, left_partition_sizes[i], i):
            left[i] = left_addendum
            right[i] = right_addendum
            cls._generate_small_afixes(left_partition_sizes, right_partition_sizes, left, right, i + 1)
            print("end iteration")

    @classmethod
    def _compute_all(cls, left: list[int], right: list[int], i: int) -> int:
        s = 0
        for j in range(i):
            s += left[j] * right[i - 1 - j]
        return s

    @classmethod
    def precompute(cls, codeword_length: int, *, do_print: bool = False) -> Self:
        # left_partition_sizes: list[int]
        # right_partition_sizes: list[int]
        #    if (codeword_length < 6):
        #        left_partition_sizes = [3, 0, 0, 0]
        #        right_partition_sizes = [1, 3, 9, 27]
        #    elif (codeword_length < 8):
        #        left_partition_sizes = [3, 1, 0, 0]
        #        right_partition_sizes = [1, 2, 7, 23]
        #    elif (codeword_length < 12):
        #        left_partition_sizes = [3, 2, 0, 0]
        #        right_partition_sizes = [1, 1, 5, 17]
        #    elif (codeword_length < 14):
        #        left_partition_sizes = [3, 2, 1, 0]
        #        right_partition_sizes = [1, 1, 4, 15]
        #    elif (codeword_length < 16):
        #        left_partition_sizes = [3, 2, 2, 0]
        #        right_partition_sizes = [1, 1, 3, 13]
        if codeword_length < 23:
            left_partition_sizes = [3, 3, 0, 0]
            right_partition_sizes = [1, 0, 3, 9]
        else:
            left_partition_sizes = [3, 3, 0, 1]
            right_partition_sizes = [1, 0, 3, 8]

        for i in range(4, codeword_length):
            left_partition_sizes.append(0)
            s = cls._compute_all(left_partition_sizes, right_partition_sizes, i)
            right_partition_sizes.append(s)
            if do_print:
                print(s)
        if do_print:
            print(len(left_partition_sizes))

        left = [[] for _ in range(codeword_length)]
        right = [[] for _ in range(codeword_length)]

        left[0] = ["C", "G", "A"]
        right[0] = ["T"]
        left[1] = ["CT", "GT", "AT"]
        return cls(
            codeword_length=codeword_length,
            left_partition_sizes=left_partition_sizes,
            right_partition_sizes=right_partition_sizes,
            left=left,
            right=right,
        )


def find_codeword(
    left_partition_sizes: list[int],
    right_partition_sizes: list[int],
    left: list[list[str]],
    right: list[list[str]],
    i: int,
    k: int,
) -> str:
    if i < 2:
        return right[i][k]

    s_old = 0
    j = 0
    s_new = left_partition_sizes[0] * right_partition_sizes[i - 1]

    while s_new <= k:
        s_old = s_new
        j += 1
        s_new += left_partition_sizes[j] * right_partition_sizes[i - 1 - j]
    new_k = (k - s_old) // left_partition_sizes[j]

    return left[j][(k - s_old) % left_partition_sizes[j]] + find_codeword(
        left_partition_sizes, right_partition_sizes, left, right, i - 1 - j, new_k
    )


def chunked_range(range_min: int, range_max: int, num_chunks: int) -> list[range]:
    # exactness not important because range_max handles overflows
    # but having one slightly smaller chunk is better than one very small chunk by accident
    step = int((range_max - range_min) / num_chunks) + 1

    result = []
    for start in range(range_min, range_max, step):
        result.append(range(start, min(start + step, range_max)))
    return result


# returns increment of adequate_melting
def batched_parallel_unit_of_work(codeword_length: int, unit_of_work_offsets: range) -> int:
    # every process precomputes this as it takes negligible time
    data = PrecomputationData.precompute(codeword_length)

    result = 0
    for uow_offset in unit_of_work_offsets:
        w = find_codeword(
            data.left_partition_sizes,
            data.right_partition_sizes,
            data.left,
            data.right,
            codeword_length - 1,
            uow_offset,
        )
        temperature = MeltingTemp.Tm_NN(w)
        if 60.0 >= temperature >= 55.0:
            result += 1
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--codeword-length", type=int, required=True)
    parser.add_argument("--node-index", type=int, required=False, default=0)
    parser.add_argument("--node-count", type=int, required=False, default=1)
    args = parser.parse_args()

    if args.codeword_length < 10:
        print("Argument too small.")
        exit(1)
    if args.codeword_length > 30:
        print("Argument too big.")
        exit(2)

    process_count = multiprocessing.cpu_count()
    data = PrecomputationData.precompute(args.codeword_length)

    per_node_chunks = chunked_range(0, data.right_partition_sizes[data.codeword_length - 1], num_chunks=args.node_count)
    my_node_chunk = per_node_chunks[args.node_index]
    per_process_chunks = chunked_range(my_node_chunk.start, my_node_chunk.stop, num_chunks=process_count)

    print(
        dedent(
            f"""
        parallelization params
            codeword length: {data.codeword_length}
            process count: {process_count}
            node index {args.node_index} of {args.node_count} nodes
            per-node chunk size: {len(my_node_chunk)}
            per-process chunk size: {len(per_process_chunks[0])}
        """
        )
    )

    # generate_small_afixes(left_partition_sizes,right_partition_sizes,left,right,2)
    pool = ProcessPoolExecutor(max_workers=process_count)

    time_start = time.time()
    futures: list[Future] = []
    for chunk in per_process_chunks:
        futures.append(pool.submit(batched_parallel_unit_of_work, data.codeword_length, chunk))
    num_of_adequate_melting_temps = sum(f.result() for f in futures)
    time_end = time.time()

    print(
        f"### RESULT ### num of adequate melting temps (node {args.node_index} of {args.node_count}): {num_of_adequate_melting_temps}"
    )
    print(f"time taken (node {args.node_index} of {args.node_count}): {int(time_end - time_start)} seconds")


if __name__ == "__main__":
    main()
