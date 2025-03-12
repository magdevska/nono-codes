import sys


# helper function
def recursive_levy_call(word, n, k, i, n1, count, last_run):
    if last_run == k:
        return count
    if i == n - 1:
        if n1 == n / 2 - 2:
            count += 1
        return count
    if n1 < n / 2 - 2:
        word[i] = "1"
        count = recursive_levy_call(word, n, k, i + 1, n1 + 1, count, 0)
    word[i] = "0"
    return recursive_levy_call(word, n, k, i + 1, n1, count, last_run + 1)

# counts the number of words in the Construction of Levy and Yaakobi (2018) by constructing the codewords
def construction_levy(n, k):
    if k > n / 2:
        raise ValueError("Invalid parameter choice, k > n / 2.")
    word = [None] * n
    for i in range(k):
        word[i] = "0"
    word[k] = "1"
    word[n - 1] = "1"
    return recursive_levy_call(word, n, k, k + 1, 0, 0, 0)


# helper function
def restricted_levy_recursive_call(word, n, k, i, n1, count, last_run, l):
    if word[i - 1] == 0 and last_run == k:
        return count
    if word[i - 1] == 1 and last_run >= l:
        return count
    if i == n - 1:
        if n1 == n / 2:
            count += 1
        return count
    old_last_run = last_run
    if n1 < n / 2:
        word[i] = 1
        if word[i - 1] == 1:
            last_run += 1
        else:
            last_run = 1
        count = restricted_levy_recursive_call(word, n, k, i + 1, n1 + 1, count, last_run, l)
    word[i] = 0
    if word[i - 1] == 0:
        last_run = old_last_run + 1
    else:
        last_run = 1
    return restricted_levy_recursive_call(word, n, k, i + 1, n1, count, last_run, l)


# counts the number of words in the Construction of Levy and Yaakobi (2018) with run-lengths of at most l by constructing the codewords
def restricted_levy(n, k, l):
    if l <= k:
        raise ValueError("Invalid parameter choice, l <= k.")
    if k > n / 2:
        raise ValueError("Invalid parameter choice, k > n / 2.")
    if l > n / 2 - 2:
        construction_levy(n, k)
        return
    word = [None] * n
    for i in range(k):
        word[i] = 0
    word[k] = 1
    word[n - 1] = 1
    return restricted_levy_recursive_call(word, n, k, k + 1, 2, 0, 1, l)


if __name__ == "__main__":
    print(construction_levy(int(sys.argv[1]), int(sys.argv[2])))
    print(restricted_levy(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])))

