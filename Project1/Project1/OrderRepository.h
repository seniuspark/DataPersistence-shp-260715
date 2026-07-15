#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
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

    bool Update(const Order& order) {
        std::vector<Order> orders = LoadFromFile();
        for (auto& existing : orders) {
            if (existing.orderId == order.orderId) {
                existing = order;
                SaveAll(orders);
                return true;
            }
        }
        return false;
    }

    bool Delete(const std::string& orderId) {
        std::vector<Order> orders = LoadFromFile();
        auto it = std::find_if(orders.begin(), orders.end(),
                                [&](const Order& order) { return order.orderId == orderId; });
        if (it == orders.end()) {
            return false;
        }
        orders.erase(it);
        SaveAll(orders);
        return true;
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

        std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (content.empty()) {
            return {};
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(content);
        } catch (const nlohmann::json::parse_error&) {
            return {};
        }

        if (!json.is_object() || !json.contains("orders") || !json.at("orders").is_array()) {
            return {};
        }

        std::vector<Order> orders;
        for (const auto& item : json.at("orders")) {
            try {
                orders.push_back(OrderFromJson(item));
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "Skipping invalid order entry: " << e.what() << std::endl;
            }
        }
        return orders;
    }

    std::filesystem::path filePath_;
};
