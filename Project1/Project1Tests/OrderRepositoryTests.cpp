#include <gtest/gtest.h>

#include <filesystem>

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
