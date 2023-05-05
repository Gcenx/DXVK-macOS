#include <array>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <numeric>

#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#endif

#include "util_env.h"

#include "./com/com_include.h"

namespace dxvk::env {

  std::string getEnvVar(const char* name) {
#ifdef _WIN32
    std::vector<WCHAR> result;
    result.resize(MAX_PATH + 1);

    DWORD len = ::GetEnvironmentVariableW(str::tows(name).c_str(), result.data(), MAX_PATH);
    result.resize(len);

    return str::fromws(result.data());
#else
    const char* result = std::getenv(name);
    return result ? result : "";
#endif
  }

  void setEnvVar(const char* name, const char* value) {
    wineSetUnixEnv_proc wineSetUnixEnv = nullptr;
#ifdef _WIN32
    HMODULE ntdll = LoadLibrary("ntdll.dll");
    if(ntdll) {
      wineSetUnixEnv = reinterpret_cast<wineSetUnixEnv_proc>(GetProcAddress(ntdll, "__wine_set_unix_envt"));
      wineSetUnixEnv(name, value);
      FreeLibrary(ntdll);
    }
    return;
#else
    putenv(env);
    return;
#endif
  }


  size_t matchFileExtension(const std::string& name, const char* ext) {
    auto pos = name.find_last_of('.');

    if (pos == std::string::npos)
      return pos;

    bool matches = std::accumulate(name.begin() + pos + 1, name.end(), true,
      [&ext] (bool current, char a) {
        if (a >= 'A' && a <= 'Z')
          a += 'a' - 'A';
        return current && *ext && a == *(ext++);
      });

    return matches ? pos : std::string::npos;
  }


  std::string getExeName() {
    std::string fullPath = getExePath();
    auto n = fullPath.find_last_of(env::PlatformDirSlash);
    
    return (n != std::string::npos)
      ? fullPath.substr(n + 1)
      : fullPath;
  }


  std::string getExeBaseName() {
    auto exeName = getExeName();
#ifdef _WIN32
    auto extp = matchFileExtension(exeName, "exe");

    if (extp != std::string::npos)
      exeName.erase(extp);
#endif

    return exeName;
  }


  std::string getExePath() {
#if defined(_WIN32)
    std::vector<WCHAR> exePath;
    exePath.resize(MAX_PATH + 1);

    DWORD len = ::GetModuleFileNameW(NULL, exePath.data(), MAX_PATH);
    exePath.resize(len);

    return str::fromws(exePath.data());
#elif defined(__linux__)
    std::array<char, PATH_MAX> exePath = {};

    size_t count = readlink("/proc/self/exe", exePath.data(), exePath.size());

    return std::string(exePath.begin(), exePath.begin() + count);
#endif
  }
  
  
  void setThreadName(const std::string& name) {
#ifdef _WIN32
    using SetThreadDescriptionProc = HRESULT (WINAPI *) (HANDLE, PCWSTR);

    static auto proc = reinterpret_cast<SetThreadDescriptionProc>(
      ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "SetThreadDescription"));

    if (proc != nullptr) {
      auto wideName = std::vector<WCHAR>(name.length() + 1);
      str::tows(name.c_str(), wideName.data(), wideName.size());
      (*proc)(::GetCurrentThread(), wideName.data());
    }
#else
    std::array<char, 16> posixName = {};
    dxvk::str::strlcpy(posixName.data(), name.c_str(), 16);
    ::pthread_setname_np(pthread_self(), posixName.data());
#endif
  }


  bool createDirectory(const std::string& path) {
#ifdef _WIN32
    WCHAR widePath[MAX_PATH];
    str::tows(path.c_str(), widePath);
    return !!CreateDirectoryW(widePath, nullptr);
#else
    return std::filesystem::create_directories(path);
#endif
  }
  
}
