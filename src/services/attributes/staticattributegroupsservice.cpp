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

#include "staticattributegroupsservice.h"

#include "../../common/logmessages.h"

#include "../../common/messages/sqlitemessages.h"

namespace tks::Services
{
StaticAttributeGroupsService::StaticAttributeGroupsService(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : Persistence::PersistenceBase(logger, databaseFilePath)
    , pLogger(logger)
{
}

SqliteResult StaticAttributeGroupsService::FilterByStaticFlagAndWithValueCounts(
    std::vector<StaticAttributeGroupViewModel>& staticAttributeGroupViewModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        StaticAttributeGroupsService::filterStaticWithValueCounts.c_str(),
        static_cast<int>(StaticAttributeGroupsService::filterStaticWithValueCounts.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            StaticAttributeGroupsService::filterStaticWithValueCounts,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            StaticAttributeGroupViewModel model;

            int columnIndex = 0;

            model.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.AttributeGroupName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            model.StaticAttributeValueCount = sqlite3_column_int(stmt, columnIndex++);

            staticAttributeGroupViewModels.push_back(model);
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
        pLogger->error(LogMessages::ExecStepTemplate,
            StaticAttributeGroupsService::filterStaticWithValueCounts,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::FilterEntities, staticAttributeGroupViewModels.size(), "");

    return SqliteResult::OK();
}

std::string StaticAttributeGroupsService::filterStaticWithValueCounts =
    "SELECT "
    "attribute_groups.attribute_group_id, "
    "attribute_groups.name, "
    "COUNT(static_attribute_values.static_attribute_value_id) AS static_attribute_value_count "
    "FROM attribute_groups "
    "INNER JOIN static_attribute_values "
    "ON attribute_groups.attribute_group_id = static_attribute_values.attribute_group_id "
    "WHERE attribute_groups.is_active = 1 "
    "AND attribute_groups.is_static = 1 "
    "AND static_attribute_values.is_active = 1 "
    "GROUP BY attribute_groups.attribute_group_id, attribute_groups.name";
} // namespace tks::Services
