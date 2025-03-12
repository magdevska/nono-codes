import argparse
import multiprocessing
import time
from asyncio import Future
from concurrent.futures.process import ProcessPoolExecutor
from dataclasses import dataclass
from textwrap import dedent
from typing import Any, Self
import itertools

from Bio.SeqUtils import MeltingTemp


@dataclass
class PrecomputationData:
    codeword_length: int
    left_partition_sizes: list[int]
    right_partition_sizes: list[int]
    left: list[list[str]]
    right: list[list[str]]
    storage_depth: int
    partition_choice: int

    @classmethod
    def _compute_all(cls, left: list[int], right: list[int], i: int) -> int:
        s = 0
        for j in range(i):
            s += left[j] * right[i - 1 - j]
        return s

    @classmethod
    def generate_pre_suf_fixes(cls, left: list[list[str]], right: list[list[str]], i: int) -> list[str]:
        span_end = i - 1

        result: list[str] = []
        for j in range(i):
            result.extend(f"{left}{right}" for left, right in itertools.product(left[j], right[span_end - j]))

        return result

    @classmethod
    def precompute(cls, codeword_length: int, partition_choice: int, *, do_print: bool = False) -> Self:
        if codeword_length < 23:
            left_partition_sizes = [3, 3]
            right_partition_sizes = [1, 0]
            storage_depth = 2

            if partition_choice <= 0 or partition_choice > 3:
                left = [["C", "G", "A"], ["CT", "GT", "AT"]]
                right = [["T"], []]
            elif partition_choice == 1:
                left = [["C", "G", "T"], ["CA", "GA", "TA"]]
                right = [["A"], []]
            elif partition_choice == 2:
                left = [["C", "T", "A"], ["CG", "TG", "AG"]]
                right = [["G"], []]
            else:
                left = [["G", "T", "A"], ["GC", "TC", "AC"]]
                right = [["C"], []]
        else:
            left_partition_sizes = [3, 3, 0, 1]
            right_partition_sizes = [1, 0, 3, 8]
            storage_depth = 4
            if partition_choice < 9 or partition_choice > 35:
                left = [["A", "C", "G"], ["CT", "GT", "AT"], [], []]
                right = [["T"], [], ["ATT", "CTT", "GTT"]]
            elif partition_choice < 18:
                left = [["C", "G", "T"], ["CA", "GA", "TA"], [], []]
                right = [["A"], [], ["CAA", "GAA", "TAA"]]
            elif partition_choice < 27:
                left = [["C", "T", "A"], ["CG", "TG", "AG"], [], []]
                right = [["G"], [], ["CGG", "TGG", "AGG"]]
            else:
                left = [["G", "T", "A"], ["GC", "TC", "AC"], [], []]
                right = [["C"], [], ["GCC", "TCC", "ACC"]]

            words = cls.generate_pre_suf_fixes(left, right, 3)
            left[3] = words[partition_choice % 9]
            right.append(words[0 : (partition_choice % 9)] + words[partition_choice % 9 + 1 :])

        for i in range(storage_depth, codeword_length):
            left_partition_sizes.append(0)
            s = cls._compute_all(left_partition_sizes, right_partition_sizes, i)
            right_partition_sizes.append(s)
            if do_print:
                print(s)
        if do_print:
            print(len(left_partition_sizes))

        return cls(
            codeword_length=codeword_length,
            left_partition_sizes=left_partition_sizes,
            right_partition_sizes=right_partition_sizes,
            left=left,
            right=right,
            storage_depth=storage_depth,
            partition_choice=partition_choice,
        )


def find_codeword(
    left_partition_sizes: list[int],
    right_partition_sizes: list[int],
    left: list[list[str]],
    right: list[list[str]],
    i: int,
    k: int,
    storage_depth: int,
) -> str:
    if i < storage_depth:
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
        left_partition_sizes, right_partition_sizes, left, right, i - 1 - j, new_k, storage_depth
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
def batched_parallel_unit_of_work(
    codeword_length: int, partition_choice: int, unit_of_work_offsets: range
) -> list[int]:
    # every process precomputes this as it takes negligible time
    data = PrecomputationData.precompute(codeword_length, partition_choice)

    result = [0, 0, 0, 0, 0, 0, 0, 0]
    for uow_offset in unit_of_work_offsets:
        w = find_codeword(
            data.left_partition_sizes,
            data.right_partition_sizes,
            data.left,
            data.right,
            codeword_length - 1,
            uow_offset,
            data.storage_depth,
        )
        temperature = MeltingTemp.Tm_NN(w, dnac2=0)
        if 50 <= temperature <= 52:
            result[0] += 1
        elif 52 < temperature <= 54:
            result[1] += 1
        elif 54 < temperature <= 56:
            result[2] += 1
        elif 56 < temperature <= 58:
            result[3] += 1
        elif 58 < temperature <= 60:
            result[4] += 1
        elif 60 < temperature <= 62:
            result[5] += 1
        elif 62 < temperature and codeword_length > 16:
            prefix_temperature = MeltingTemp.Tm_NN(w[0:16], dnac2=0)
            if prefix_temperature < 62:
                result[6] += 1
            result[7] += 1
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--codeword-length", type=int, required=True)
    parser.add_argument("--node-index", type=int, required=False, default=0)
    parser.add_argument("--node-count", type=int, required=False, default=1)
    parser.add_argument("--partition-choice", type=int, required=False, default=0)
    args = parser.parse_args()

    if args.codeword_length < 10:
        print("Argument too small.")
        exit(1)
    if args.codeword_length > 30:
        print("Argument too big.")
        exit(2)

    process_count = multiprocessing.cpu_count()
    data = PrecomputationData.precompute(args.codeword_length, args.partition_choice)

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

    pool = ProcessPoolExecutor(max_workers=process_count)

    time_start = time.time()
    futures: list[Future] = []
    for chunk in per_process_chunks:
        futures.append(pool.submit(batched_parallel_unit_of_work, data.codeword_length, data.partition_choice, chunk))
    num_of_adequate_melting_temps = [sum(f.result()[i] for f in futures) for i in range(8)]
    time_end = time.time()

    print(
        f"### RESULT ### num of adequate melting temps per range (node {args.node_index} of {args.node_count}): {num_of_adequate_melting_temps !r} \n"
        f"total number of adequate melting temps in partition # {args.partition_choice}: {sum(num_of_adequate_melting_temps[0:6])}"
    )
    print(f"time taken (node {args.node_index} of {args.node_count}): {int(time_end - time_start)} seconds")


if __name__ == "__main__":
    main()
