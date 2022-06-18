#pragma once
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
