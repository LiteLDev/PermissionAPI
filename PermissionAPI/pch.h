﻿#ifndef PCH_H
#define PCH_H

#include <LLAPI.h>
#include <Global.h>
#include <LoggerAPI.h>

#define PERM_VER_MAJOR 2
#define PERM_VER_MINOR 9
#define PERM_VER_REV 0
#define PERM_VER_STATUS ll::Version::Beta

extern Logger& logger;

namespace fs = std::filesystem;

static ll::Version PERM_VER{PERM_VER_MAJOR,
                           PERM_VER_MINOR,
                           PERM_VER_REV,
                           PERM_VER_STATUS};
static const std::string PLUGIN_DIR = "./plugins/PermissionAPI/";
static const std::string DATA_FILE = PLUGIN_DIR + "data.json";

#endif // PCH_H
