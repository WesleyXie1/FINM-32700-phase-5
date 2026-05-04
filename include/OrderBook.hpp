#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include "Order.hpp"

#include <atomic>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// 订单簿，负责存买单、存卖单、快速查订单、修改订单、删除订单，以及模拟市价单成交

class OrderBook {
private:
    using OrderMap = std::unordered_map<std::string, Order>;

    std::map<double, OrderMap, std::greater<double>> bids;      // 存买单   买单的规则是 价格越高越优先
    std::map<double, OrderMap> asks;                            // 存卖单   卖单的规则是：价格越低越优先

    std::unordered_map<std::string, Order> orderLookup;         // 用订单 ID 快速找订单

    std::atomic<int> activeOrderCount;                          // 记录当前 order book 里面还有多少 active orders


    // 从价格层删订单 
    /*
    给我一个 order，我根据它是 buy 还是 sell，
    去 bids 或 asks 里面找到它所在的 price level，
    然后把这个 order 删除。
    */
    void eraseFromPriceLevel(const Order& order) {
        if (order.isBuy) {
            auto levelIt = bids.find(order.price);
            if (levelIt != bids.end()) {
                levelIt->second.erase(order.id);

                if (levelIt->second.empty()) {
                    bids.erase(levelIt);
                }
            }
        } else {
            auto levelIt = asks.find(order.price);
            if (levelIt != asks.end()) {
                levelIt->second.erase(order.id);

                if (levelIt->second.empty()) {
                    asks.erase(levelIt);
                }
            }
        }
    }


// 初始化 order book
/*
activeOrderCount 初始为 0
给 orderLookup 提前 reserve 内存
*/    
public:
    explicit OrderBook(size_t expectedOrders = 100000) : activeOrderCount(0) {
        orderLookup.reserve(expectedOrders);
    }

// 添加订单
/*
1. 检查订单是否合法
2. 如果 ID 已经存在，先删掉旧订单
3. 创建一个 Order object
4. 如果是 buy，放进 bids
5. 如果是 sell，放进 asks
6. 同时放进 orderLookup
7. activeOrderCount 加 1
*/
    bool addOrder(const std::string& id, double price, int quantity, bool isBuy) {
        if (id.empty() || price <= 0.0 || quantity <= 0) {
            return false;
        }

        if (orderLookup.find(id) != orderLookup.end()) {
            deleteOrder(id);
        }

        Order order(id, price, quantity, isBuy);

        if (isBuy) {
            bids[price][id] = order;
        } else {
            asks[price][id] = order;
        }

        orderLookup[id] = order;
        activeOrderCount.fetch_add(1, std::memory_order_relaxed);

        return true;
    }

// 修改订单
/*
1. 先用 orderLookup 找这个订单
2. 找不到就 return false
3. 检查新价格和新数量是否合法
4. 保存旧订单的信息
5. 从旧价格层删掉旧订单
6. 创建新订单
7. 放到新的价格层
8. 更新 orderLookup
*/
    bool modifyOrder(const std::string& id, double newPrice, int newQuantity) {
        auto lookupIt = orderLookup.find(id);

        if (lookupIt == orderLookup.end()) {
            return false;
        }

        if (newPrice <= 0.0 || newQuantity <= 0) {
            return false;
        }

        Order oldOrder = lookupIt->second;
        bool side = oldOrder.isBuy;

        eraseFromPriceLevel(oldOrder);

        Order newOrder(id, newPrice, newQuantity, side);

        if (side) {
            bids[newPrice][id] = newOrder;
        } else {
            asks[newPrice][id] = newOrder;
        }

        lookupIt->second = newOrder;

        return true;
    }


// 删除订单
/*
1. 用 orderLookup 找订单
2. 找不到就 return false
3. 找到以后，根据订单信息去 bids 或 asks 删除它
4. 从 orderLookup 删除它
5. activeOrderCount 减 1
*/
    bool deleteOrder(const std::string& id) {
        auto lookupIt = orderLookup.find(id);

        if (lookupIt == orderLookup.end()) {
            return false;
        }

        Order order = lookupIt->second;

        eraseFromPriceLevel(order);
        orderLookup.erase(lookupIt);

        activeOrderCount.fetch_sub(1, std::memory_order_relaxed);

        return true;
    }

