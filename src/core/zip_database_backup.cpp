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

#include "zip_database_backup.h"

#include <filesystem>
#include <fstream>
#include <vector>

#include <fmt/format.h>
#include <libzippp/libzippp.h>

namespace tks::Core
{
ZipResult ZipResult::OK()
{
    return ZipResult{ true };
}

ZipResult ZipResult::Fail(int returnCode, const std::string& errorMessage)
{
    return ZipResult{ false, returnCode, errorMessage };
}

ZipDatabaseBackup::ZipDatabaseBackup(std::shared_ptr<spdlog::logger> logger,
    const std::string& backupDirectory)
    : pLogger(logger)
    , mBackupDirectory(backupDirectory)
{
}

ZipResult ZipDatabaseBackup::operator()(const std::string& inFileName)
{
    std::ifstream inFile(inFileName, std::ios::binary | std::ios::ate);
    if (!inFile.is_open()) {
        return ZipResult::Fail(-1, fmt::format("Failed to read file: \"{0}\"", inFileName));
    }

    size_t fileSize = inFile.tellg();
    if (fileSize < 0) {
        return ZipResult::Fail(-1, "File is empty or could not get file size");
    }

    SPDLOG_LOGGER_TRACE(pLogger, "Read \"{0}\" bytes of file: \"{1}\"", fileSize, inFileName);

    inFile.seekg(0, std::ios::beg);

    std::vector<char*> buffer(fileSize);

    inFile.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    inFile.close();

    std::filesystem::path backupFilePath(inFileName);
    std::filesystem::path fileName = backupFilePath.filename();
    if (fileName.empty()) {
        return ZipResult::Fail(-1, "Could not get file name");
    }

    std::string dbFileName = fileName.string();

    std::filesystem::path outputFileFullPath =
        std::filesystem::path(mBackupDirectory) / "taskies.zip";
    SPDLOG_LOGGER_TRACE(pLogger, "Zip file path: \"{0}\"", outputFileFullPath.string());

    libzippp::ZipArchive zf(outputFileFullPath.string());
    bool result = zf.open(libzippp::ZipArchive::Write);
    if (!result) {
        return ZipResult::Fail(result, "Failed to open or create zip file archive");
    }

    result = zf.addData(dbFileName, buffer.data(), fileSize);
    if (!result) {
        return ZipResult::Fail(result, "Failed to add database file to zip file archive");
    }

    int ret = zf.close();
    if (ret != LIBZIPPP_OK) {
        return ZipResult::Fail(ret, "Failed to close zip file archive");
    }

    SPDLOG_LOGGER_TRACE(pLogger, "Completed creating zip file");

    try {
        if (!std::filesystem::remove(inFileName)) {
            return ZipResult::Fail(
                -1, fmt::format("Failed to delete original backup file \"{0}\"", "filename"));
        }
    } catch (const std::filesystem::filesystem_error& err) {
        return ZipResult::Fail(err.code().value(),
            fmt::format("Failed to delete original backup file \"{0}\"", err.what()));
    }

    return ZipResult::OK();
}
} // namespace tks::Core
