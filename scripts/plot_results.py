import csv
import matplotlib.pyplot as plt


def read_benchmark_results(filename):
    orders = []
    add_ms = []
    modify_ms = []
    delete_ms = []
    total_ms = []

    with open(filename, "r", newline="") as file:
        reader = csv.DictReader(file)

        for row in reader:
            orders.append(int(row["orders"]))
            add_ms.append(float(row["add_ms"]))
            modify_ms.append(float(row["modify_ms"]))
            delete_ms.append(float(row["delete_ms"]))
            total_ms.append(float(row["total_ms"]))

    return orders, add_ms, modify_ms, delete_ms, total_ms


def main():
    orders, add_ms, modify_ms, delete_ms, total_ms = read_benchmark_results(
        "benchmark_results.csv"
    )

    plt.figure()
    plt.plot(orders, total_ms, marker="o", linestyle="-")
    plt.xlabel("Number of Orders")
    plt.ylabel("Execution Time (milliseconds)")
    plt.title("HFT Order Book Total Execution Time")
    plt.grid(True)
    plt.savefig("performance_chart.png", dpi=300, bbox_inches="tight")
    plt.show()

    plt.figure()
    plt.plot(orders, add_ms, marker="o", linestyle="-", label="Add")
    plt.plot(orders, modify_ms, marker="o", linestyle="-", label="Modify")
    plt.plot(orders, delete_ms, marker="o", linestyle="-", label="Delete")
    plt.xlabel("Number of Orders")
    plt.ylabel("Execution Time (milliseconds)")
    plt.title("Order Book Operation Latency Breakdown")
    plt.legend()
    plt.grid(True)
    plt.savefig("latency_breakdown.png", dpi=300, bbox_inches="tight")
    plt.show()


if __name__ == "__main__":
    main()