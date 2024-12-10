// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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

#include "taskpersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

// TODO: use a static_assert to check bindIndex with number of expected parameters

namespace tks::Persistence
{
TaskPersistence::TaskPersistence(const std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "TaskPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "TaskPersistence", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskPersistence", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskPersistence", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskPersistence", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskPersistence", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskPersistence", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

TaskPersistence::~TaskPersistence()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "TaskPersistence");
}

int TaskPersistence::GetById(const std::int64_t taskId, Model::TaskModel& model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskPersistence", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc =
        sqlite3_prepare_v2(pDb, TaskPersistence::getById.c_str(), static_cast<int>(TaskPersistence::getById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", TaskPersistence::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "task_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", TaskPersistence::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    model.TaskId = sqlite3_column_int64(stmt, columnIndex++);
    model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.UniqueIdentifier = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        model.UniqueIdentifier = std::make_optional(
            std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
    }
    columnIndex++;

    model.Hours = sqlite3_column_int(stmt, columnIndex++);
    model.Minutes = sqlite3_column_int(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    model.DateModified = sqlite3_column_int(stmt, columnIndex++);
    model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    model.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
    model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
    model.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TaskPersistence", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskPersistence", taskId);

    return 0;
}

std::int64_t TaskPersistence::Create(Model::TaskModel& model)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "TaskPersistence", "task", "");
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, TaskPersistence::create.c_str(), static_cast<int>(TaskPersistence::create.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", TaskPersistence::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "billable", 1, rc, err);
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
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "unique_identifier", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.Hours);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "hours", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // minutes
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.Minutes);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "minutes", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    rc = sqlite3_bind_text(
        stmt, bindIndex++, model.Description.c_str(), static_cast<int>(model.Description.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "description", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.ProjectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "project_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "category_id", 7, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.WorkdayId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "workday_id", 8, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", TaskPersistence::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "TaskPersistence", rowId);

    return rowId;
}

int TaskPersistence::Update(Model::TaskModel& task)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "TaskPersistence", "task", task.TaskId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, TaskPersistence::update.c_str(), static_cast<int>(TaskPersistence::update.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", TaskPersistence::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, task.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "billable", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // unique identifier
    if (task.UniqueIdentifier.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            task.UniqueIdentifier.value().c_str(),
            static_cast<int>(task.UniqueIdentifier.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "unique_identifier", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int(stmt, bindIndex++, task.Hours);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "hours", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // minutes
    rc = sqlite3_bind_int(stmt, bindIndex++, task.Minutes);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "minutes", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    rc = sqlite3_bind_text(
        stmt, bindIndex++, task.Description.c_str(), static_cast<int>(task.Description.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "description", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.ProjectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "project_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "category_id", 7, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.WorkdayId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "workday_id", 8, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "date_modified", 9, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // task id
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.TaskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "task_id", 10, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", TaskPersistence::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndUpdateEntity, "TaskPersistence", task.TaskId);

    return 0;
}

int TaskPersistence::Delete(const std::int64_t taskId)
{
    pLogger->info(LogMessage::InfoBeginDeleteEntity, "TaskPersistence", "task", taskId);
    sqlite3_stmt* stmt = nullptr;

    int rc =
        sqlite3_prepare_v2(pDb, TaskPersistence::isActive.c_str(), static_cast<int>(TaskPersistence::isActive.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", TaskPersistence::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;

    rc = sqlite3_bind_int64(stmt, bindIdx++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "date_modified", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIdx++, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "task_id", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", TaskPersistence::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndDeleteEntity, "TaskPersistence", taskId);

    return 0;
}

int TaskPersistence::GetDescriptionById(const std::int64_t taskId, std::string& description)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskPersistence", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, TaskPersistence::getDescriptionById.c_str(), static_cast<int>(TaskPersistence::getDescriptionById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", TaskPersistence::getDescriptionById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "task_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", TaskPersistence::getDescriptionById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TaskPersistence", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskPersistence", taskId);

    return 0;
}

int TaskPersistence::IsDeleted(const std::int64_t taskId, bool& value)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskPersistence", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, TaskPersistence::isDeleted.c_str(), static_cast<int>(TaskPersistence::isDeleted.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", TaskPersistence::isDeleted, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "task_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", TaskPersistence::isDeleted, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    value = !!sqlite3_column_int(stmt, columnIndex);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TaskPersistence", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskPersistence", taskId);

    return 0;
}

int TaskPersistence::GetTaskDurationsForDateRange(const std::string& startDate,
    const std::string& endDate,
    TaskDurationType type,
    std::vector<Model::TaskDurationModel>& models)
{
    auto paramFmt = startDate + "|" + endDate;
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskPersistence", "task", paramFmt);

    // clang-format off
    std::string sql = type == TaskDurationType::Default
        ? TaskPersistence::getAllHoursForDateRange
        : TaskPersistence::getBillableHoursForDateRange;

    std::size_t sqlSize = type == TaskDurationType::Default
        ? TaskPersistence::getAllHoursForDateRange.size()
        : TaskPersistence::getBillableHoursForDateRange.size();
    // clang-format on

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sqlSize), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", sql, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(stmt, bindIndex++, startDate.c_str(), static_cast<int>(startDate.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "date", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, bindIndex++, endDate.c_str(), static_cast<int>(endDate.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "date", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            Model::TaskDurationModel model;
            rc = SQLITE_ROW;
            int columnIndex = 0;

            model.Hours = sqlite3_column_int(stmt, columnIndex++);
            model.Minutes = sqlite3_column_int(stmt, columnIndex++);

            models.push_back(model);
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
        pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", sql, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskPersistence", paramFmt);

    return 0;
}

int TaskPersistence::GetHoursForDateRangeGroupedByDate(const std::vector<std::string>& dates,
    std::map<std::string, std::vector<Model::TaskDurationModel>>& durationsGroupedByDate)
{
    for (const auto& date : dates) {
        pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskPersistence", "task", date);

        sqlite3_stmt* stmt = nullptr;
        std::vector<Model::TaskDurationModel> models;

        int rc = sqlite3_prepare_v2(pDb,
            TaskPersistence::getAllHoursForDate.c_str(),
            static_cast<int>(TaskPersistence::getAllHoursForDate.size()),
            &stmt,
            nullptr);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessage::PrepareStatementTemplate, "TaskPersistence", TaskPersistence::getAllHoursForDate, rc, err);
            sqlite3_finalize(stmt);
            return -1;
        }

        int bindIndex = 1;

        rc = sqlite3_bind_text(stmt, bindIndex++, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessage::BindParameterTemplate, "TaskPersistence", "date", 1, rc, err);
            sqlite3_finalize(stmt);
            return -1;
        }

        bool done = false;
        while (!done) {
            switch (sqlite3_step(stmt)) {
            case SQLITE_ROW: {
                Model::TaskDurationModel model{};

                rc = SQLITE_ROW;
                int columnIndex = 0;

                model.Hours = sqlite3_column_int(stmt, columnIndex++);
                model.Minutes = sqlite3_column_int(stmt, columnIndex++);

                models.push_back(model);
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
            pLogger->error(LogMessage::ExecStepTemplate, "TaskPersistence", TaskPersistence::getAllHoursForDate, rc, err);
            sqlite3_finalize(stmt);
            return -1;
        }

        durationsGroupedByDate[date] = models;

        sqlite3_finalize(stmt);
        pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskPersistence", date);
    }

    return 0;
}

