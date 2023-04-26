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

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <spdlog/logger.h>
#include <sqlite3.h>

#include "../models/clientmodel.h"

namespace tks
{
namespace Core
{
class Environment;
} // namespace Core
namespace Data
{
class ClientData final
{
public:
    ClientData() = delete;
    ClientData(const ClientData&) = delete;
    ClientData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~ClientData();

    ClientData& operator=(const ClientData&) = delete;

    std::int64_t Create(Model::ClientModel& client);
    int Filter(const std::string& searchTerm, /*out*/ std::vector<Model::ClientModel>& clients);
    int GetById(const std::int64_t clientId, /*out*/ Model::ClientModel& model);
    int Update(Model::ClientModel& client);
    int Delete(const std::int64_t clientId);
    int FilterByEmployerId(const std::int64_t employerId, /*out*/ std::vector<Model::ClientModel>& clients);

    std::int64_t GetLastInsertId() const;

private:
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static const std::string createClient;
    static const std::string filterClients;
    static const std::string getClientById;
    static const std::string filterClientsByEmployerId;
    static const std::string updateClient;
    static const std::string deleteClient;
};
} // namespace Data
} // namespace tks
