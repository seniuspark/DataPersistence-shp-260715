#include <gtest/gtest.h>

#include <filesystem>

#include "SampleRepository.h"

namespace {

std::filesystem::path MakeMissingFilePath(const std::string& testName) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / testName;
    std::filesystem::remove_all(dir);
    return dir / "samples.json";
}

}  // namespace

TEST(SampleRepositoryTests, FileDoesNotExist_GetAllReturnsEmpty) {
    SampleRepository repository(MakeMissingFilePath("FileDoesNotExist_GetAllReturnsEmpty"));

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(SampleRepositoryTests, FileDoesNotExist_FindByIdReturnsNullopt) {
    SampleRepository repository(MakeMissingFilePath("FileDoesNotExist_FindByIdReturnsNullopt"));

    EXPECT_FALSE(repository.FindById("S-001").has_value());
}

TEST(SampleRepositoryTests, ConstructingDoesNotCreateFileImmediately) {
    std::filesystem::path filePath = MakeMissingFilePath("ConstructingDoesNotCreateFileImmediately");

    SampleRepository repository(filePath);

    EXPECT_FALSE(std::filesystem::exists(filePath));
}

TEST(SampleRepositoryTests, DirectoryDoesNotExist_GetAllReturnsEmpty) {
    std::filesystem::path filePath = MakeMissingFilePath("DirectoryDoesNotExist_GetAllReturnsEmpty");
    ASSERT_FALSE(std::filesystem::exists(filePath.parent_path()));

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}
