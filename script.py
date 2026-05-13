import pandas as pd
import matplotlib.pyplot as plt

CSV_FILE = "build/ber_results.csv"

data = pd.read_csv(CSV_FILE)

codes = data["code"].unique()

# -------------------------------------------------
# Отдельный график для каждого кода
# -------------------------------------------------

for code in codes:
    subset = data[data["code"] == code]

    plt.figure(figsize=(8, 5))

    plt.plot(
        subset["p"],
        subset["ber"],
        marker="o"
    )

    plt.xlabel("Channel error probability p")
    plt.ylabel("Bit error rate after Viterbi decoding")
    plt.title(f"BER vs channel error probability ({code})")

    plt.grid(True)

    # BER обычно смотрят в логарифмическом масштабе
    plt.yscale("log")

    plt.tight_layout()

    output_name = f"ber_{code.replace('/', '_')}.png"
    plt.savefig(output_name)

    print(f"Saved: {output_name}")

    plt.close()

# -------------------------------------------------
# Общий график со всеми кривыми
# -------------------------------------------------

plt.figure(figsize=(10, 6))

for code in codes:
    subset = data[data["code"] == code]

    plt.plot(
        subset["p"],
        subset["ber"],
        marker="o",
        label=code
    )

plt.xlabel("Channel error probability p")
plt.ylabel("Bit error rate after Viterbi decoding")

plt.title("Comparison of convolutional codes")

plt.grid(True)

plt.yscale("log")

plt.legend()

plt.tight_layout()

plt.savefig("ber_comparison.png")

print("Saved: ber_comparison.png")

