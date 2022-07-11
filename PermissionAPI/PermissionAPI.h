#pragma once
#include <LLAPI.h>
#include "Data/PermGroup.hpp"

class PermissionAPI {

    LL::Plugin plugin;

    using FuncCreateGroup = void(*)(const std::string&, const std::string&, std::weak_ptr<PermGroup>&);
    using FuncGroupExists = bool(*)(const std::string&);
    using FuncGetGroup = void(*)(const std::string&, std::weak_ptr<PermGroup>&);
    using FuncGetOrCreateGroup = void(*)(const std::string&, std::weak_ptr<PermGroup>&);
    using FuncRegisterAbility = void(*)(const std::string&, const std::string&);
    using FuncDeleteAbility = void(*)(const std::string&);
    using FuncAbilityExists = bool(*)(const std::string&);
    using FuncCheckAbility = bool(*)(const xuid_t&, const std::string&);
    using FuncIsMemberOf = bool(*)(const xuid_t&, const std::string&);
    using FuncGetPlayerGroups = void(*)(const xuid_t&, PermGroups&);
    using FuncGetPlayerAbilities = void(*)(const xuid_t&, PermAbilities&);
    using FuncSaveData = void(*)();

    FuncCreateGroup funcCreateGroup;
    FuncGroupExists funcGroupExists;
    FuncGetGroup funcGetGroup;
    FuncGetOrCreateGroup funcGetOrCreateGroup;
    FuncRegisterAbility funcRegisterAbility;
    FuncDeleteAbility funcDeleteAbility;
    FuncAbilityExists funcAbilityExists;
    FuncCheckAbility funcCheckAbility;
    FuncIsMemberOf funcIsMemberOf;
    FuncGetPlayerGroups funcGetPlayerGroups;
    FuncGetPlayerAbilities funcGetPlayerAbilities;
    FuncSaveData funcSaveData;

    template <typename T>
    T getFunc(const std::string& name) {
        return GetProcAddress(plugin.handle, name.c_str());
    }

public:
    
    PermissionAPI(bool option = false) {
        if (!LL::hasPlugin("PermissionAPI")) {
            throw std::runtime_error("Dependency plugin PermissionAPI not found");
        }
        auto pPtr = LL::getPlugin("PermissionAPI");
        if (!pPtr) throw std::runtime_error("Cannot get the plugin object");
        plugin = *pPtr;
        funcCreateGroup = getFunc<FuncCreateGroup>("PERM_CreateGroup");
        funcGroupExists = getFunc<FuncGroupExists>("PERM_GroupExists");
        funcGetGroup = getFunc<FuncGetGroup>("PERM_GetGroup");
        funcGetOrCreateGroup = getFunc<FuncGetOrCreateGroup>("PERM_GetOrCreateGroup");
        funcRegisterAbility = getFunc<FuncRegisterAbility>("PERM_RegisterAbility");
        funcDeleteAbility = getFunc<FuncDeleteAbility>("PERM_DeleteAbility");
        funcAbilityExists = getFunc<FuncAbilityExists>("PERM_AbilityExists");
        funcCheckAbility = getFunc<FuncCheckAbility>("PERM_CheckAbility");
        funcIsMemberOf = getFunc<FuncIsMemberOf>("PERM_IsMemberOf");
        funcGetPlayerGroups = getFunc<FuncGetPlayerGroups>("PERM_GetPlayerGroups");
        funcGetPlayerAbilities = getFunc<FuncGetPlayerAbilities>("PERM_GetPlayerAbilities");
        funcSaveData = getFunc<FuncSaveData>("PERM_SaveData");
    }

    /**
     * @brief Create a Group object.
     * 
     * @param  name         The name of the group.
     * @param  displayName  The display name of the group.
     * @return std::weak_ptr<PermGroup>  The created group(weak ref).
     * @throws std::invalid_argument     If the group already exists. 
     * @par Example
     * @code
     * PermissionAPI api;
     * auto group = api.createGroup("group1", "Group 1");
     * ...
     * if (!group.expired()) {
     *     auto groupPtr = group.lock();
     *     groupPtr->addMember("1145141919810");
     *     api.saveData();
     * }
     * @endcode
     */
    std::weak_ptr<PermGroup> createGroup(const std::string& name, const std::string& displayName) {
        if (funcCreateGroup == nullptr) throw std::runtime_error("Function not found");
        std::weak_ptr<PermGroup> ptr{};
        funcCreateGroup(name, displayName, ptr);
        return ptr;
    }

