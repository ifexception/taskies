#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ProjectionKeyValuePairModel {
    ProjectionKeyValuePairModel() = delete;
    ProjectionKeyValuePairModel(const std::string& key, const std::string& value);
    ~ProjectionKeyValuePairModel() = default;

    std::string Key;
    std::string Value;
};

struct ProjectionListModel {
    ProjectionListModel();
    ~ProjectionListModel() = default;

    std::int64_t TaskId;
    std::vector<ProjectionKeyValuePairModel> ProjectionKeyValuePairModels;
};
