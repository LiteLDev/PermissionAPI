#include "pch.h"
#include "Mod.h"
#include "Data.h"

#pragma warning(disable: 4297)

PERMAPI void PERM_CreateRole(const std::string& name, const std::string& displayName, std::weak_ptr<PERM::Role>& ret) {
    ret = mod.perm.createRole(name, displayName);
}

PERMAPI bool PERM_RoleExists(const std::string& name) {
    return mod.perm.roles.contains(name);
}

PERMAPI void PERM_GetRole(const std::string& name, std::weak_ptr<PERM::Role>& ret) {
    ret = mod.perm.getRole(name);
}

PERMAPI void PERM_GetOrCreateRole(const std::string& name, std::weak_ptr<PERM::Role>& ret) {
    ret = mod.perm.getOrCreateRole(name);
}

PERMAPI void PERM_RegisterPermission(const std::string& name, const std::string& desc) {
    mod.perm.registerPermission(name, desc);
}

PERMAPI void PERM_DeletePermission(const std::string& name) {
    mod.perm.deletePermission(name);
}

PERMAPI bool PERM_PermissionExists(const std::string& name) {
    return mod.perm.permInfoList.contains(name);
}

PERMAPI bool PERM_CheckPermission(const xuid_t& xuid, const std::string& name) {
    return mod.perm.checkPermission(xuid, name);
}

PERMAPI bool PERM_IsMemberOf(const std::string& xuid, const std::string& role) {
    return mod.perm.isMemberOf(xuid, role);
}

PERMAPI void PERM_GetPlayer$1bilities(const xuid_t& xuid, PERM::Permissions& ret) {
    ret = mod.perm.getPlayerPermissions(xuid);
}

PERMAPI void PERM_GetPlayerRoles(const xuid_t& xuid, PERM::Roles& ret) {
    ret = mod.perm.getPlayerRoles(xuid);
}

PERMAPI void PERM_SaveData() {
    mod.perm.save();
}