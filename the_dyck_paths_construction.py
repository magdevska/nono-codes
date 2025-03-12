import sys


def generate_dyck_word(word, n, n0, n1, i, code):
    if i == 2 * n:
        code.append("".join(word))
        return
    if n0 < n and n1 < n:
        word[i] = "0"
        generate_dyck_word(word, n, n0 + 1, n1, i + 1, code)
        if n0 > n1:
            word[i] = "1"
            generate_dyck_word(word, n, n0, n1 + 1, i + 1, code)
        return
    if n0 < n and n1 == n:
        word[i] = "0"
        generate_dyck_word(word, n, n0 + 1, n1, i + 1, code)
        return
    if n0 == n and n1 < n:
        word[i] = "1"
        generate_dyck_word(word, n, n0, n1 + 1, i + 1, code)


def generate_quaternary_dyck_word(word, n, n0, n1, i, code):
    if i == 2 * n:
        code.append("".join(word))
        return
    if n0 < n and n1 < n:
        word[i] = "T"
        generate_quaternary_dyck_word(word, n, n0 + 1, n1, i + 1, code)
        word[i] = "A"
        generate_quaternary_dyck_word(word, n, n0 + 1, n1, i + 1, code)
        if n0 > n1:
            word[i] = "C"
            generate_quaternary_dyck_word(word, n, n0, n1 + 1, i + 1, code)
            word[i] = "G"
            generate_quaternary_dyck_word(word, n, n0, n1 + 1, i + 1, code)
        return
    if n0 < n and n1 == n:
        word[i] = "T"
        generate_quaternary_dyck_word(word, n, n0 + 1, n1, i + 1, code)
        word[i] = "A"
        generate_quaternary_dyck_word(word, n, n0 + 1, n1, i + 1, code)
        return
    if n0 == n and n1 < n:
        word[i] = "C"
        generate_quaternary_dyck_word(word, n, n0, n1 + 1, i + 1, code)
        word[i] = "G"
        generate_quaternary_dyck_word(word, n, n0, n1 + 1, i + 1, code)
        return


# generates the balanced Dyck words of length 2n starting with a sequence of at most l-1 ones,
# ending in a sequence of at most l-1, and having no runs longer than l
def generate_restricted_dyck_word2(word, n, n0, n1, i, code, last_run, l, is_first_run):
    if last_run > l:
        return
    if is_first_run and last_run > l - 1:
        return
    if i == 2 * n:
        if last_run > l - 1:
            return
        code.append("".join(word))
        return
    if n0 < n and n1 < n:
        word[i] = "0"
        old_run = last_run
        if i > 0 and word[i - 1] == "0":
            last_run += 1
        else:
            last_run = 1
        generate_restricted_dyck_word2(word, n, n0 + 1, n1, i + 1, code, last_run, l, is_first_run)
        if n0 > n1:
            word[i] = "1"
            if word[i - 1] == "1":
                last_run = old_run + 1
            else:
                last_run = 1
            generate_restricted_dyck_word2(word, n, n0, n1 + 1, i + 1, code, last_run, l, False)
        return
    if n0 < n and n1 == n:
        word[i] = "0"
        if word[i - 1] == "0":
            last_run += 1
        else:
            last_run = 1
        generate_restricted_dyck_word2(word, n, n0 + 1, n1, i + 1, code, last_run, l, is_first_run)
        return
    if n0 == n and n1 < n:
        word[i] = "1"
        if word[i - 1] == "1":
            last_run += 1
        else:
            last_run = 1
        generate_restricted_dyck_word2(word, n, n0, n1 + 1, i + 1, code, last_run, l, False)


# generates the balanced Dyck words of length 2n having no runs longer than l
def generate_restricted_dyck_word(word, n, n0, n1, i, code, last_run, l):
    if last_run > l:
        return
    if i == 2 * n:
        code.append("".join(word))
        return
    if n0 < n and n1 < n:
        word[i] = "0"
        old_run = last_run
        if i > 0 and word[i - 1] == "0":
            last_run += 1
        else:
            last_run = 1
        generate_restricted_dyck_word(word, n, n0 + 1, n1, i + 1, code, last_run, l)
        if n0 > n1:
            word[i] = "1"
            if word[i - 1] == "1":
                last_run = old_run + 1
            else:
                last_run = 1
            generate_restricted_dyck_word(word, n, n0, n1 + 1, i + 1, code, last_run, l)
        return
    if n0 < n and n1 == n:
        word[i] = "0"
        if word[i - 1] == "0":
            last_run += 1
        else:
            last_run = 1
        generate_restricted_dyck_word(word, n, n0 + 1, n1, i + 1, code, last_run, l)
        return
    if n0 == n and n1 < n:
        word[i] = "1"
        if word[i - 1] == "1":
            last_run += 1
        else:
            last_run = 1
        generate_restricted_dyck_word(word, n, n0, n1 + 1, i + 1, code, last_run, l)


def count_restricted_non_overlapping_codes(n, l):
    if n % 2 == 0:
        code_size = 0
        for i in range(int(n / 2) + 1):
            a = [None] * (2 * i)
            code = []
            generate_restricted_dyck_word(a, i, 0, 0, 0, code, 0, l)
            b = [None] * (2 * (n - i))
            code2 = []
            generate_restricted_dyck_word2(b, n - i, 0, 0, 0, code2, 0, l, True)
            if len(code) == 0:
                code_size += len(code2)
            elif len(code2) == 0:
                code_size += len(code)
            else:
                code_size += len(code) * len(code2)
        print(code_size)
    if n % 2 == 1:
        code_size = 0
        for i in range(int((n + 1) / 2) + 1):
            a = [None] * (2 * i)
            code = []
            generate_restricted_dyck_word(a, i, 0, 0, 0, code, 0, l)
            b = [None] * (2 * (n - i))
            code2 = []
            generate_restricted_dyck_word2(b, n - i, 0, 0, 0, code2, 0, l, True)
            if len(code) == 0:
                code_size += len(code2)
            elif len(code2) == 0:
                code_size += len(code)
            else:
                code_size += len(code) * len(code2)
                b = [None] * (2 * (n - i))
        w = [None] * (n - 1)
        code2 = []
        generate_restricted_dyck_word2(w, (n - 1) / 2, 0, 0, 0, code2, 0, l, True)
        code_size -= len(code2) ** 2
        print(code_size)


def enumerate_restricted_dyck_paths(n, l):
    word = [None] * (2 * n)
    code = []
    generate_restricted_dyck_word(word, n, 0, 0, 0, code, 0, l)
    print(len(code))


def generate_table(l):
    for n in range(1, 19):
        enumerate_restricted_dyck_paths(n, l)


def enumerate_restricted_dyck_paths2(n, l):
    word = [None] * (2 * n)
    code = []
    generate_restricted_dyck_word2(word, n, 0, 0, 0, code, 0, l, True)
    print(len(code))


def generate_table2(l):
    for n in range(1, 19):
        enumerate_restricted_dyck_paths2(n, l)


if __name__ == "__main__":
    generate_table(int(sys.argv[1]))
    generate_table2(int(sys.argv[1]))
