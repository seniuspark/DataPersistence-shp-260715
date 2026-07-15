#include <gtest/gtest.h>
#include "Sample.h"

TEST(SampleSerializationTests, Sample_ToJson_ProducesExpectedFields) {
    Sample sample{"S-001", "웨이퍼 A", 12.5, 0.95, 100};

    nlohmann::json json = ToJson(sample);

    EXPECT_EQ(json.at("sampleId").get<std::string>(), "S-001");
    EXPECT_EQ(json.at("name").get<std::string>(), "웨이퍼 A");
    EXPECT_DOUBLE_EQ(json.at("avgProductionTime").get<double>(), 12.5);
    EXPECT_DOUBLE_EQ(json.at("yield").get<double>(), 0.95);
    EXPECT_EQ(json.at("stock").get<int>(), 100);
}

TEST(SampleSerializationTests, Sample_RoundTrip_PreservesAllFields) {
    Sample original{"S-002", "웨이퍼 B", 8.0, 0.5, 42};

    nlohmann::json json = ToJson(original);
    Sample restored = SampleFromJson(json);

    EXPECT_EQ(restored.sampleId, original.sampleId);
    EXPECT_EQ(restored.name, original.name);
    EXPECT_DOUBLE_EQ(restored.avgProductionTime, original.avgProductionTime);
    EXPECT_DOUBLE_EQ(restored.yield, original.yield);
    EXPECT_EQ(restored.stock, original.stock);
}

TEST(SampleSerializationTests, Sample_RoundTrip_PreservesBoundaryValues) {
    Sample original{"S-003", "웨이퍼 C", 0.0, 0.0, 0};

    Sample restored = SampleFromJson(ToJson(original));

    EXPECT_DOUBLE_EQ(restored.yield, 0.0);
    EXPECT_EQ(restored.stock, 0);

    Sample originalYieldOne{"S-004", "웨이퍼 D", 1.0, 1.0, 1};
    Sample restoredYieldOne = SampleFromJson(ToJson(originalYieldOne));
    EXPECT_DOUBLE_EQ(restoredYieldOne.yield, 1.0);
}

TEST(SampleSerializationTests, SampleList_ToJsonArray_RoundTrip) {
    std::vector<Sample> samples{
        {"S-001", "웨이퍼 A", 12.5, 0.95, 100},
        {"S-002", "웨이퍼 B", 8.0, 0.5, 42},
    };

    nlohmann::json wrapped;
    wrapped["samples"] = nlohmann::json::array();
    for (const auto& sample : samples) {
        wrapped["samples"].push_back(ToJson(sample));
    }

    std::vector<Sample> restored;
    for (const auto& item : wrapped.at("samples")) {
        restored.push_back(SampleFromJson(item));
    }

    ASSERT_EQ(restored.size(), samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        EXPECT_EQ(restored[i].sampleId, samples[i].sampleId);
        EXPECT_EQ(restored[i].name, samples[i].name);
        EXPECT_DOUBLE_EQ(restored[i].avgProductionTime, samples[i].avgProductionTime);
        EXPECT_DOUBLE_EQ(restored[i].yield, samples[i].yield);
        EXPECT_EQ(restored[i].stock, samples[i].stock);
    }
}
