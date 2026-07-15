#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Sample.h"

class SampleRepository {
public:
    explicit SampleRepository(std::filesystem::path filePath) : filePath_(std::move(filePath)) {}

    std::vector<Sample> GetAll() const {
        return LoadFromFile();
    }

    std::optional<Sample> FindById(const std::string& sampleId) const {
        for (const auto& sample : LoadFromFile()) {
            if (sample.sampleId == sampleId) {
                return sample;
            }
        }
        return std::nullopt;
    }

private:
    std::vector<Sample> LoadFromFile() const {
        if (!std::filesystem::exists(filePath_)) {
            return {};
        }

        std::ifstream input(filePath_);
        if (!input) {
            return {};
        }

        nlohmann::json json;
        input >> json;

        std::vector<Sample> samples;
        for (const auto& item : json.at("samples")) {
            samples.push_back(SampleFromJson(item));
        }
        return samples;
    }

    std::filesystem::path filePath_;
};
