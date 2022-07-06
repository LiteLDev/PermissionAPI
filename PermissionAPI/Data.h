#pragma once
#include <Global.h>
#include <third-party/Nlohmann/json.hpp>
#include "Data/PermGroup.hpp"

class Permission {

    bool validateData();

public:

    PermGroups groups;
    PermAbilitiesInfo abilitiesInfo;
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
     * @param  name  The name of the ability like 'Namespace:AbilityName'
     * @param  desc  The description of the ability 
     * @throws std::invalid_argument
     */
    void registerAbility(const std::string& name, const std::string& desc);

    void deleteAbility(const std::string& name);

    /**
     * @brief Check if the player has the ability.
     * 
     * @param  xuid  The xuid of the player
     * @param  name  The name of the ability
     * @return bool  True if the player has the ability, false otherwise.
     */
    bool checkAbility(const xuid_t& xuid, const std::string& name) const;

    /**
     * @brief Get if the player is in the group.
     * 
     * @param  xuid  The xuid of the player
     * @param  name  The name of the group
     * @return bool  True if the player is in the group, false otherwise.
     */
    bool isMemberOf(const xuid_t& xuid, const std::string& name) const;

    PermGroups getPlayerGroups(const xuid_t& xuid) const;

    PermAbilities getPlayerAbilities(const xuid_t& xuid) const;

};