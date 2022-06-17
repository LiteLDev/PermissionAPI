#ifndef PCH_H
#define PCH_H

#include <LLAPI.h>
#include "framework.h"
#include "Mod.h"

#define PERMAPI extern "C" _declspec(dllexport)
#define PERM_VER_MAJOR 0
#define PERM_VER_MINOR 0
#define PERM_VER_REV 1
#define PERM_VER_STATUS LL::Version::Beta

class Mod;
extern Mod mod;

namespace fs = std::filesystem;

static Logger& logger = mod.logger;

static const LL::Version PERM_VER{PERM_VER_MAJOR,
                           PERM_VER_MINOR,
                           PERM_VER_REV,
                           PERM_VER_STATUS};
static const std::string PLUGIN_DIR = "./plugins/PermissionAPI/";
static const std::string DATA_FILE = PLUGIN_DIR + "data.json";

#endif // PCH_H
