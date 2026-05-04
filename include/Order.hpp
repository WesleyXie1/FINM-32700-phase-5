#ifndef ORDER_HPP
#define ORDER_HPP

#include <string>
// 定义四个状态
struct Order {
    std::string id;
    double price;
    int quantity;
    bool isBuy;

    Order() = default;

    Order(const std::string& orderId, double orderPrice, int orderQuantity, bool buySide)
        : id(orderId), price(orderPrice), quantity(orderQuantity), isBuy(buySide) {}
};

#endif