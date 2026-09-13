/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-System.
 *
 * StormByte-System is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
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
