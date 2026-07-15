#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
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

    bool Update(const Sample& sample) {
        std::vector<Sample> samples = LoadFromFile();
        for (auto& existing : samples) {
            if (existing.sampleId == sample.sampleId) {
                existing = sample;
                SaveAll(samples);
                return true;
            }
        }
        return false;
    }

    bool Delete(const std::string& sampleId) {
        std::vector<Sample> samples = LoadFromFile();
        auto it = std::find_if(samples.begin(), samples.end(),
                                [&](const Sample& sample) { return sample.sampleId == sampleId; });
        if (it == samples.end()) {
            return false;
        }
        samples.erase(it);
        SaveAll(samples);
        return true;
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

        std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (content.empty()) {
            return {};
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(content);
        } catch (const nlohmann::json::parse_error&) {
            return {};
        }

        if (!json.is_object() || !json.contains("samples") || !json.at("samples").is_array()) {
            return {};
        }

        std::vector<Sample> samples;
        for (const auto& item : json.at("samples")) {
            try {
                samples.push_back(SampleFromJson(item));
            } catch (const nlohmann::json::exception& e) {
                std::cerr << "Skipping invalid sample entry: " << e.what() << std::endl;
            }
        }
        return samples;
    }

    std::filesystem::path filePath_;
};
