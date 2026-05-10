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

#include "filterentityservice.h"

#include "../../common/logmessages.h"

#include "../../common/messages/sqlitemessages.h"

#include "../../utils/utils.h"

namespace tks::Services
{
FilterEntityModel::FilterEntityModel(EditListEntityType type)
    : Type(type)
    , EntityId(-1)
    , EntityName("")
    , EntityDateModified(-1)
    , ParentEntityName("")
{
}

FilterEntityModel::FilterEntityModel(EditListEntityType type,
    std::int64_t entityId,
    const std::string& entityName,
    std::int32_t entityDateModified,
    std::string parentEntityName)
    : Type(type)
    , EntityId(entityId)
    , EntityName(entityName)
    , EntityDateModified(entityDateModified)
    , ParentEntityName(parentEntityName)
{
}

FilterEntityService::FilterEntityService(const std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : Persistence::PersistenceBase(logger, databaseFilePath)
{
}

FilterEntityService::~FilterEntityService() {}

SqliteResult FilterEntityService::FilterClients(const std::string& searchTerm,
    std::vector<FilterEntityModel> models)
{
    sqlite3_stmt* stmt = nullptr;

    auto formattedSearchTerm = Utils::FormatSqlSearchTerm(searchTerm);

    int rc = sqlite3_prepare_v2(pDb,
        FilterEntityService::filterClients.c_str(),
        static_cast<int>(FilterEntityService::filterClients.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, FilterEntityService::filterClients, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // client name
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

    // client description
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

    bindIndex++;

    // linked employer name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessages::BindParameterTemplate, "employer_name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            FilterEntityModel clientModel(EditListEntityType::Clients);
            int columnIndex = 0;

            clientModel.EntityId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            clientModel.EntityName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            clientModel.EntityDateModified = sqlite3_column_int(stmt, columnIndex++);

            res = sqlite3_column_text(stmt, columnIndex);
            clientModel.ParentEntityName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            models.push_back(clientModel);
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
            LogMessages::ExecStepTemplate, FilterEntityService::filterClients, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, models.size(), searchTerm);

    return SqliteResult::OK();
}

std::string FilterEntityService::filterClients = "SELECT "
                                                 "clients.client_id, "
                                                 "clients.name AS client_name, "
                                                 "clients.description AS client_description, "
                                                 "clients.date_created, "
                                                 "clients.date_modified, "
                                                 "clients.is_active, "
                                                 "clients.employer_id, "
                                                 "employers.name AS employer_name "
                                                 "FROM clients "
                                                 "INNER JOIN employers "
                                                 "ON clients.employer_id = employers.employer_id "
                                                 "WHERE clients.is_active = 1 "
                                                 "AND (client_name LIKE ? "
                                                 "OR client_description LIKE ? "
                                                 "OR employer_name LIKE ?); ";
} // namespace tks::Services
