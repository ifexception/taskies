// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include "categorypersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
CategoryPersistence::CategoryPersistence(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "CategoryPersistence", databaseFilePath);
    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "CategoryPersistence", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryPersistence", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryPersistence", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryPersistence", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryPersistence", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryPersistence", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

CategoryPersistence::~CategoryPersistence()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "CategoryPersistence");
}

int CategoryPersistence::Filter(const std::string& searchTerm, std::vector<Model::CategoryModel>& categories)
{
    pLogger->info(LogMessage::InfoBeginFilterEntities, "CategoryPersistence", "categories", searchTerm);

    sqlite3_stmt* stmt = nullptr;
    auto formattedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    int rc = sqlite3_prepare_v2(
        pDb, CategoryPersistence::filter.c_str(), static_cast<int>(CategoryPersistence::filter.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "CategoryPersistence", CategoryPersistence::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;
    // category name
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category description
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::CategoryModel model;
            int columnIndex = 0;

            model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            model.Color = sqlite3_column_int(stmt, columnIndex++);
            model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                model.Description =
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }
            columnIndex++;
            model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            model.DateModified = sqlite3_column_int(stmt, columnIndex++);
            model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.ProjectId = std::nullopt;
            } else {
                model.ProjectId = std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
            }
            columnIndex++;

            categories.push_back(model);
            break;
        }
        case SQLITE_DONE:
            rc = SQLITE_DONE;
            done = true;
            break;
        default:
            break;
        }
    }
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "CategoryPersistence", CategoryPersistence::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndFilterEntities, "CategoryPersistence", categories.size(), searchTerm);

    return 0;
}

int CategoryPersistence::GetById(const std::int64_t categoryId, Model::CategoryModel& model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "CategoryPersistence", "category", categoryId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, CategoryPersistence::getById.c_str(), static_cast<int>(CategoryPersistence::getById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "CategoryPersistence", CategoryPersistence::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, categoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "category_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "CategoryPersistence", CategoryPersistence::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    model.Color = sqlite3_column_int(stmt, columnIndex++);
    model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        model.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }
    columnIndex++;
    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    model.DateModified = sqlite3_column_int(stmt, columnIndex++);
    model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.ProjectId = std::nullopt;
    } else {
        model.ProjectId = std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
    }
    columnIndex++;

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "CategoryPersistence", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "CategoryPersistence", categoryId);

    return 0;
}

std::int64_t CategoryPersistence::Create(Model::CategoryModel& category)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "CategoryPersistence", "category", category.Name);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, CategoryPersistence::create.c_str(), static_cast<int>(CategoryPersistence::create.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "CategoryPersistence", CategoryPersistence::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc = sqlite3_bind_text(
        stmt, bindIndex++, category.Name.c_str(), static_cast<int>(category.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, static_cast<int>(category.Color));
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "color", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, category.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "billable", 3, rc, err);
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
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "description", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (category.ProjectId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, category.ProjectId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "project_id", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "CategoryPersistence", CategoryPersistence::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "CategoryPersistence", rowId);

    return rowId;
}

int CategoryPersistence::Update(Model::CategoryModel& model)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "CategoryPersistence", "category", model.CategoryId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, CategoryPersistence::update.c_str(), static_cast<int>(CategoryPersistence::update.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "CategoryPersistence", CategoryPersistence::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // name
    rc =
        sqlite3_bind_text(stmt, bindIndex++, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // color
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Color);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "color", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "billable", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    if (model.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            model.Description.value().c_str(),
            static_cast<int>(model.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "description", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // date_modified
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "date_modified", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project_id
    if (model.ProjectId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, model.ProjectId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "project_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category_id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "category_id", 7, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "CategoryPersistence", CategoryPersistence::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "Category", model.CategoryId);

    return 0;
}

int CategoryPersistence::Delete(const std::int64_t categoryId)
{
    pLogger->info(LogMessage::InfoBeginDeleteEntity, "CategoryPersistence", "category", categoryId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, CategoryPersistence::isActive.c_str(), static_cast<int>(CategoryPersistence::isActive.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "CategoryPersistence", CategoryPersistence::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;

    rc = sqlite3_bind_int64(stmt, bindIdx++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "date_modified", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIdx++, categoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryPersistence", "category_id", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "CategoryPersistence", CategoryPersistence::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndDeleteEntity, "CategoryPersistence", categoryId);

    return 0;
}

std::int64_t CategoryPersistence::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string CategoryPersistence::filter = "SELECT "
                                        "category_id, "
                                        "name, "
                                        "color, "
                                        "description, "
                                        "date_created, "
                                        "date_modified, "
                                        "is_active, "
                                        "project_id "
                                        "FROM categories "
                                        "WHERE is_active = 1 "
                                        "AND (name LIKE ? "
                                        "OR description LIKE ?);";

const std::string CategoryPersistence::getById = "SELECT "
                                         "category_id, "
                                         "name, "
                                         "color, "
                                         "billable, "
                                         "description, "
                                         "date_created, "
                                         "date_modified, "
                                         "is_active, "
                                         "project_id "
                                         "FROM categories "
                                         "WHERE category_id = ? "
                                         "AND is_active = 1;";

const std::string CategoryPersistence::create = "INSERT INTO "
                                        "categories "
                                        "("
                                        "name, "
                                        "color, "
                                        "billable, "
                                        "description, "
                                        "project_id "
                                        ") "
                                        "VALUES (?, ?, ?, ?, ?)";

const std::string CategoryPersistence::update = "UPDATE categories "
                                        "SET "
                                        "name = ?, "
                                        "color = ?, "
                                        "billable = ?, "
                                        "description = ?, "
                                        "date_modified = ?, "
                                        "project_id = ? "
                                        "WHERE category_id = ?;";

const std::string CategoryPersistence::isActive = "UPDATE categories "
                                          "SET "
                                          "is_active = 0, "
                                          "date_modified = ? "
                                          "WHERE category_id = ?;";
} // namespace tks::Persistence
