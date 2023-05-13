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
CategoryData::CategoryData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
    , pDb(nullptr)
{
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "CategoryData",
            pEnv->GetDatabaseName(),
            pEnv->GetDatabasePath().string(),
            rc,
            std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryData", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryData", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryData", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryData", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryData", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

CategoryData::~CategoryData()
{
    sqlite3_close(pDb);
}

std::int64_t CategoryData::Create(Model::CategoryModel& category)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, CategoryData::create.c_str(), static_cast<int>(CategoryData::create.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "CategoryData", CategoryData::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc = sqlite3_bind_text(
        stmt, bindIndex++, category.Name.c_str(), static_cast<int>(category.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryData", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, static_cast<int>(category.Color));
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryData", "color", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, category.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryData", "billable", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (category.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            category.Description.value().c_str(),
            static_cast<int>(category.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryData", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "CategoryData", CategoryData::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    return rowId;
}

int CategoryData::Filter(const std::string& searchTerm, std::vector<Model::CategoryModel>& categories)
{
    return 0;
}

int CategoryData::GetById(const std::int64_t categoryId, Model::CategoryModel& model)
{
    return 0;
}

int CategoryData::Update(Model::CategoryModel& category)
{
    return 0;
}

int CategoryData::Delete(const std::int64_t categoryId)
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
