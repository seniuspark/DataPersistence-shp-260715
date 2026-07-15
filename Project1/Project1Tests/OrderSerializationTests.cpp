#include <gtest/gtest.h>
#include "Order.h"

TEST(OrderSerializationTests, Order_ToJson_ProducesExpectedFields) {
    Order order{"ORD-20260416-0043", "S-001", "고객사A", 50, OrderStatus::RESERVED, "2026-04-16T09:00:00"};

    nlohmann::json json = ToJson(order);

    EXPECT_EQ(json.at("orderId").get<std::string>(), "ORD-20260416-0043");
    EXPECT_EQ(json.at("sampleId").get<std::string>(), "S-001");
    EXPECT_EQ(json.at("customerName").get<std::string>(), "고객사A");
    EXPECT_EQ(json.at("quantity").get<int>(), 50);
    EXPECT_EQ(json.at("status").get<std::string>(), "RESERVED");
    EXPECT_EQ(json.at("createdAt").get<std::string>(), "2026-04-16T09:00:00");
}

class OrderStatusRoundTripTests : public ::testing::TestWithParam<OrderStatus> {};

TEST_P(OrderStatusRoundTripTests, Order_RoundTrip_PreservesAllFields) {
    Order original{"ORD-20260416-0001", "S-001", "고객사A", 50, GetParam(), "2026-04-16T09:00:00"};

    Order restored = OrderFromJson(ToJson(original));

    EXPECT_EQ(restored.orderId, original.orderId);
    EXPECT_EQ(restored.sampleId, original.sampleId);
    EXPECT_EQ(restored.customerName, original.customerName);
    EXPECT_EQ(restored.quantity, original.quantity);
    EXPECT_EQ(restored.status, original.status);
    EXPECT_EQ(restored.createdAt, original.createdAt);
}

INSTANTIATE_TEST_CASE_P(
    AllStatuses,
    OrderStatusRoundTripTests,
    ::testing::Values(
        OrderStatus::RESERVED,
        OrderStatus::REJECTED,
        OrderStatus::PRODUCING,
        OrderStatus::CONFIRMED,
        OrderStatus::RELEASE));

TEST(OrderSerializationTests, Order_InvalidStatusString_ThrowsOrHandlesDefined) {
    nlohmann::json json;
    json["orderId"] = "ORD-20260416-0001";
    json["sampleId"] = "S-001";
    json["customerName"] = "고객사A";
    json["quantity"] = 50;
    json["status"] = "UNKNOWN_STATUS";
    json["createdAt"] = "2026-04-16T09:00:00";

    EXPECT_THROW(OrderFromJson(json), std::invalid_argument);
}

TEST(OrderSerializationTests, OrderList_ToJsonArray_RoundTrip) {
    std::vector<Order> orders{
        {"ORD-20260416-0001", "S-001", "고객사A", 50, OrderStatus::RESERVED, "2026-04-16T09:00:00"},
        {"ORD-20260416-0002", "S-002", "고객사B", 10, OrderStatus::RELEASE, "2026-04-17T10:00:00"},
    };

    nlohmann::json wrapped;
    wrapped["orders"] = nlohmann::json::array();
    for (const auto& order : orders) {
        wrapped["orders"].push_back(ToJson(order));
    }

    std::vector<Order> restored;
    for (const auto& item : wrapped.at("orders")) {
        restored.push_back(OrderFromJson(item));
    }

    ASSERT_EQ(restored.size(), orders.size());
    for (size_t i = 0; i < orders.size(); ++i) {
        EXPECT_EQ(restored[i].orderId, orders[i].orderId);
        EXPECT_EQ(restored[i].sampleId, orders[i].sampleId);
        EXPECT_EQ(restored[i].customerName, orders[i].customerName);
        EXPECT_EQ(restored[i].quantity, orders[i].quantity);
        EXPECT_EQ(restored[i].status, orders[i].status);
        EXPECT_EQ(restored[i].createdAt, orders[i].createdAt);
    }
}
