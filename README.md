# FINM-32700-phase-5
HFT Order book

## Overview

This project implements a simplified high-frequency trading order book in C++. The main goal is to build a fast order management system that can add, modify, delete, and match orders efficiently.

The order book simulates the core structure used by trading systems to manage buy and sell orders. It is not a trading strategy or price prediction model. Instead, it focuses on the low level data structure and performance side of high frequency trading.

The project supports:

- Adding new limit orders
- Modifying existing orders
- Deleting orders
- Looking up orders by string-based order ID
- Tracking best bid and best ask
- Executing simple market orders
- Running unit tests
- Running stress tests
- Measuring execution time with benchmarks
- Generating performance charts

---

1. Sorted price levels

std::map<double, OrderMap, std::greater<double>> bids;
std::map<double, OrderMap> asks;

The bid side is sorted from highest price to lowest price because the highest buy price has priority.

The ask side is sorted from lowest price to highest price because the lowest sell price has priority.

This makes it easy to find:

Best bid = highest buy price
Best ask = lowest sell price



2. Fast order ID lookup
   
std::unordered_map<std::string, Order> orderLookup;

This allows the system to quickly find an order using its string ID.

For example, if the system needs to modify or delete ORD123, it does not need to scan the whole book. It can find the order directly through the hash table.


## Core Features

a) Add Order

The addOrder function inserts a new order into the correct side of the book.

If the order is a buy order, it goes into the bid book.
If the order is a sell order, it goes into the ask book.

The order is also stored in the lookup table for fast access by ID.


b) Modify Order

The modifyOrder function updates the price and quantity of an existing order.

Since price is used as a key in the sorted map, modifying an order requires removing it from its old price level and inserting it again at the new price level.


c) Delete Order

The deleteOrder function removes an order from both the price level map and the order lookup table.

If a price level becomes empty after deletion, the price level is removed from the book.


d) Best Bid and Best Ask

The order book can return the current best bid and best ask.

The best bid is the highest buy price.
The best ask is the lowest sell price.

These values are important because they describe the top of the market.


e) Market Order Execution

The project also includes a simple market order execution function.

A buy market order consumes liquidity from the lowest ask price.
A sell market order consumes liquidity from the highest bid price.

This simulates the basic behavior of order matching in an exchange.


Performance Analysis

The benchmark results show that execution time increases as the number of orders increases. This is expected because the order book has to manage more price levels and more order IDs.

Adding orders requires inserting into both the sorted price map and the hash lookup table.

Modifying orders is more expensive than a simple lookup because the order must be removed from the old price level and inserted into the new price level.

Deleting orders is usually faster because the system removes the order from its current price level and from the lookup table.

The main bottleneck is the sorted std::map structure. Accessing and updating price levels costs approximately O(log P), where P is the number of price levels.

The std::unordered_map lookup is faster on average because order ID lookup is approximately O(1).


Optimization Techniques

Preallocated Hash Table

The order lookup table reserves memory in advance:

orderLookup.reserve(expectedOrders);

This reduces rehashing and repeated memory allocation during large stress tests.

Separate Bid and Ask Books

The implementation keeps bids and asks in separate maps.

This makes the best bid and best ask logic simple and fast.

Sorted Price Levels

Bids are sorted in descending order.
Asks are sorted in ascending order.

This allows the order book to access the top of book using begin().



Atomic Order Counter

The active order count is stored using:

std::atomic<int>

The code uses relaxed memory ordering because the benchmark is single-threaded and does not require strict synchronization.

Efficient Modify and Delete

The system uses the lookup table to find the order quickly, then removes it directly from its current price level.

This avoids scanning the whole order book.
































