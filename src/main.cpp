#include "OrderBook.hpp"
#include "Timer.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

struct BenchmarkResult {
    int orderCount;
    double addMilliseconds;
    double modifyMilliseconds;
    double deleteMilliseconds;
    double totalMilliseconds;
};

std::vector<Order> generateOrders(int numOrders) {
    std::vector<Order> orders;
    orders.reserve(numOrders);

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> priceDist(50.0, 100.0);
    std::uniform_int_distribution<int> quantityDist(1, 500);
    std::bernoulli_distribution sideDist(0.5);

    for (int i = 0; i < numOrders; ++i) {
        std::string id = "ORD" + std::to_string(i);
        double price = std::round(priceDist(rng) * 100.0) / 100.0;
        int quantity = quantityDist(rng);
        bool isBuy = sideDist(rng);

        orders.emplace_back(id, price, quantity, isBuy);
    }

    return orders;
}

BenchmarkResult runBenchmark(int numOrders) {
    OrderBook book(static_cast<size_t>(numOrders * 2));
    std::vector<Order> orders = generateOrders(numOrders);

    Timer totalTimer;

    Timer addTimer;
    for (const auto& order : orders) {
        book.addOrder(order.id, order.price, order.quantity, order.isBuy);
    }
    double addMs = addTimer.elapsedMilliseconds();

    Timer modifyTimer;
    for (int i = 0; i < numOrders; i += 2) {
        double newPrice = orders[i].price + 0.01;
        int newQuantity = orders[i].quantity + 10;

        book.modifyOrder(orders[i].id, newPrice, newQuantity);
    }
    double modifyMs = modifyTimer.elapsedMilliseconds();

    Timer deleteTimer;
    for (int i = 1; i < numOrders; i += 2) {
        book.deleteOrder(orders[i].id);
    }
    double deleteMs = deleteTimer.elapsedMilliseconds();

    double totalMs = totalTimer.elapsedMilliseconds();

    return BenchmarkResult{
        numOrders,
        addMs,
        modifyMs,
        deleteMs,
        totalMs
    };
}

void writeResultsToCsv(const std::vector<BenchmarkResult>& results) {
    std::ofstream file("benchmark_results.csv");

    file << "orders,add_ms,modify_ms,delete_ms,total_ms\n";

    for (const auto& result : results) {
        file << result.orderCount << ","
             << result.addMilliseconds << ","
             << result.modifyMilliseconds << ","
             << result.deleteMilliseconds << ","
             << result.totalMilliseconds << "\n";
    }

    file.close();
}

int main() {
    std::cout << "Phase 5: HFT Order Book Benchmark\n";
    std::cout << "=================================\n\n";

    std::vector<int> orderSizes = {
        1000,
        5000,
        10000,
        50000,
        100000,
        250000,
        500000
    };

    std::vector<BenchmarkResult> results;
    results.reserve(orderSizes.size());

    std::cout << std::fixed << std::setprecision(4);

    for (int size : orderSizes) {
        BenchmarkResult result = runBenchmark(size);
        results.push_back(result);

        double throughput = size / (result.totalMilliseconds / 1000.0);

        std::cout << "Orders: " << size << "\n";
        std::cout << "Add time: " << result.addMilliseconds << " ms\n";
        std::cout << "Modify time: " << result.modifyMilliseconds << " ms\n";
        std::cout << "Delete time: " << result.deleteMilliseconds << " ms\n";
        std::cout << "Total time: " << result.totalMilliseconds << " ms\n";
        std::cout << "Approx throughput: " << throughput << " operations/sec\n";
        std::cout << "---------------------------------\n";
    }

    writeResultsToCsv(results);

    std::cout << "\nBenchmark results saved to benchmark_results.csv\n";

    OrderBook demoBook;
    demoBook.addOrder("BUY001", 99.50, 100, true);
    demoBook.addOrder("BUY002", 100.25, 150, true);
    demoBook.addOrder("SELL001", 101.00, 200, false);
    demoBook.addOrder("SELL002", 100.75, 100, false);

    std::cout << "\nDemo Order Book:\n";
    demoBook.printTopOfBook();

    int filled = demoBook.executeMarketOrder(true, 120);

    std::cout << "\nExecuted buy market order for 120 shares.\n";
    std::cout << "Filled quantity: " << filled << "\n";
    demoBook.printTopOfBook();

    return 0;
}