const std::string TaskPersistence::getById = "SELECT "
                                     "task_id, "
                                     "billable, "
                                     "unique_identifier, "
                                     "hours, "
                                     "minutes, "
                                     "description, "
                                     "date_created, "
                                     "date_modified, "
                                     "is_active, "
                                     "project_id, "
                                     "category_id, "
                                     "workday_id "
                                     "FROM tasks "
                                     "WHERE task_id = ?;";

const std::string TaskPersistence::create = "INSERT INTO "
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

const std::string TaskPersistence::update = "UPDATE tasks "
                                    "SET "
                                    "billable = ?, "
                                    "unique_identifier = ?, "
                                    "hours = ?, "
                                    "minutes = ?, "
                                    "description = ?, "
                                    "project_id = ?, "
                                    "category_id = ?, "
                                    "workday_id = ?, "
                                    "date_modified = ? "
                                    "WHERE task_id = ?;";

const std::string TaskPersistence::isActive = "UPDATE tasks "
                                      "SET "
                                      "is_active = 0, "
                                      "date_modified = ? "
                                      "WHERE task_id = ?;";

const std::string TaskPersistence::getDescriptionById = "SELECT "
                                                "description "
                                                "FROM tasks "
                                                "WHERE task_id = ?;";

const std::string TaskPersistence::isDeleted = "SELECT "
                                       "is_active "
                                       "FROM tasks "
                                       "WHERE task_id = ?;";

const std::string TaskPersistence::getAllHoursForDateRange = "SELECT "
                                                     "hours, "
                                                     "minutes "
                                                     "FROM tasks "
                                                     "INNER JOIN workdays "
                                                     "ON tasks.workday_id = workdays.workday_id "
                                                     "WHERE workdays.date >= ? "
                                                     "AND workdays.date <= ? "
                                                     "AND tasks.is_active = 1";

const std::string TaskPersistence::getBillableHoursForDateRange = "SELECT "
                                                          "hours, "
                                                          "minutes "
                                                          "FROM tasks "
                                                          "INNER JOIN workdays "
                                                          "ON tasks.workday_id = workdays.workday_id "
                                                          "WHERE workdays.date >= ? "
                                                          "AND workdays.date <= ? "
                                                          "AND tasks.billable = 1 "
                                                          "AND tasks.is_active = 1";

const std::string TaskPersistence::getAllHoursForDate = "SELECT "
                                                "hours, "
                                                "minutes "
                                                "FROM tasks "
                                                "INNER JOIN workdays "
                                                "ON tasks.workday_id = workdays.workday_id "
                                                "WHERE workdays.date = ? "
                                                "AND tasks.is_active = 1";
} // namespace tks::Persistence
