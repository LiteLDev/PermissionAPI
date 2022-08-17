#pragma once
#include <LoggerAPI.h>
#include <I18nAPI.h>
#include "Data.h"
#include "pch.h"

class Permission;

class Mod {
    
public:

    Logger logger;
    Permission perm;

    Mod();
    void entry();

};
extern Mod mod;
