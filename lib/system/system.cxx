#include <system/system.hxx>
#include <util/string.hxx>
#ifdef WINDOWS
#include <windows.h> // For MAX_PATH
#else
#include <cstdlib> // For mkstemp
#include <unistd.h> // For close
#endif

using namespace StormByte;

std::filesystem::path System::TempFileName() {
#ifdef WINDOWS
    wchar_t tempPath[MAX_PATH];
    wchar_t tempFile[MAX_PATH];

    // Get the path to the temporary file directory
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        throw std::runtime_error("Error getting temp path");
    }

    // Create a unique temporary filename
    if (GetTempFileNameW(tempPath, L"TMP", 0, tempFile) == 0) {
        throw std::runtime_error("Error getting temp file name");
    }

    return Util::String::UTF8Encode(std::wstring(tempFile));
#else
    char temp_filename[] = "/tmp/config_testXXXXXX";
    int fd = mkstemp(temp_filename);
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    close(fd);
    return std::string(temp_filename);
#endif
}
