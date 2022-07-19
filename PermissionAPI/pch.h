#ifndef PCH_H
#define PCH_H

#define _AMD64_
#define NOMINMAX
#include <LLAPI.h>
#include <Global.h>
#include <LoggerAPI.h>

#define PERMAPI extern "C" _declspec(dllexport)
#define PERM_VER_MAJOR 0
#define PERM_VER_MINOR 1
#define PERM_VER_REV 0
#define PERM_VER_STATUS LL::Version::Beta

extern Logger& logger;

namespace fs = std::filesystem;

static LL::Version PERM_VER{PERM_VER_MAJOR,
                           PERM_VER_MINOR,
                           PERM_VER_REV,
                           PERM_VER_STATUS};
static const std::string PLUGIN_DIR = "./plugins/PermissionAPI/";
static const std::string DATA_FILE = PLUGIN_DIR + "data.json";
static const std::string LANG_FILE = PLUGIN_DIR + "lang.json";

#endif // PCH_H
