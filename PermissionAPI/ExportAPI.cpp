#include "pch.h"
#include "Mod.h"
#include "PermissionAPI.h"

#pragma warning(disable: 4297)

extern "C" {

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

PERMAPI void PERM_DeleteRole(const std::string& name) {
    mod.perm.deleteRole(name);
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

}

std::weak_ptr<PERM::Role> PermissionAPI::createRole(const std::string& name, const std::string& displayName) {
    return mod.perm.createRole(name, displayName);
}

bool PermissionAPI::roleExists(const std::string& name) {
    return mod.perm.roles.contains(name);
}

std::weak_ptr<PERM::Role> PermissionAPI::getRole(const std::string& name) {
    return mod.perm.getRole(name);
}

std::weak_ptr<PERM::Role> PermissionAPI::getOrCreateRole(const std::string& name) {
    return mod.perm.getOrCreateRole(name);
}

void PermissionAPI::deleteRole(const std::string& name) {
    mod.perm.deleteRole(name);
}

void PermissionAPI::registerPermission(const std::string& name, const std::string& desc) {
    mod.perm.registerPermission(name, desc);
}

void PermissionAPI::deletePermission(const std::string& name) {
    mod.perm.deletePermission(name);
}

bool PermissionAPI::permissionExists(const std::string& name) {
    return mod.perm.permInfoList.contains(name);
}

bool PermissionAPI::checkPermission(const xuid_t& xuid, const std::string& name) {
    return mod.perm.checkPermission(xuid, name);
}

bool PermissionAPI::isMemberOf(const std::string& xuid, const std::string& role) {
    return mod.perm.isMemberOf(xuid, role);
}

PERM::Permissions PermissionAPI::getPlayerPermissions(const xuid_t& xuid) {
    return mod.perm.getPlayerPermissions(xuid);
}

PERM::Roles PermissionAPI::getPlayerRoles(const xuid_t& xuid) {
    return mod.perm.getPlayerRoles(xuid);
}

void PermissionAPI::saveData() {
    mod.perm.save();
}