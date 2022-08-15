#pragma once
#include <LoggerAPI.h>
#include <I18nAPI.h>
#include "Data.h"
#include "pch.h"

class Permission;

class Mod {
    
public:

    Logger logger = Logger("PermAPI");
    Permission perm;

    Mod();
    void entry();

    static const I18nBase::LangData defaultLangData;

};
extern Mod mod;