    /**
     * @brief Check if a group exists.
     * 
     * @param  name  The name of the group.
     * @return bool  True If the group exists, false otherwise.
     */
    bool groupExists(const std::string& name) {
        if (funcGroupExists == nullptr) throw std::runtime_error("Function not found");
        return funcGroupExists(name);
    }

    /**
     * @brief Get a group object.
     * 
     * @param  name                      The name of the group.
     * @return std::weak_ptr<PermGroup>  The group(weak ref).
     * @throws std::invalid_argument     If the group does not exist. 
     */
    std::weak_ptr<PermGroup> getGroup(const std::string& name) {
        if (funcGetGroup == nullptr) throw std::runtime_error("Function not found");
        std::weak_ptr<PermGroup> ptr{};
        funcGetGroup(name, ptr);
        return ptr;
    }

    /**
     * @brief Get a group object. If the group does not exist, it will be created.
     * 
     * @param  name                      The name of the group.
     * @return std::weak_ptr<PermGroup>  The group(weak ref).
     */
    std::weak_ptr<PermGroup> getOrCreateGroup(const std::string& name) {
        if (funcGetOrCreateGroup == nullptr) throw std::runtime_error("Function not found");
        std::weak_ptr<PermGroup> ptr{};
        funcGetOrCreateGroup(name, ptr);
        return ptr;
    }

    /**
     * @brief Register an ability.
     *
     * @param name  The name of the ability.
     * @param desc  The description name of the ability.
     */
    void registerAbility(const std::string& name, const std::string& desc) {
        if (funcRegisterAbility == nullptr) throw std::runtime_error("Function not found");
        funcRegisterAbility(name, desc);
    }
    
    /**
     * @brief Delete an ability.
     * 
     * @param    name  The name of the ability.
     * @warning  This function will also delete the ability instance in groups.
     */
    void deleteAbility(const std::string& name) {
        if (funcDeleteAbility == nullptr) throw std::runtime_error("Function not found");
        funcDeleteAbility(name);
    }

    /**
     * @brief Check if an ability exists.
     * 
     * @param  name  The name of the ability.
     * @return bool  True If the ability exists, false otherwise.
     */
    bool abilityExists(const std::string& name) {
        if (funcAbilityExists == nullptr) throw std::runtime_error("Function not found");
        return funcAbilityExists(name);
    }

    /**
     * @brief Check whether the player has the ability or not.
     * 
     * @param  xuid  The xuid of the player.
     * @param  name  The name of the ability.
     * @return bool  True If the player has the ability, false otherwise.
     */
    bool checkAbility(const xuid_t& xuid, const std::string& name) {
        if (funcCheckAbility == nullptr) throw std::runtime_error("Function not found");
        return funcCheckAbility(xuid, name);
    }

    /**
     * @brief Check if a player is a member of a group.
     * 
     * @param  xuid  The xuid of the player.
     * @param  name  The name of the group.
     * @return bool  True If the player is a member of the group, false otherwise.
     */
    bool isMemberOf(const xuid_t& xuid, const std::string& name) {
        if (funcIsMemberOf == nullptr) throw std::runtime_error("Function not found");
        return funcIsMemberOf(xuid, name);
    }

    /**
     * @brief Get the groups of a player.
     * 
     * @param  xuid        The xuid of the player.
     * @return PermGroups  The groups of the player.
     */
    PermGroups getPlayerGroups(const xuid_t& xuid) {
        if (funcGetPlayerGroups == nullptr) throw std::runtime_error("Function not found");
        PermGroups groups;
        funcGetPlayerGroups(xuid, groups);
        return groups;
    }

    /**
     * @brief Get the abilities of a player.
     * 
     * @param  xuid        The xuid of the player.
     * @return PermAbilities  The abilities of the player.
     */
    PermAbilities getPlayerAbilities(const xuid_t& xuid) {
        if (funcGetPlayerAbilities == nullptr) throw std::runtime_error("Function not found");
        PermAbilities abilities;
        funcGetPlayerAbilities(xuid, abilities);
        return abilities;
    }

    /**
     * @brief Save the data.
     */
    void saveData() {
        if (funcSaveData == nullptr) throw std::runtime_error("Function not found");
        funcSaveData();
    }

};