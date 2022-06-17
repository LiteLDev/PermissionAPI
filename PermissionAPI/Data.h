#pragma once
#include <Global.h>
#include <third-party/Nlohmann/json.hpp>
#include "pch.h"

struct PermAbility;
struct PermAbilityInfo;
struct PermGroup;

//void from_json(const nlohmann::json& from, PermAbility& to);
//void to_json(nlohmann::json& to, const PermAbility& from);
//void from_json(const nlohmann::json& from, PermGroup& to);
//void to_json(nlohmann::json& to, const PermGroup& from);

struct PermAbility {
    std::string name;
    bool enabled;
    nlohmann::json raw;

    /**
     * @brief Get the namespace of the ability.
     * 
     * @return std::string  The namespace of the ability.
     */
    inline std::string namespc() const {
        return this->name.substr(0, this->name.find_first_of(':'));
    }
};

struct PermAbilityInfo {
    std::string name;
    std::string desc;
};

struct PermGroup {
    std::string name;
    std::unordered_map<std::string, PermAbility> abilities;
    std::vector<xuid_t> members;
    int priority = 0;
};

class Permission {
    std::unordered_map<std::string, PermGroup> groups;
    std::vector<PermAbilityInfo> abilityInfo;
    static nlohmann::json defaultData;

public:

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

};