import numpy as np
import sys

sizes = [200, 400, 800, 1200, 1600, 2000]

for n in sizes:
    try:
        a = np.loadtxt(f"../../matrices/data/m{n}_1.txt", dtype=np.int64, comments='#')
        b = np.loadtxt(f"../../matrices/data/m{n}_2.txt", dtype=np.int64, comments='#')
        c = np.loadtxt(f"../../matrices/result/result{n}.txt", dtype=np.int64, comments='#')

        if np.array_equal(a @ b, c):
            print(f"[{n}x{n}] OK")
        else:
            print(f"[{n}x{n}] FAIL")
    except FileNotFoundError:
        print(f"[{n}x{n}] skip (no files)")