#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
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

private:
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
