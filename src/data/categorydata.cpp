// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contact:
//     szymonwelgus at gmail dot com

#include "categorydata.h"

#include "../core/environment.h"
#include "../common/constants.h"
#include "../utils/utils.h"

namespace tks::Data
{
CategoryData::CategoryData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger) {}
CategoryData::~CategoryData() {}
std::int64_t CategoryData::Create(Model::CategoryModel& client)
{
    return std::int64_t();
}
int CategoryData::Filter(const std::string& searchTerm, std::vector<Model::CategoryModel>& clients)
{
    return 0;
}
int CategoryData::GetById(const std::int64_t clientId, Model::CategoryModel& model)
{
    return 0;
}
int CategoryData::Update(Model::CategoryModel& client)
{
    return 0;
}
int CategoryData::Delete(const std::int64_t clientId)
{
    return 0;
}
std::int64_t CategoryData::GetLastInsertId() const
{
    return std::int64_t();
}

const std::string CategoryData::create = "INSERT INTO "
                                         "categories "
                                         "("
                                         "name, "
                                         "color, "
                                         "billable, "
                                         "description "
                                         ") "
                                         "VALUES (?, ?, ?, ?)";

const std::string CategoryData::filter = "SELECT "
                                         "categories.category_id, "
                                         "categories.name, "
                                         "categories.color, "
                                         "categories.date_created, "
                                         "categories.date_modified, "
                                         "categories.is_active "
                                         "FROM categories "
                                         "WHERE categories.is_active = 1";

const std::string CategoryData::getById = "SELECT "
                                          "categories.category_id, "
                                          "categories.name, "
                                          "categories.color, "
                                          "categories.billable, "
                                          "categories.description, "
                                          "categories.date_created, "
                                          "categories.date_modified, "
                                          "categories.is_active "
                                          "FROM categories "
                                          "WHERE categories.is_active = 1";

const std::string CategoryData::update = "UPDATE categories "
                                         "SET "
                                         "name = ?, "
                                         "color = ?, "
                                         "billable = ?, "
                                         "description = ?, "
                                         "date_modified = ? "
                                         "WHERE category_id = ?";

const std::string CategoryData::isActive = "UPDATE categories "
                                           "SET "
                                           "is_active = 0, "
                                           "date_modified = ? "
                                           "WHERE category_id = ?";
} // namespace tks::Data
