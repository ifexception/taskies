// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include "categoriespersistence.h"

#include "../common/logmessages.h"

#include "../common/messages/sqlitemessages.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
CategoriesPersistence::CategoriesPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : PersistenceBase(logger, databaseFilePath)
{
}

CategoriesPersistence::~CategoriesPersistence() {}

SqliteResult CategoriesPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::CategoryModel>& categoryModels) const
{
    sqlite3_stmt* stmt = nullptr;

    auto formattedSearchTerm = Utils::FormatSqlSearchTerm(searchTerm);

    int rc = sqlite3_prepare_v2(pDb,
        CategoriesPersistence::filter.c_str(),
        static_cast<int>(CategoriesPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, CategoriesPersistence::filter, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // category name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // category description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            Model::CategoryModel categoryModel;
            int columnIndex = 0;

            categoryModel.CategoryId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            categoryModel.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            categoryModel.Color = sqlite3_column_int(stmt, columnIndex++);
            categoryModel.Billable = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                categoryModel.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                categoryModel.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }

            columnIndex++;

            categoryModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            categoryModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            categoryModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                categoryModel.ProjectId = std::nullopt;
            } else {
                categoryModel.ProjectId =
                    std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
            }

            categoryModels.push_back(categoryModel);
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
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CategoriesPersistence::filter, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, categoryModels.size(), searchTerm);

    return SqliteResult::OK();
}

SqliteResult CategoriesPersistence::GetById(const std::int64_t categoryId,
    Model::CategoryModel& categoryModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoriesPersistence::getById.c_str(),
        static_cast<int>(CategoriesPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, CategoriesPersistence::getById, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, categoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "category_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CategoriesPersistence::getById, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    int columnIndex = 0;

    categoryModel.CategoryId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    categoryModel.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    categoryModel.Color = sqlite3_column_int(stmt, columnIndex++);
    categoryModel.Billable = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        categoryModel.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        categoryModel.Description = std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }

    columnIndex++;

    categoryModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    categoryModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    categoryModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        categoryModel.ProjectId = std::nullopt;
    } else {
        categoryModel.ProjectId =
            std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::StepStatementReturnedMultipleRowsMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "categories", categoryId);

    return SqliteResult::OK();
}

SqliteResult CategoriesPersistence::Create(std::int64_t& categoryId,
    const Model::CategoryModel& category) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoriesPersistence::create.c_str(),
        static_cast<int>(CategoriesPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, CategoriesPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(stmt,
        bindIndex,
        category.Name.c_str(),
        static_cast<int>(category.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    rc = sqlite3_bind_int(stmt, bindIndex, static_cast<int>(category.Color));

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "color", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    rc = sqlite3_bind_int(stmt, bindIndex, category.Billable);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "billable", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

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
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    if (category.ProjectId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, category.ProjectId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CategoriesPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    categoryId = rowId;
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "category", rowId);

    return SqliteResult::OK();
}

SqliteResult CategoriesPersistence::Update(const Model::CategoryModel& categoryModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoriesPersistence::update.c_str(),
        static_cast<int>(CategoriesPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, CategoriesPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        categoryModel.Name.c_str(),
        static_cast<int>(categoryModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // color
    rc = sqlite3_bind_int(stmt, bindIndex, categoryModel.Color);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "color", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex, categoryModel.Billable);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "billable", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // description
    if (categoryModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            categoryModel.Description.value().c_str(),
            static_cast<int>(categoryModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // date_modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // project_id
    if (categoryModel.ProjectId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, categoryModel.ProjectId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // category_id
    rc = sqlite3_bind_int64(stmt, bindIndex, categoryModel.CategoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "category_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CategoriesPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityUpdated, "category", categoryModel.CategoryId);

    return SqliteResult::OK();
}

SqliteResult CategoriesPersistence::Delete(const std::int64_t categoryId) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoriesPersistence::isActive.c_str(),
        static_cast<int>(CategoriesPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, CategoriesPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, categoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "category_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CategoriesPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityDeleted, "category", categoryId);

    return SqliteResult::OK();
}

std::string CategoriesPersistence::filter = "SELECT "
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

std::string CategoriesPersistence::getById = "SELECT "
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

std::string CategoriesPersistence::create = "INSERT INTO "
                                            "categories "
                                            "("
                                            "name, "
                                            "color, "
                                            "billable, "
                                            "description, "
                                            "project_id "
                                            ") "
                                            "VALUES (?, ?, ?, ?, ?)";

std::string CategoriesPersistence::update = "UPDATE categories "
                                            "SET "
                                            "name = ?, "
                                            "color = ?, "
                                            "billable = ?, "
                                            "description = ?, "
                                            "date_modified = ?, "
                                            "project_id = ? "
                                            "WHERE category_id = ?;";

std::string CategoriesPersistence::isActive = "UPDATE categories "
                                              "SET "
                                              "is_active = 0, "
                                              "date_modified = ? "
                                              "WHERE category_id = ?;";
} // namespace tks::Persistence
