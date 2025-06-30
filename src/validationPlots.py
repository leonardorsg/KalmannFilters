import pandas as pd
import matplotlib.pyplot as plt
import os

# Optional for headless environments
import matplotlib
matplotlib.use('Agg')


def load_validation_data(file_path="../TestResults/validation_results.csv"):
    if not os.path.exists(file_path):
        print(f"[ERROR] File '{file_path}' not found.")
        return None

    df = pd.read_csv(file_path)
    if df.empty:
        print(f"[WARNING] File '{file_path}' is empty.")
        return None

    return df


def generate_histogram(df):
    plt.figure(figsize=(8, 5))
    plt.hist(df["abs_error"], bins=10, color="skyblue", edgecolor="black")
    plt.xlabel("Absolute ETA Error (minutes)")
    plt.ylabel("Frequency")
    plt.title("Distribution of ETA Errors")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("../plots/eta_error_histogram.png")
    print("[INFO] Saved: plots/eta_error_histogram.png")


def generate_boxplot(df):
    plt.figure(figsize=(6, 4))
    plt.boxplot(df["abs_error"], vert=False)
    plt.xlabel("Absolute ETA Error (minutes)")
    plt.title("Boxplot of ETA Errors")
    plt.tight_layout()
    plt.savefig("../plots/eta_error_boxplot.png")
    print("[INFO] Saved: plots/eta_error_boxplot.png")


def generate_summary(df):
    summary = {
        "count": len(df),
        "mean_error": df["abs_error"].mean(),
        "std_dev": df["abs_error"].std(),
        "max_error": df["abs_error"].max(),
        "min_error": df["abs_error"].min(),
        "within_5_min": (df["abs_error"] <= 5).sum(),
        "within_10_min": (df["abs_error"] <= 10).sum()
    }

    print("\n[Summary Statistics]")
    for key, value in summary.items():
        print(f"{key.replace('_', ' ').title()}: {value}")

    return summary


if __name__ == "__main__":
    os.makedirs("../plots", exist_ok=True)
    df = load_validation_data()

    if df is not None:
        generate_summary(df)
        generate_histogram(df)
        generate_boxplot(df)
