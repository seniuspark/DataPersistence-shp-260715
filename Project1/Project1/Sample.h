#pragma once

#include <string>

#include <nlohmann/json.hpp>

struct Sample {
    std::string sampleId;
    std::string name;
    double avgProductionTime;
    double yield;
    int stock;
};

inline nlohmann::json ToJson(const Sample& sample) {
    return nlohmann::json{
        {"sampleId", sample.sampleId},
        {"name", sample.name},
        {"avgProductionTime", sample.avgProductionTime},
        {"yield", sample.yield},
        {"stock", sample.stock},
    };
}

inline Sample SampleFromJson(const nlohmann::json& json) {
    return Sample{
        json.at("sampleId").get<std::string>(),
        json.at("name").get<std::string>(),
        json.at("avgProductionTime").get<double>(),
        json.at("yield").get<double>(),
        json.at("stock").get<int>(),
    };
}
