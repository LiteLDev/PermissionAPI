#include "pch.h"
#include "Mod.h"
#include "Data.h"

PERMAPI void PERM_CreateGroup(const std::string& name, const std::string& displayName, std::weak_ptr<PermGroup>& ret) {
    if (mod.perm.groups.contains(name)) {
        throw std::invalid_argument("Group already exists");
    }
    PermGroup* group = nullptr;
    if (name == "everyone") {
        group = new EveryonePermGroup;
    } else if (name == "admin") {
        group = new AdminPermGroup;
    } else {
        group = new GeneralPermGroup;
    }
    group->name = name;
    group->displayName = displayName;
    ret = (mod.perm.groups[name] = std::shared_ptr<PermGroup>(group));
    mod.perm.save();
}

PERMAPI bool PERM_GroupExists(const std::string& name) {
    return mod.perm.groups.contains(name);
}

PERMAPI void PERM_GetGroup(const std::string& name, std::weak_ptr<PermGroup>& ret) {
    if (!mod.perm.groups.contains(name)) {
        throw std::invalid_argument("Group not found");
    }
    ret = mod.perm.groups[name];
}

PERMAPI void PERM_GetOrCreateGroup(const std::string& name, std::weak_ptr<PermGroup>& ret) {
    ret = mod.perm.groups[name];
}

PERMAPI void PERM_RegisterAbility(const std::string& name, const std::string& desc) {
    if (mod.perm.abilitiesInfo.contains(name)) {
        throw std::invalid_argument("Ability already exists");     
    }
    mod.perm.abilitiesInfo[name] = {name, desc};
    mod.perm.save();
}

PERMAPI bool PERM_AbilityExists(const std::string& name) {
    return mod.perm.abilitiesInfo.contains(name);
}

PERMAPI bool PERM_HasAbility(const xuid_t& xuid, const std::string& name) {
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