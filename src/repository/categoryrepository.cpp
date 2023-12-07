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

#include "categoryrepository.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::repos
{
CategoryRepository::CategoryRepository(const std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "CategoryRepository", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "CategoryRepository", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "CategoryRepository", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "CategoryRepository", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "CategoryRepository", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryRepository", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "CategoryRepository", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

CategoryRepository::~CategoryRepository()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "CategoryRepository");
}

int CategoryRepository::Filter(std::vector<CategoryRepositoryModel>& categories)
{
    pLogger->info(LogMessage::InfoBeginFilterEntities, "CategoryRepository", "categories", "n/a");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, CategoryRepository::filter.c_str(), static_cast<int>(CategoryRepository::filter.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "CategoryRepository", CategoryRepository::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            CategoryRepositoryModel model;
            int columnIndex = 0;

            model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            model.Color = sqlite3_column_int(stmt, columnIndex++);
            model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                res = sqlite3_column_text(stmt, columnIndex);
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
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.ProjectDisplayName = std::nullopt;
            } else {
                res = sqlite3_column_text(stmt, columnIndex);
                model.ProjectDisplayName = std::make_optional<std::string>(
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
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
        pLogger->error(LogMessage::ExecStepTemplate, "CategoryRepository", CategoryRepository::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndFilterEntities, "CategoryRepository", categories.size(), "n/a");

    return 0;
}

int CategoryRepository::FilterByProjectId(const std::int64_t projectId,
    std::vector<CategoryRepositoryModel>& categories)
{
    pLogger->info(LogMessage::InfoBeginFilterEntities, "CategoryRepository", "categories", projectId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        CategoryRepository::filterByProjectId.c_str(),
        static_cast<int>(CategoryRepository::filterByProjectId.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::PrepareStatementTemplate, "CategoryRepository", CategoryRepository::filterByProjectId, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;
    // project id
    rc = sqlite3_bind_int64(stmt, bindIdx, projectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "CategoryRepository", "project_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            CategoryRepositoryModel model;
            int columnIndex = 0;

            model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            model.Color = sqlite3_column_int(stmt, columnIndex++);
            model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                res = sqlite3_column_text(stmt, columnIndex);
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

            res = sqlite3_column_text(stmt, columnIndex);
            model.ProjectDisplayName =
                std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

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
        pLogger->error(
            LogMessage::ExecStepTemplate, "CategoryRepository", CategoryRepository::filterByProjectId, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndFilterEntities, "CategoryRepository", categories.size(), projectId);

    return 0;
}

const std::string CategoryRepository::filter = "SELECT "
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

const std::string CategoryRepository::filterByProjectId = "SELECT "
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
} // namespace tks::repos
