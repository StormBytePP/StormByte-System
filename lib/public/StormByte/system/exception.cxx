/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-System.
 *
 * StormByte-System is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
 *
 * StormByte-System is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte-System. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#include <StormByte/system/exception.hxx>
#include <cstring>
using namespace StormByte::System;
Exception::Exception(const std::string& message):StormByte::Exception(message) {}
FileIOError::FileIOError(const std::filesystem::path& file, const Operation& operation):
Exception("File " + file.string() + " can not be opened for " + operation_to_string(operation)) {}
ExecutableNotFound::ExecutableNotFound(const std::filesystem::path& exec):
Exception("Executable " + exec.string() + " not found") {}
