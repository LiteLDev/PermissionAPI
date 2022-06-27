#pragma once
#include <Global.h>
#include <third-party/Nlohmann/json.hpp>
#include "pch.h"
#include "Data/PermGroup.hpp"

class Permission {

public:

    PermGroups groups;
    std::vector<PermAbilityInfo> abilityInfo;
    static nlohmann::json defaultData;

    /**
     * @brief Loads the data from the file.
     * 
     */
    void load();
    /**
     * @brief Saves the data to the file.
     * 
     */
    void save();
    
    /**
     * @brief Register a ability.
     * 
     * @param  nspc  The namespace of the ability(without ':')
     * @param  name  The name of the ability
     * @param  desc  The description of the ability 
     * @throws std::invalid_argument
     */
    void registerAbility(const std::string& nspc, const std::string& name, const std::string& desc);

    /**
     * @brief Get the ability info.
     * 
     * @param  name             The full name of the ability
     * @return PermAbilityInfo  The info of the ability
     * @throws std::out_of_range
     */
    PermAbilityInfo getAbilityInfo(const std::string& name) const;

    /**
     * @brief Create a new permission group.
     * 
     * @param  name        The name of the group
     * @param  members     The members of the group(optional)
     * @return PermGroup&  The created group
     * @note   Please save the data after changing the group.
     */
    PermGroup& createGroup(const std::string& name, const std::vector<xuid_t>& members = {});

    /**
     * @brief Get the group.
     * 
     * @param  name       The name of the group
     * @return PermGroup  The group
     * @throws std::out_of_range
     * @note   Please save the data after changing the group.
     */
    PermGroup& getGroup(const std::string& name);

    /**
     * @brief Check if the player has the ability.
     * 
     * @param  xuid  The xuid of the player
     * @param  name  The name of the ability
     * @return bool  True if the player has the ability, false otherwise.
     */
    bool checkAbility(const xuid_t& xuid, const std::string& name) const;

    /**
     * @brief Check if the player is in the group.
     * 
     * @param  xuid  The xuid of the player
     * @param  name  The name of the group
     * @return bool  True if the player is in the group, false otherwise.
     */
    bool checkIsMember(const xuid_t& xuid, const std::string& name) const;

    PermGroups getPlayerGroups(const xuid_t& xuid) const;

    std::vector<std::string> getPlayerAbilities(const xuid_t& xuid);

};