    // 检查订单是否存在
    /*
    如果存在，返回 true 
    如果不存在，返回 false
    */
    bool containsOrder(const std::string& id) const {
        return orderLookup.find(id) != orderLookup.end();
    }

    // getOrder 通过 ID 拿订单
    std::optional<Order> getOrder(const std::string& id) const {
        auto lookupIt = orderLookup.find(id);

        if (lookupIt == orderLookup.end()) {
            return std::nullopt;
        }

        return lookupIt->second;
    }

    // 函数返回当前最高买价
    std::optional<double> bestBid() const {
        if (bids.empty()) {
            return std::nullopt;
        }

        return bids.begin()->first;
    }

    // 函数返回当前最低卖价
    std::optional<double> bestAsk() const {
        if (asks.empty()) {
            return std::nullopt;
        }

        return asks.begin()->first;
    }

    int size() const {
        return activeOrderCount.load(std::memory_order_relaxed);
    }

// 返回价格层数量    
    size_t bidLevelCount() const {
        return bids.size();
    }

    size_t askLevelCount() const {
        return asks.size();
    }
// 清空 order book
    void clear() {
        bids.clear();
        asks.clear();
        orderLookup.clear();
        activeOrderCount.store(0, std::memory_order_relaxed);
    }
// 模拟市价单成交
    int executeMarketOrder(bool isBuyMarketOrder, int quantity) {
        if (quantity <= 0) {
            return 0;
        }

        int filledQuantity = 0;

        if (isBuyMarketOrder) {
            while (quantity > 0 && !asks.empty()) {
                auto levelIt = asks.begin();
                auto& ordersAtLevel = levelIt->second;

                auto orderIt = ordersAtLevel.begin();

                while (quantity > 0 && orderIt != ordersAtLevel.end()) {
                    Order& restingOrder = orderIt->second;
                    int fill = std::min(quantity, restingOrder.quantity);

                    restingOrder.quantity -= fill;
                    quantity -= fill;
                    filledQuantity += fill;

                    auto lookupIt = orderLookup.find(restingOrder.id);
                    if (lookupIt != orderLookup.end()) {
                        lookupIt->second.quantity = restingOrder.quantity;
                    }

                    if (restingOrder.quantity == 0) {
                        orderLookup.erase(restingOrder.id);
                        activeOrderCount.fetch_sub(1, std::memory_order_relaxed);
                        orderIt = ordersAtLevel.erase(orderIt);
                    } else {
                        ++orderIt;
                    }
                }

                if (ordersAtLevel.empty()) {
                    asks.erase(levelIt);
                }
            }
        } else {
            while (quantity > 0 && !bids.empty()) {
                auto levelIt = bids.begin();
                auto& ordersAtLevel = levelIt->second;

                auto orderIt = ordersAtLevel.begin();

                while (quantity > 0 && orderIt != ordersAtLevel.end()) {
                    Order& restingOrder = orderIt->second;
                    int fill = std::min(quantity, restingOrder.quantity);

                    restingOrder.quantity -= fill;
                    quantity -= fill;
                    filledQuantity += fill;

                    auto lookupIt = orderLookup.find(restingOrder.id);
                    if (lookupIt != orderLookup.end()) {
                        lookupIt->second.quantity = restingOrder.quantity;
                    }

                    if (restingOrder.quantity == 0) {
                        orderLookup.erase(restingOrder.id);
                        activeOrderCount.fetch_sub(1, std::memory_order_relaxed);
                        orderIt = ordersAtLevel.erase(orderIt);
                    } else {
                        ++orderIt;
                    }
                }

                if (ordersAtLevel.empty()) {
                    bids.erase(levelIt);
                }
            }
        }

        return filledQuantity;
    }

    void printTopOfBook() const {
        std::cout << "Top of Book\n";

        if (bestBid().has_value()) {
            std::cout << "Best Bid: " << bestBid().value() << "\n";
        } else {
            std::cout << "Best Bid: N/A\n";
        }

        if (bestAsk().has_value()) {
            std::cout << "Best Ask: " << bestAsk().value() << "\n";
        } else {
            std::cout << "Best Ask: N/A\n";
        }

        std::cout << "Active Orders: " << size() << "\n";
    }
};

#endif