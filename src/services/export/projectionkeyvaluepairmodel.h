#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Row {
    std::vector<std::string> Values;
};

struct Data {
    std::vector<std::string> Headers;
    std::unordered_map<std::int64_t, Row> Rows;
};
