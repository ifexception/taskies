#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// -- ATTRIBUTES

constexpr int ATTRIBUTE_PROP_INDEX_TASKID = 0;
constexpr int ATTRIBUTE_PROP_INDEX_NAME = 1;
constexpr int ATTRIBUTE_PROP_INDEX_VALUE = 2;

struct AttributeHeaderValueModel {
    AttributeHeaderValueModel()
        : Header("")
        , Value("")
    {
    }
    AttributeHeaderValueModel(std::string header, std::string value)
        : Header(header)
        , Value(value)
    {
    }

    std::string Header;
    std::string Value;
};

struct AttributeValueModel {
    AttributeValueModel()
        : HeaderValueModels()
    {
    }

    std::vector<AttributeHeaderValueModel> HeaderValueModels;
};

struct Row {
    std::vector<std::string> Values;
};

struct Data {
    std::vector<std::string> Headers;
    std::unordered_map<std::int64_t, Row> Rows;
};
