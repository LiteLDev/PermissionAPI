#include "pch.h"
#include "Mod.h"
#include "Data.h"

PERMAPI std::shared_ptr<PermGroup> PERM_CreateGroup(const std::string& name, const std::string& displayName, int priority = 0) {
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
    group->priority = priority;
    auto& ret = mod.perm.groups[name] = std::shared_ptr<PermGroup>(group);
    mod.perm.save();
    return ret;
}

PERMAPI bool PERM_GroupExists(const std::string& name) {
    return mod.perm.groups.contains(name);
}

PERMAPI std::shared_ptr<PermGroup> PERM_GetGroup(const std::string& name) {
    if (!mod.perm.groups.contains(name)) {
        throw std::invalid_argument("Group not found");
    }
    return mod.perm.groups[name];
}

PERMAPI std::shared_ptr<PermGroup> PERM_GetOrCreateGroup(const std::string& name) {
    return mod.perm.groups[name];
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

PERMAPI PermAbilities PERM_GetPlayerAbilities(const xuid_t& xuid) {
    return mod.perm.getPlayerAbilities(xuid);
}

PERMAPI PermGroups PERM_GetPlayerGroups(const xuid_t& xuid) {
    return mod.perm.getPlayerGroups(xuid);
}