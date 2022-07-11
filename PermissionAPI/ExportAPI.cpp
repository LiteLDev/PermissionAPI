#include "pch.h"
#include "Mod.h"
#include "Data.h"

#pragma warning(disable: 4297)

PERMAPI void PERM_CreateGroup(const std::string& name, const std::string& displayName, std::weak_ptr<PermGroup>& ret) {
    ret = mod.perm.createGroup(name, displayName);
}

PERMAPI bool PERM_GroupExists(const std::string& name) {
    return mod.perm.groups.contains(name);
}

PERMAPI void PERM_GetGroup(const std::string& name, std::weak_ptr<PermGroup>& ret) {
    ret = mod.perm.getGroup(name);
}

PERMAPI void PERM_GetOrCreateGroup(const std::string& name, std::weak_ptr<PermGroup>& ret) {
    ret = mod.perm.getOrCreateGroup(name);
}

PERMAPI void PERM_RegisterAbility(const std::string& name, const std::string& desc) {
    mod.perm.registerAbility(name, desc);
}

PERMAPI void PERM_DeleteAbility(const std::string& name) {
    mod.perm.deleteAbility(name);
}

PERMAPI bool PERM_AbilityExists(const std::string& name) {
    return mod.perm.abilitiesInfo.contains(name);
}

PERMAPI bool PERM_checkAbility(const xuid_t& xuid, const std::string& name) {
    return mod.perm.checkAbility(xuid, name);
}

PERMAPI bool PERM_IsMemberOf(const std::string& xuid, const std::string& group) {
    return mod.perm.isMemberOf(xuid, group);
}

PERMAPI void PERM_GetPlayerAbilities(const xuid_t& xuid, PermAbilities& ret) {
    ret = mod.perm.getPlayerAbilities(xuid);
}

PERMAPI void PERM_GetPlayerGroups(const xuid_t& xuid, PermGroups& ret) {
    ret = mod.perm.getPlayerGroups(xuid);
}

PERMAPI void PERM_SaveData() {
    mod.perm.save();
}