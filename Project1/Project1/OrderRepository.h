#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Order.h"

class OrderRepository {
public:
    explicit OrderRepository(std::filesystem::path filePath) : filePath_(std::move(filePath)) {}

    std::vector<Order> GetAll() const {
        return LoadFromFile();
    }

    std::optional<Order> FindById(const std::string& orderId) const {
        for (const auto& order : LoadFromFile()) {
            if (order.orderId == orderId) {
                return order;
            }
        }
        return std::nullopt;
    }

    void Add(const Order& order) {
        std::vector<Order> orders = LoadFromFile();
        for (const auto& existing : orders) {
            if (existing.orderId == order.orderId) {
                throw std::invalid_argument("Order with orderId already exists: " + order.orderId);
            }
        }
        orders.push_back(order);
        SaveAll(orders);
    }

private:
    void SaveAll(const std::vector<Order>& orders) const {
        std::filesystem::create_directories(filePath_.parent_path());

        nlohmann::json json;
        json["orders"] = nlohmann::json::array();
        for (const auto& order : orders) {
            json["orders"].push_back(ToJson(order));
        }

        std::ofstream output(filePath_);
        output << json.dump(2);
    }

    std::vector<Order> LoadFromFile() const {
        if (!std::filesystem::exists(filePath_)) {
            return {};
        }

        std::ifstream input(filePath_);
        if (!input) {
            return {};
        }

        nlohmann::json json;
        input >> json;

        std::vector<Order> orders;
        for (const auto& item : json.at("orders")) {
            orders.push_back(OrderFromJson(item));
        }
        return orders;
    }

    std::filesystem::path filePath_;
};
