import pandas as pd
import matplotlib.pyplot as plt

INPUT  = "../result/res.csv"
OUTPUT = "../pic/time_vs_n.png"

df  = pd.read_csv(INPUT)
agg = df.groupby(["n", "workers"])["time_s"].mean().reset_index()

fig2, ax2 = plt.subplots(figsize=(9, 6))
for p, d in agg.groupby("workers"):
    d = d.sort_values("n")
    ax2.plot(d.n, d.time_s, marker="o", label=f"P = {p}")
ax2.set_xlabel("Размер матрицы n")
ax2.set_ylabel("Время, с")
ax2.set_title("Время выполнения от размера матрицы")
ax2.set_yscale("log")
ax2.grid(True, which="both", alpha=0.3)
ax2.legend()
fig2.tight_layout()
fig2.savefig(OUTPUT, dpi=160, bbox_inches="tight")
plt.show()