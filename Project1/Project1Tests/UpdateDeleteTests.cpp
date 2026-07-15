#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "OrderRepository.h"
#include "SampleRepository.h"

namespace {

std::filesystem::path MakeTestDir(const std::string& testName) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "UpdateDelete" / testName;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

}  // namespace

TEST(UpdateDeleteTests, SampleRepository_Update_ExistingSample_ChangesPersistAfterReload) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_Update_ExistingSample_ChangesPersistAfterReload") / "samples.json";

    {
        SampleRepository instanceA(filePath);
        instanceA.Add(Sample{"S-701", "Original", 10.0, 0.9, 50});

        bool updated = instanceA.Update(Sample{"S-701", "Renamed", 10.0, 0.9, 5});
        EXPECT_TRUE(updated);
    }

    SampleRepository instanceB(filePath);
    std::optional<Sample> reloaded = instanceB.FindById("S-701");
    ASSERT_TRUE(reloaded.has_value());
    EXPECT_EQ(reloaded->name, "Renamed");
    EXPECT_EQ(reloaded->stock, 5);
}

TEST(UpdateDeleteTests, SampleRepository_Update_NonExistentId_ReturnsFalse) {
    SampleRepository repository(MakeTestDir("SampleRepository_Update_NonExistentId_ReturnsFalse") / "samples.json");
    repository.Add(Sample{"S-702", "Existing", 1.0, 1.0, 1});

    bool updated = repository.Update(Sample{"S-999", "Ghost", 1.0, 1.0, 1});

    EXPECT_FALSE(updated);
    EXPECT_EQ(repository.GetAll().size(), 1u);
}

TEST(UpdateDeleteTests, SampleRepository_Delete_ExistingSample_RemovedAfterReload) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_Delete_ExistingSample_RemovedAfterReload") / "samples.json";

    {
        SampleRepository instanceA(filePath);
        instanceA.Add(Sample{"S-703", "ToDelete", 1.0, 1.0, 1});
        instanceA.Add(Sample{"S-704", "Keep", 1.0, 1.0, 1});

        bool deleted = instanceA.Delete("S-703");
        EXPECT_TRUE(deleted);
    }

    SampleRepository instanceB(filePath);
    std::vector<Sample> reloaded = instanceB.GetAll();
    ASSERT_EQ(reloaded.size(), 1u);
    EXPECT_EQ(reloaded[0].sampleId, "S-704");
    EXPECT_FALSE(instanceB.FindById("S-703").has_value());
}

TEST(UpdateDeleteTests, SampleRepository_Delete_NonExistentId_ReturnsFalse) {
    SampleRepository repository(MakeTestDir("SampleRepository_Delete_NonExistentId_ReturnsFalse") / "samples.json");
    repository.Add(Sample{"S-705", "Existing", 1.0, 1.0, 1});

    bool deleted = repository.Delete("S-999");

    EXPECT_FALSE(deleted);
    EXPECT_EQ(repository.GetAll().size(), 1u);
}

TEST(UpdateDeleteTests, OrderRepository_Update_StatusTransition_PersistsAfterReload) {
    std::filesystem::path filePath = MakeTestDir("OrderRepository_Update_StatusTransition_PersistsAfterReload") / "orders.json";

    {
        OrderRepository instanceA(filePath);
        instanceA.Add(Order{"ORD-20260715-0701", "S-701", "Alice", 10, OrderStatus::RESERVED, "2026-07-15T09:00:00"});

        Order updatedOrder{"ORD-20260715-0701", "S-701", "Alice", 10, OrderStatus::CONFIRMED, "2026-07-15T09:00:00"};
        bool updated = instanceA.Update(updatedOrder);
        EXPECT_TRUE(updated);
    }

    OrderRepository instanceB(filePath);
    std::optional<Order> reloaded = instanceB.FindById("ORD-20260715-0701");
    ASSERT_TRUE(reloaded.has_value());
    EXPECT_EQ(reloaded->status, OrderStatus::CONFIRMED);
}

TEST(UpdateDeleteTests, OrderRepository_Update_NonExistentId_ReturnsFalse) {
    OrderRepository repository(MakeTestDir("OrderRepository_Update_NonExistentId_ReturnsFalse") / "orders.json");
    repository.Add(Order{"ORD-20260715-0702", "S-701", "Bob", 5, OrderStatus::RESERVED, "2026-07-15T10:00:00"});

    bool updated = repository.Update(
        Order{"ORD-NONEXISTENT", "S-701", "Ghost", 1, OrderStatus::RESERVED, "2026-07-15T11:00:00"});

    EXPECT_FALSE(updated);
}

TEST(UpdateDeleteTests, OrderRepository_Delete_ExistingOrder_RemovedAfterReload) {
    std::filesystem::path filePath = MakeTestDir("OrderRepository_Delete_ExistingOrder_RemovedAfterReload") / "orders.json";

    {
        OrderRepository instanceA(filePath);
        instanceA.Add(Order{"ORD-20260715-0703", "S-701", "Carol", 1, OrderStatus::RESERVED, "2026-07-15T12:00:00"});

        bool deleted = instanceA.Delete("ORD-20260715-0703");
        EXPECT_TRUE(deleted);
    }

    OrderRepository instanceB(filePath);
    EXPECT_TRUE(instanceB.GetAll().empty());
    EXPECT_FALSE(instanceB.FindById("ORD-20260715-0703").has_value());
}

TEST(UpdateDeleteTests, OrderRepository_Delete_NonExistentId_ReturnsFalse) {
    OrderRepository repository(MakeTestDir("OrderRepository_Delete_NonExistentId_ReturnsFalse") / "orders.json");
    repository.Add(Order{"ORD-20260715-0704", "S-701", "Dave", 1, OrderStatus::RESERVED, "2026-07-15T13:00:00"});

    bool deleted = repository.Delete("ORD-NONEXISTENT");

    EXPECT_FALSE(deleted);
    EXPECT_EQ(repository.GetAll().size(), 1u);
}
