#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "OrderRepository.h"
#include "SampleRepository.h"

namespace {

std::filesystem::path MakeTestDir(const std::string& testName) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "PersistenceReload" / testName;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

}  // namespace

TEST(PersistenceReloadTests, SampleRepository_Restart_ReloadsPreviouslySavedSamples) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_Restart_ReloadsPreviouslySavedSamples") / "samples.json";

    {
        SampleRepository instanceA(filePath);
        instanceA.Add(Sample{"S-101", "Alpha", 10.0, 0.9, 50});
        instanceA.Add(Sample{"S-102", "Beta", 20.0, 0.8, 30});
    }

    SampleRepository instanceB(filePath);
    std::vector<Sample> reloaded = instanceB.GetAll();

    ASSERT_EQ(reloaded.size(), 2u);
    EXPECT_EQ(reloaded[0].sampleId, "S-101");
    EXPECT_EQ(reloaded[0].name, "Alpha");
    EXPECT_DOUBLE_EQ(reloaded[0].avgProductionTime, 10.0);
    EXPECT_DOUBLE_EQ(reloaded[0].yield, 0.9);
    EXPECT_EQ(reloaded[0].stock, 50);
    EXPECT_EQ(reloaded[1].sampleId, "S-102");
    EXPECT_EQ(reloaded[1].name, "Beta");
    EXPECT_DOUBLE_EQ(reloaded[1].avgProductionTime, 20.0);
    EXPECT_DOUBLE_EQ(reloaded[1].yield, 0.8);
    EXPECT_EQ(reloaded[1].stock, 30);

    std::optional<Sample> found = instanceB.FindById("S-101");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Alpha");
}

TEST(PersistenceReloadTests, OrderRepository_Restart_ReloadsPreviouslySavedOrders) {
    std::filesystem::path filePath = MakeTestDir("OrderRepository_Restart_ReloadsPreviouslySavedOrders") / "orders.json";

    {
        OrderRepository instanceA(filePath);
        instanceA.Add(Order{"ORD-20260715-0001", "S-101", "Alice", 10, OrderStatus::RESERVED, "2026-07-15T09:00:00"});
        instanceA.Add(Order{"ORD-20260715-0002", "S-102", "Bob", 5, OrderStatus::PRODUCING, "2026-07-15T10:00:00"});
    }

    OrderRepository instanceB(filePath);
    std::vector<Order> reloaded = instanceB.GetAll();

    ASSERT_EQ(reloaded.size(), 2u);
    EXPECT_EQ(reloaded[0].orderId, "ORD-20260715-0001");
    EXPECT_EQ(reloaded[0].sampleId, "S-101");
    EXPECT_EQ(reloaded[0].customerName, "Alice");
    EXPECT_EQ(reloaded[0].quantity, 10);
    EXPECT_EQ(reloaded[0].status, OrderStatus::RESERVED);
    EXPECT_EQ(reloaded[0].createdAt, "2026-07-15T09:00:00");
    EXPECT_EQ(reloaded[1].orderId, "ORD-20260715-0002");
    EXPECT_EQ(reloaded[1].status, OrderStatus::PRODUCING);

    std::optional<Order> found = instanceB.FindById("ORD-20260715-0002");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->customerName, "Bob");
    EXPECT_EQ(found->status, OrderStatus::PRODUCING);
}

TEST(PersistenceReloadTests, SampleRepository_Restart_PreservesInsertionOrder) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_Restart_PreservesInsertionOrder") / "samples.json";

    {
        SampleRepository instanceA(filePath);
        instanceA.Add(Sample{"S-301", "First", 1.0, 1.0, 1});
        instanceA.Add(Sample{"S-302", "Second", 2.0, 1.0, 2});
        instanceA.Add(Sample{"S-303", "Third", 3.0, 1.0, 3});
    }

    SampleRepository instanceB(filePath);
    std::vector<Sample> reloaded = instanceB.GetAll();

    ASSERT_EQ(reloaded.size(), 3u);
    EXPECT_EQ(reloaded[0].sampleId, "S-301");
    EXPECT_EQ(reloaded[1].sampleId, "S-302");
    EXPECT_EQ(reloaded[2].sampleId, "S-303");
}

TEST(PersistenceReloadTests, SampleRepository_Restart_MultipleReloadsAreIdempotent) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_Restart_MultipleReloadsAreIdempotent") / "samples.json";

    {
        SampleRepository instanceA(filePath);
        instanceA.Add(Sample{"S-401", "Only", 1.0, 1.0, 1});
    }

    SampleRepository instanceB(filePath);
    std::vector<Sample> first = instanceB.GetAll();
    std::vector<Sample> second = instanceB.GetAll();
    std::vector<Sample> third = instanceB.GetAll();

    ASSERT_EQ(first.size(), 1u);
    ASSERT_EQ(second.size(), 1u);
    ASSERT_EQ(third.size(), 1u);
    EXPECT_EQ(first[0].sampleId, "S-401");
    EXPECT_EQ(second[0].sampleId, "S-401");
    EXPECT_EQ(third[0].sampleId, "S-401");
}

TEST(PersistenceReloadTests, SampleAndOrderRepository_Restart_IndependentFiles) {
    std::filesystem::path dir = MakeTestDir("SampleAndOrderRepository_Restart_IndependentFiles");
    std::filesystem::path samplesPath = dir / "samples.json";
    std::filesystem::path ordersPath = dir / "orders.json";

    {
        SampleRepository sampleInstanceA(samplesPath);
        sampleInstanceA.Add(Sample{"S-501", "OnlySample", 1.0, 1.0, 1});

        OrderRepository orderInstanceA(ordersPath);
        orderInstanceA.Add(Order{"ORD-20260715-0501", "S-501", "Carol", 1, OrderStatus::RESERVED, "2026-07-15T11:00:00"});
    }

    SampleRepository sampleInstanceB(samplesPath);
    OrderRepository orderInstanceB(ordersPath);

    std::vector<Sample> samples = sampleInstanceB.GetAll();
    std::vector<Order> orders = orderInstanceB.GetAll();

    ASSERT_EQ(samples.size(), 1u);
    EXPECT_EQ(samples[0].sampleId, "S-501");
    ASSERT_EQ(orders.size(), 1u);
    EXPECT_EQ(orders[0].orderId, "ORD-20260715-0501");

    EXPECT_FALSE(sampleInstanceB.FindById("ORD-20260715-0501").has_value());
    EXPECT_FALSE(orderInstanceB.FindById("S-501").has_value());
}

TEST(PersistenceReloadTests, SampleRepository_Restart_AfterMultipleAddsAcrossInstances) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_Restart_AfterMultipleAddsAcrossInstances") / "samples.json";

    {
        SampleRepository instanceA(filePath);
        instanceA.Add(Sample{"S-601", "First", 1.0, 1.0, 1});
    }

    {
        SampleRepository instanceB(filePath);
        ASSERT_EQ(instanceB.GetAll().size(), 1u);
        instanceB.Add(Sample{"S-602", "Second", 2.0, 1.0, 2});
    }

    SampleRepository instanceC(filePath);
    std::vector<Sample> all = instanceC.GetAll();

    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].sampleId, "S-601");
    EXPECT_EQ(all[1].sampleId, "S-602");
}
