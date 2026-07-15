#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
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

    void Add(const Sample& sample) {
        std::vector<Sample> samples = LoadFromFile();
        for (const auto& existing : samples) {
            if (existing.sampleId == sample.sampleId) {
                throw std::invalid_argument("Sample with sampleId already exists: " + sample.sampleId);
            }
        }
        samples.push_back(sample);
        SaveAll(samples);
    }

private:
    void SaveAll(const std::vector<Sample>& samples) const {
        std::filesystem::create_directories(filePath_.parent_path());

        nlohmann::json json;
        json["samples"] = nlohmann::json::array();
        for (const auto& sample : samples) {
            json["samples"].push_back(ToJson(sample));
        }

        std::ofstream output(filePath_);
        output << json.dump(2);
    }

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
