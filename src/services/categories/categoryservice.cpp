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

#include "categoryservice.h"

#include "../../common/logmessages.h"

#include "../../common/messages/sqlitemessages.h"

#include "../../utils/utils.h"

namespace tks::Services
{
CategoryService::CategoryService(const std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : Persistence::PersistenceBase(logger, databaseFilePath)
{
}

CategoryService::~CategoryService() {}

SqliteResult CategoryService::Filter(std::vector<CategoryViewModel>& categories) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoryService::filter.c_str(),
        static_cast<int>(CategoryService::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, CategoryService::filter, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            CategoryViewModel model;

            int columnIndex = 0;

            model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            model.Color = sqlite3_column_int(stmt, columnIndex++);
            model.Billable = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                res = sqlite3_column_text(stmt, columnIndex);
                model.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }
            columnIndex++;

            model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            model.DateModified = sqlite3_column_int(stmt, columnIndex++);
            model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.ProjectId = std::nullopt;
            } else {
                model.ProjectId =
                    std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
            }
            columnIndex++;

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.ProjectDisplayName = std::nullopt;
            } else {
                res = sqlite3_column_text(stmt, columnIndex);
                model.ProjectDisplayName = std::make_optional<std::string>(std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
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
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CategoryService::filter, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, categories.size(), "");

    return SqliteResult::OK();
}

SqliteResult CategoryService::FilterByProjectId(const std::int64_t projectId,
    std::vector<CategoryViewModel>& categories) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoryService::filterByProjectId.c_str(),
        static_cast<int>(CategoryService::filterByProjectId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, CategoryService::filterByProjectId, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex, projectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", 1, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            CategoryViewModel model;

            int columnIndex = 0;

            model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            model.Color = sqlite3_column_int(stmt, columnIndex++);
            model.Billable = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                res = sqlite3_column_text(stmt, columnIndex);
                model.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }
            columnIndex++;

            model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            model.DateModified = sqlite3_column_int(stmt, columnIndex++);
            model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.ProjectId = std::nullopt;
            } else {
                model.ProjectId =
                    std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
            }
            columnIndex++;

            res = sqlite3_column_text(stmt, columnIndex);
            model.ProjectDisplayName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

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
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, CategoryService::filterByProjectId, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, categories.size(), projectId);

    return SqliteResult::OK();
}

SqliteResult CategoryService::GetById(const std::int64_t categoryId,
    CategoryViewModel& category) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoryService::getById.c_str(),
        static_cast<int>(CategoryService::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, CategoryService::getById, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, categoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "category_id", 1, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CategoryService::getById, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    int columnIndex = 0;

    category.CategoryId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    category.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    category.Color = sqlite3_column_int(stmt, columnIndex++);
    category.Billable = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        category.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        category.Description = std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }
    columnIndex++;

    category.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    category.DateModified = sqlite3_column_int(stmt, columnIndex++);
    category.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        category.ProjectId = std::nullopt;
    } else {
        category.ProjectId =
            std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
    }
    columnIndex++;

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        category.ProjectDisplayName = std::nullopt;
    } else {
        res = sqlite3_column_text(stmt, columnIndex);
        category.ProjectDisplayName = std::make_optional<std::string>(std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
    }
    columnIndex++;

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

std::string CategoryService::filter = "SELECT "
                                      "categories.category_id, "
                                      "categories.name, "
                                      "categories.color, "
                                      "categories.billable, "
                                      "categories.description, "
                                      "categories.date_created, "
                                      "categories.date_modified, "
                                      "categories.is_active, "
                                      "categories.project_id, "
                                      "projects.display_name "
                                      "FROM categories "
                                      "LEFT JOIN projects "
                                      "ON categories.project_id = projects.project_id "
                                      "WHERE categories.is_active = 1;";

std::string CategoryService::filterByProjectId = "SELECT "
                                                 "categories.category_id, "
                                                 "categories.name, "
                                                 "categories.color, "
                                                 "categories.billable, "
                                                 "categories.description, "
                                                 "categories.date_created, "
                                                 "categories.date_modified, "
                                                 "categories.is_active, "
                                                 "categories.project_id, "
                                                 "projects.display_name "
                                                 "FROM categories "
                                                 "INNER JOIN projects "
                                                 "ON categories.project_id = projects.project_id "
                                                 "WHERE categories.project_id = ? "
                                                 "AND categories.is_active = 1;";

std::string CategoryService::getById = "SELECT "
                                       "categories.category_id, "
                                       "categories.name, "
                                       "categories.color, "
                                       "categories.billable, "
                                       "categories.description, "
                                       "categories.date_created, "
                                       "categories.date_modified, "
                                       "categories.is_active, "
                                       "categories.project_id, "
                                       "projects.display_name "
                                       "FROM categories "
                                       "LEFT JOIN projects "
                                       "ON categories.project_id = projects.project_id "
                                       "WHERE categories.category_id = ? "
                                       "AND categories.is_active = 1;";
} // namespace tks::Services
