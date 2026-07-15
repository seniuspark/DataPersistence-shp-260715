#pragma once

#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

enum class OrderStatus {
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASE,
};

inline std::string ToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::RESERVED:
            return "RESERVED";
        case OrderStatus::REJECTED:
            return "REJECTED";
        case OrderStatus::PRODUCING:
            return "PRODUCING";
        case OrderStatus::CONFIRMED:
            return "CONFIRMED";
        case OrderStatus::RELEASE:
            return "RELEASE";
    }
    throw std::invalid_argument("Unknown OrderStatus value");
}

inline OrderStatus ParseOrderStatus(const std::string& text) {
    if (text == "RESERVED") return OrderStatus::RESERVED;
    if (text == "REJECTED") return OrderStatus::REJECTED;
    if (text == "PRODUCING") return OrderStatus::PRODUCING;
    if (text == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (text == "RELEASE") return OrderStatus::RELEASE;
    throw std::invalid_argument("Unknown order status string: " + text);
}

struct Order {
    std::string orderId;
    std::string sampleId;
    std::string customerName;
    int quantity;
    OrderStatus status;
    std::string createdAt;
};

inline nlohmann::json ToJson(const Order& order) {
    return nlohmann::json{
        {"orderId", order.orderId},
        {"sampleId", order.sampleId},
        {"customerName", order.customerName},
        {"quantity", order.quantity},
        {"status", ToString(order.status)},
        {"createdAt", order.createdAt},
    };
}

inline Order OrderFromJson(const nlohmann::json& json) {
    return Order{
        json.at("orderId").get<std::string>(),
        json.at("sampleId").get<std::string>(),
        json.at("customerName").get<std::string>(),
        json.at("quantity").get<int>(),
        ParseOrderStatus(json.at("status").get<std::string>()),
        json.at("createdAt").get<std::string>(),
    };
}
