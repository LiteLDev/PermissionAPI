#pragma once
#include <Global.h>
#include <third-party/Nlohmann/json.hpp>
#include "Data/Role.hpp"

class Permission {

    bool validateData();

public:

    PERM::Roles roles;
    PERM::PermInfoList permInfoList;
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
     * @brief Create a role.
     * 
     * @param  name         The name of the role.
     * @param  diaplayName  The display name of the role.
     * @return std::shared_ptr<Role>  The shared pointer to the role.
     */
    std::shared_ptr<PERM::Role> createRole(const std::string& name, const std::string& displayName);

    std::shared_ptr<PERM::Role> getRole(const std::string& name);

    std::shared_ptr<PERM::Role> getOrCreateRole(const std::string& name);

    void deleteRole(const std::string& name);
    
    /**
     * @brief Register a permission.
     * 
     * @param  name  The name of the permission like 'Namespace:permissionName'
     * @param  desc  The description of the permission 
     * @throws std::invalid_argument
     */
    void registerPermission(const std::string& name, const std::string& desc);

    void deletePermission(const std::string& name);

    /**
     * @brief Check if the player has the permission.
     * 
     * @param  xuid  The xuid of the player
     * @param  name  The name of the permission
     * @return bool  True if the player has the permission, false otherwise.
     */
    bool checkPermission(const xuid_t& xuid, const std::string& name) const;

    /**
     * @brief Get if the player is in the role.
     * 
     * @param  xuid  The xuid of the player
     * @param  name  The name of the role
     * @return bool  True if the player is in the role, false otherwise.
     */
    bool isMemberOf(const xuid_t& xuid, const std::string& name) const;

    PERM::Roles getPlayerRoles(const xuid_t& xuid) const;

    PERM::Permissions getPlayerPermissions(const xuid_t& xuid) const;

};