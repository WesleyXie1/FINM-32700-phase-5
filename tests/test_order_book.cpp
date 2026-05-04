#include "OrderBook.hpp"

#include <cassert>
#include <iostream>



void testAddOrder() {
    OrderBook book;

    bool added = book.addOrder("ORD001", 50.10, 100, true);

    assert(added);
    assert(book.containsOrder("ORD001"));
    assert(book.size() == 1);
    assert(book.bestBid().has_value());
    assert(book.bestBid().value() == 50.10);
}

void testModifyOrder() {
    OrderBook book;

    book.addOrder("ORD001", 50.10, 100, true);

    bool modified = book.modifyOrder("ORD001", 51.25, 250);

    assert(modified);
    assert(book.containsOrder("ORD001"));

    auto order = book.getOrder("ORD001");

    assert(order.has_value());
    assert(order->price == 51.25);
    assert(order->quantity == 250);
    assert(order->isBuy == true);
    assert(book.bestBid().value() == 51.25);
}

void testDeleteOrder() {
    OrderBook book;

    book.addOrder("ORD001", 50.10, 100, true);

    bool deleted = book.deleteOrder("ORD001");

    assert(deleted);
    assert(!book.containsOrder("ORD001"));
    assert(book.size() == 0);
    assert(!book.bestBid().has_value());
}

void testBestBidAndAsk() {
    OrderBook book;

    book.addOrder("B1", 99.00, 100, true);
    book.addOrder("B2", 101.00, 100, true);
    book.addOrder("B3", 100.00, 100, true);

    book.addOrder("S1", 105.00, 100, false);
    book.addOrder("S2", 103.00, 100, false);
    book.addOrder("S3", 104.00, 100, false);

    assert(book.bestBid().has_value());
    assert(book.bestAsk().has_value());

    assert(book.bestBid().value() == 101.00);
    assert(book.bestAsk().value() == 103.00);
}

void testMarketOrderExecution() {
    OrderBook book;

    book.addOrder("S1", 100.00, 50, false);
    book.addOrder("S2", 101.00, 100, false);

    int filled = book.executeMarketOrder(true, 70);

    assert(filled == 70);
    assert(!book.containsOrder("S1"));
    assert(book.containsOrder("S2"));

    auto remaining = book.getOrder("S2");
    assert(remaining.has_value());
    assert(remaining->quantity == 80);
}

void testInvalidOrders() {
    OrderBook book;

    assert(!book.addOrder("", 50.00, 100, true));
    assert(!book.addOrder("BAD_PRICE", -1.00, 100, true));
    assert(!book.addOrder("BAD_QTY", 50.00, 0, true));

    assert(book.size() == 0);
}

void stressTestSmall() {
    OrderBook book(10000);

    for (int i = 0; i < 10000; ++i) {
        std::string id = "ORD" + std::to_string(i);
        double price = 50.0 + static_cast<double>(i % 100);
        int quantity = 1 + (i % 500);
        bool isBuy = (i % 2 == 0);

        bool added = book.addOrder(id, price, quantity, isBuy);
        assert(added);
    }

    assert(book.size() == 10000);

    for (int i = 0; i < 5000; ++i) {
        std::string id = "ORD" + std::to_string(i);
        bool modified = book.modifyOrder(id, 75.0, 200);
        assert(modified);
    }

    for (int i = 5000; i < 10000; ++i) {
        std::string id = "ORD" + std::to_string(i);
        bool deleted = book.deleteOrder(id);
        assert(deleted);
    }

    assert(book.size() == 5000);
}

int main() {
    testAddOrder();
    testModifyOrder();
    testDeleteOrder();
    testBestBidAndAsk();
    testMarketOrderExecution();
    testInvalidOrders();
    stressTestSmall();

    std::cout << "All unit tests and stress tests passed successfully.\n";

    return 0;
}