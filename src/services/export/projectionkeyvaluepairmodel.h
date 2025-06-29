#pragma once

#include <string>

struct ProjectionKeyValuePairModel {
    ProjectionKeyValuePairModel() = delete;
    ProjectionKeyValuePairModel(const std::string& key, const std::string& value);
    ~ProjectionKeyValuePairModel() = default;

    std::string Key;
    std::string Value;
};
