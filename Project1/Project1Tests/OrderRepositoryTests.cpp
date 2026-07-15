#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "OrderRepository.h"

namespace {

std::filesystem::path MakeMissingFilePath(const std::string& testName) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / testName;
    std::filesystem::remove_all(dir);
    return dir / "orders.json";
}

}  // namespace

TEST(OrderRepositoryTests, FileDoesNotExist_GetAllReturnsEmpty) {
    OrderRepository repository(MakeMissingFilePath("Order_FileDoesNotExist_GetAllReturnsEmpty"));

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(OrderRepositoryTests, FileDoesNotExist_FindByIdReturnsNullopt) {
    OrderRepository repository(MakeMissingFilePath("Order_FileDoesNotExist_FindByIdReturnsNullopt"));

    EXPECT_FALSE(repository.FindById("ORD-20260416-0043").has_value());
}

TEST(OrderRepositoryTests, ConstructingDoesNotCreateFileImmediately) {
    std::filesystem::path filePath = MakeMissingFilePath("Order_ConstructingDoesNotCreateFileImmediately");

    OrderRepository repository(filePath);

    EXPECT_FALSE(std::filesystem::exists(filePath));
}

TEST(OrderRepositoryTests, DirectoryDoesNotExist_GetAllReturnsEmpty) {
    std::filesystem::path filePath = MakeMissingFilePath("Order_DirectoryDoesNotExist_GetAllReturnsEmpty");
    ASSERT_FALSE(std::filesystem::exists(filePath.parent_path()));

    OrderRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(OrderRepositoryTests, Add_ThenGetAll_ContainsAddedOrder) {
    OrderRepository repository(MakeMissingFilePath("Order_Add_ThenGetAll_ContainsAddedOrder"));
    Order order{"ORD-20260416-0001", "S-001", "Alice", 10, OrderStatus::RESERVED, "2026-04-16T09:00:00"};

    repository.Add(order);

    std::vector<Order> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].orderId, "ORD-20260416-0001");
    EXPECT_EQ(all[0].customerName, "Alice");
    EXPECT_EQ(all[0].quantity, 10);
    EXPECT_EQ(all[0].status, OrderStatus::RESERVED);
}

TEST(OrderRepositoryTests, Add_ThenFindById_ReturnsOrder) {
    OrderRepository repository(MakeMissingFilePath("Order_Add_ThenFindById_ReturnsOrder"));
    Order order{"ORD-20260416-0002", "S-002", "Bob", 5, OrderStatus::CONFIRMED, "2026-04-16T10:00:00"};

    repository.Add(order);

    std::optional<Order> found = repository.FindById("ORD-20260416-0002");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->customerName, "Bob");
    EXPECT_EQ(found->status, OrderStatus::CONFIRMED);
}

TEST(OrderRepositoryTests, Add_DuplicateOrderId_Throws) {
    OrderRepository repository(MakeMissingFilePath("Order_Add_DuplicateOrderId_Throws"));
    repository.Add(Order{"ORD-20260416-0003", "S-001", "Carol", 1, OrderStatus::RESERVED, "2026-04-16T11:00:00"});

    EXPECT_THROW(
        repository.Add(Order{"ORD-20260416-0003", "S-002", "Dave", 2, OrderStatus::RESERVED, "2026-04-16T12:00:00"}),
        std::invalid_argument);

    std::vector<Order> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].customerName, "Carol");
}

TEST(OrderRepositoryTests, Add_CreatesFileOnDisk) {
    std::filesystem::path filePath = MakeMissingFilePath("Order_Add_CreatesFileOnDisk");
    OrderRepository repository(filePath);

    repository.Add(Order{"ORD-20260416-0004", "S-001", "Eve", 3, OrderStatus::RESERVED, "2026-04-16T13:00:00"});

    ASSERT_TRUE(std::filesystem::exists(filePath));
}
