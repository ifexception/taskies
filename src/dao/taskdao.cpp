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

#include "taskdao.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::DAO
{
TaskDao::TaskDao(const std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "TaskDao", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "TaskDao", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

TaskDao::~TaskDao()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "TaskDao");
}

std::int64_t TaskDao::Create(Model::TaskModel& model)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "TaskDao", "task", "");
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, TaskDao::create.c_str(), static_cast<int>(TaskDao::create.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskDao", TaskDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "billable", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // unique identifier
    if (model.UniqueIdentifier.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            model.UniqueIdentifier.value().c_str(),
            static_cast<int>(model.UniqueIdentifier.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "unique_identifier", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.Hours);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "hours", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // minutes
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.Minutes);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "minutes", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    rc = sqlite3_bind_text(
        stmt, bindIndex++, model.Description.c_str(), static_cast<int>(model.Description.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "description", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.ProjectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "project_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "category_id", 7, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.WorkdayId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "workday_id", 8, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskDao", TaskDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "TaskDao", rowId);

    return rowId;
}

int TaskDao::Update(Model::TaskModel& project)
{
    return 0;
}

int TaskDao::Delete(const std::int64_t projectId)
{
    return 0;
}

const std::string TaskDao::create = "INSERT INTO "
                                    "tasks "
                                    "("
                                    "billable, "
                                    "unique_identifier, "
                                    "hours, "
                                    "minutes, "
                                    "description, "
                                    "project_id, "
                                    "category_id, "
                                    "workday_id "
                                    ") "
                                    "VALUES (?,?,?,?,?,?,?,?)";

const std::string TaskDao::update = "";

const std::string TaskDao::isActive = "";
} // namespace tks::DAO
