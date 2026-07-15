#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "OrderRepository.h"
#include "SampleRepository.h"

namespace {

std::filesystem::path MakeTestDir(const std::string& testName) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "Project1Tests" / "CorruptedJson" / testName;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

void WriteRawFile(const std::filesystem::path& filePath, const std::string& content) {
    std::ofstream output(filePath);
    output << content;
}

}  // namespace

TEST(CorruptedJsonTests, SampleRepository_CorruptedJsonFile_GetAllReturnsEmptyAndPreservesFile) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_CorruptedJsonFile_GetAllReturnsEmptyAndPreservesFile") / "samples.json";
    const std::string corrupted = "{\"samples\": [ { \"sampleId\": \"S-001\" ";
    WriteRawFile(filePath, corrupted);

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());

    std::ifstream input(filePath);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, corrupted);
}

TEST(CorruptedJsonTests, SampleRepository_JsonWithMissingRequiredField_SkipsOnlyThatItem) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_JsonWithMissingRequiredField_SkipsOnlyThatItem") / "samples.json";
    const std::string json = R"({"samples": [
        {"sampleId": "S-801", "avgProductionTime": 1.0, "yield": 1.0, "stock": 1},
        {"sampleId": "S-802", "name": "Valid", "avgProductionTime": 2.0, "yield": 1.0, "stock": 2}
    ]})";
    WriteRawFile(filePath, json);

    SampleRepository repository(filePath);
    std::vector<Sample> all = repository.GetAll();

    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].sampleId, "S-802");
}

TEST(CorruptedJsonTests, SampleRepository_JsonWithUnknownTopLevelStructure_ArrayAtTopLevel_ReturnsEmpty) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_JsonWithUnknownTopLevelStructure_ArrayAtTopLevel_ReturnsEmpty") / "samples.json";
    WriteRawFile(filePath, R"([{"sampleId": "S-901"}])");

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(CorruptedJsonTests, SampleRepository_JsonWithUnknownTopLevelStructure_MissingSamplesKey_ReturnsEmpty) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_JsonWithUnknownTopLevelStructure_MissingSamplesKey_ReturnsEmpty") / "samples.json";
    WriteRawFile(filePath, R"({"unexpected": []})");

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(CorruptedJsonTests, OrderRepository_CorruptedJsonFile_SamePolicyAsSample) {
    std::filesystem::path filePath = MakeTestDir("OrderRepository_CorruptedJsonFile_SamePolicyAsSample") / "orders.json";
    const std::string corrupted = "{\"orders\": [ { \"orderId\": \"ORD-BROKEN\" ";
    WriteRawFile(filePath, corrupted);

    OrderRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());

    std::ifstream input(filePath);
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, corrupted);
}

TEST(CorruptedJsonTests, SampleRepository_EmptyFileContent_GetAllReturnsEmpty) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_EmptyFileContent_GetAllReturnsEmpty") / "samples.json";
    WriteRawFile(filePath, "");

    SampleRepository repository(filePath);

    EXPECT_TRUE(repository.GetAll().empty());
}

TEST(CorruptedJsonTests, SampleRepository_CorruptedFile_AddOverwritesWithValidData) {
    std::filesystem::path filePath = MakeTestDir("SampleRepository_CorruptedFile_AddOverwritesWithValidData") / "samples.json";
    WriteRawFile(filePath, "{ not valid json");

    SampleRepository repository(filePath);
    repository.Add(Sample{"S-950", "Recovered", 1.0, 1.0, 1});

    std::vector<Sample> all = repository.GetAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].sampleId, "S-950");
}
