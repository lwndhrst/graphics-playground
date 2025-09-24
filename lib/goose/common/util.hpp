#pragma once

#include "goose/common/types.hpp"

#include <fstream>

#define UINT_DIV_CEIL(numerator, denominator) ((numerator + denominator - 1) / denominator)

namespace goose {

template <typename T>
bool
read_file(std::vector<T> &buffer, const std::string &file_path)
{
    // Open file with cursor at the end
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    // Because the cursor is at the end, it gives the size directly in bytes
    usize file_size = static_cast<usize>(file.tellg());

    // Pad buffer so its size is a multiple of sizeof(T)
    buffer.resize(UINT_DIV_CEIL(file_size, sizeof(T)));

    // Move cursor to start of file and read
    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), file_size);
    file.close();

    return true;
}

} // namespace goose
