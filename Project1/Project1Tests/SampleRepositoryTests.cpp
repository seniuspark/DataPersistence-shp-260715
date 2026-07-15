#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

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

TEST(SampleRepositoryTests, Add_ThenGetAll_ContainsAddedSample) {
    SampleRepository repository(MakeMissingFilePath("Add_ThenGetAll_ContainsAddedSample"));
    Sample sample{"S-001", "TestSample", 12.5, 0.95, 100};

    repository.Add(sample);

    std::vector<Sample> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].sampleId, "S-001");
    EXPECT_EQ(all[0].name, "TestSample");
    EXPECT_DOUBLE_EQ(all[0].avgProductionTime, 12.5);
    EXPECT_DOUBLE_EQ(all[0].yield, 0.95);
    EXPECT_EQ(all[0].stock, 100);
}

TEST(SampleRepositoryTests, Add_ThenFindById_ReturnsSample) {
    SampleRepository repository(MakeMissingFilePath("Add_ThenFindById_ReturnsSample"));
    Sample sample{"S-002", "AnotherSample", 5.0, 0.8, 10};

    repository.Add(sample);

    std::optional<Sample> found = repository.FindById("S-002");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "AnotherSample");
}

TEST(SampleRepositoryTests, Add_DuplicateSampleId_Throws) {
    SampleRepository repository(MakeMissingFilePath("Add_DuplicateSampleId_Throws"));
    repository.Add(Sample{"S-003", "First", 1.0, 1.0, 1});

    EXPECT_THROW(repository.Add(Sample{"S-003", "Second", 2.0, 0.5, 2}), std::invalid_argument);

    std::vector<Sample> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].name, "First");
}

TEST(SampleRepositoryTests, Add_CreatesFileOnDisk) {
    std::filesystem::path filePath = MakeMissingFilePath("Add_CreatesFileOnDisk");
    SampleRepository repository(filePath);

    repository.Add(Sample{"S-004", "DiskSample", 3.0, 0.9, 5});

    ASSERT_TRUE(std::filesystem::exists(filePath));
    std::ifstream input(filePath);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("S-004"), std::string::npos);
}

TEST(SampleRepositoryTests, Add_CreatesMissingParentDirectory) {
    std::filesystem::path filePath = MakeMissingFilePath("Add_CreatesMissingParentDirectory");
    ASSERT_FALSE(std::filesystem::exists(filePath.parent_path()));
    SampleRepository repository(filePath);

    repository.Add(Sample{"S-005", "DirSample", 1.0, 1.0, 1});

    EXPECT_TRUE(std::filesystem::exists(filePath.parent_path()));
    EXPECT_TRUE(std::filesystem::exists(filePath));
}

TEST(SampleRepositoryTests, GetAll_ReturnsCopiesNotInternalReference) {
    SampleRepository repository(MakeMissingFilePath("GetAll_ReturnsCopiesNotInternalReference"));
    repository.Add(Sample{"S-006", "Original", 1.0, 1.0, 1});

    std::vector<Sample> all = repository.GetAll();
    all[0].name = "Mutated";
    all.push_back(Sample{"S-999", "Extra", 1.0, 1.0, 1});

    std::vector<Sample> reloaded = repository.GetAll();
    ASSERT_EQ(reloaded.size(), 1u);
    EXPECT_EQ(reloaded[0].name, "Original");
}
