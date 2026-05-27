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

#include "database_backup_zip.h"

#include <filesystem>
#include <fstream>
#include <vector>

#include <fmt/format.h>

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

DatabaseBackupZip::DatabaseBackupZip(const std::string& backupDirectory)
    : mBackupDirectory(backupDirectory)
{
}

ZipResult DatabaseBackupZip::operator()(const std::string& inFileName)
{
    std::ifstream inFile(inFileName, std::ios::binary);
    if (!inFile.is_open()) {
        return ZipResult::Fail(-1, fmt::format("Failed to read file: \"{0}\"", inFileName));
    }

    inFile.seekg(0, std::ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    std::vector<char> fileData(fileSize);
    inFile.read(fileData.data(), fileSize);
    inFile.close();

    std::vector<unsigned char> compressedData;

    z_stream stream{};
    stream.avail_in = fileSize;
    stream.next_in = const_cast<unsigned char*>(reinterpret_cast<unsigned char*>(fileData.data()));

    int ret =
        deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        return ZipResult::Fail(ret, "Unable to initialize zlib");
    }

    compressedData.resize(deflateBound(&stream, fileSize));
    stream.avail_out = compressedData.size();
    stream.next_out = compressedData.data();

    ret = deflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&stream);
        return ZipResult::Fail(ret, "Deflate (zip) file failed with zlib");
    }

    compressedData.resize(stream.total_out);
    deflateEnd(&stream);

    // Extract filename from path
    std::string filename = inFileName;
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = filename.substr(lastSlash + 1);
    }

    auto outputFileFullPath = fmt::format("{0}\\{1}", mBackupDirectory, filename);
    std::ofstream outFile(outputFileFullPath, std::ios::binary);
    if (!outFile.is_open()) {
        return ZipResult::Fail(-1, "Cannot create output zip file");
    }

    outFile.write(reinterpret_cast<char*>(compressedData.data()), compressedData.size());
    outFile.close();

    try {
        if (!std::filesystem::remove(inFileName)) {
            return ZipResult::Fail(-1, fmt::format("Failed to delete file \"{0}\"", filename));
        }
    } catch (const std::filesystem::filesystem_error& err) {
        ret = err.code().value();
        return ZipResult::Fail(ret, fmt::format("Failed to delete file \"{0}\"", err.what()));
    }

    return ZipResult::OK();
}
} // namespace tks::Core
