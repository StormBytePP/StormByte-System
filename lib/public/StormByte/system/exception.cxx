#include <StormByte/system/exception.hxx>

#include <cstring>

using namespace StormByte::System;

Exception::Exception(const std::string& message):StormByte::Exception(message) {}

FileIOError::FileIOError(const std::filesystem::path& file, const Operation& operation):
Exception("File " + file.string() + " can not be opened for " + operation_to_string(operation)) {}
