#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

constexpr int ATTRIBUTE_PROP_INDEX_TASKID = 0;
constexpr int ATTRIBUTE_PROP_INDEX_NAME = 1;
constexpr int ATTRIBUTE_PROP_INDEX_VALUE = 2;

struct HeaderValuePair {
    HeaderValuePair()
        : Header("")
        , Value("")
    {
    }
    HeaderValuePair(std::string header, std::string value)
        : Header(header)
        , Value(value)
    {
    }

    std::string Header;
    std::string Value;
};

struct HeaderValueRow {
    HeaderValueRow()
        : HeaderValuePairs()
    {
    }

    std::vector<HeaderValuePair> HeaderValuePairs;
};

struct Row {
    std::vector<std::string> Values;
};

struct Data {
    std::vector<std::string> Headers;
    std::unordered_map<std::int64_t, Row> Rows;
};
