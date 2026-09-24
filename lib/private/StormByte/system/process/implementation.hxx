/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-System.
 *
 * StormByte-System original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-System source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte Base tree), which
 * remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-System is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-System. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#pragma once

#include <StormByte/system/pipe.hxx>
#include <StormByte/system/process.hxx>

#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace StormByte::System {
	/**
	 * @class ProcessImplementation
	 * @brief Private implementation state for Process.
	 */
	class STORMBYTE_SYSTEM_PRIVATE ProcessImplementation {
		public:
			Process::Status m_status;
#ifdef UNIX
			pid_t m_pid;
#else
			STARTUPINFOW m_siStartInfo;
			PROCESS_INFORMATION m_piProcInfo;
#endif
			std::shared_ptr<Pipe> m_pstdout;
			std::shared_ptr<Pipe> m_pstdin;
			std::shared_ptr<Pipe> m_pstderr;
			std::filesystem::path m_program;
			std::vector<std::string> m_arguments;
			std::unique_ptr<std::thread> m_forwarder;
			std::shared_ptr<std::atomic_bool> m_forwarder_cancel;
	};
}
