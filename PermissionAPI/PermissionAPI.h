#pragma once
#include <LLAPI.h>
#include "Data/Role.hpp"

#if defined(PERMAPI_EXPORTS)
#define PERMAPI _declspec(dllexport)
#else
#define PERMAPI _declspec(dllimport)
#endif

#if defined(PERMAPI_DYNAMIC_DEPENDENCY)

class PermissionAPI {

    HMODULE hPlugin;

    using FuncCreateRole = void(*)(const std::string&, const std::string&, std::weak_ptr<PERM::Role>&);
    using FuncRoleExists = bool(*)(const std::string&);
    using FuncGetRole = void(*)(const std::string&, std::weak_ptr<PERM::Role>&);
    using FuncGetOrCreateRole = void(*)(const std::string&, std::weak_ptr<PERM::Role>&);
    using FuncRegisterPermission = void(*)(const std::string&, const std::string&);
    using FuncDeletePermission = void(*)(const std::string&);
    using FuncPermissionExists = bool(*)(const std::string&);
    using FuncCheckPermission = bool(*)(const xuid_t&, const std::string&);
    using FuncIsMemberOf = bool(*)(const xuid_t&, const std::string&);
    using FuncGetPlayerRoles = void(*)(const xuid_t&, PERM::Roles&);
    using FuncGetPlayerPermissions = void(*)(const xuid_t&, PERM::Permissions&);
    using FuncSaveData = void(*)();

    FuncCreateRole funcCreateRole;
    FuncRoleExists funcRoleExists;
    FuncGetRole funcGetRole;
    FuncGetOrCreateRole funcGetOrCreateRole;
    FuncRegisterPermission funcRegisterPermission;
    FuncDeletePermission funcDeletePermission;
    FuncPermissionExists funcPermissionExists;
    FuncCheckPermission funcCheckPermission;
    FuncIsMemberOf funcIsMemberOf;
    FuncGetPlayerRoles funcGetPlayerRoles;
    FuncGetPlayerPermissions funcGetPlayerPermissions;
    FuncSaveData funcSaveData;

    template <typename T>
    T getFunc(const std::string& name) {
        return (T)GetProcAddress(hPlugin, name.c_str());
    }

public:
    
    PermissionAPI(bool option = false) {
        if (!LL::hasPlugin("PermissionAPI")) {
            throw std::runtime_error("Dependency plugin PermissionAPI not found");
        }
        auto pPtr = LL::getPlugin("PermissionAPI");
        if (!pPtr) {
            throw std::runtime_error("Cannot get the plugin object");
        }
        hPlugin = pPtr->handle;
        funcCreateRole = getFunc<FuncCreateRole>("PERM_CreateRole");
        funcRoleExists = getFunc<FuncRoleExists>("PERM_RoleExists");
        funcGetRole = getFunc<FuncGetRole>("PERM_GetRole");
        funcGetOrCreateRole = getFunc<FuncGetOrCreateRole>("PERM_GetOrCreateRole");
        funcRegisterPermission = getFunc<FuncRegisterPermission>("PERM_RegisterPermission");
        funcDeletePermission = getFunc<FuncDeletePermission>("PERM_DeletePermission");
        funcPermissionExists = getFunc<FuncPermissionExists>("PERM_PermissionExists");
        funcCheckPermission = getFunc<FuncCheckPermission>("PERM_CheckPermission");
        funcIsMemberOf = getFunc<FuncIsMemberOf>("PERM_IsMemberOf");
        funcGetPlayerRoles = getFunc<FuncGetPlayerRoles>("PERM_GetPlayerRoles");
        funcGetPlayerPermissions = getFunc<FuncGetPlayerPermissions>("PERM_GetPlayerPermissions");
        funcSaveData = getFunc<FuncSaveData>("PERM_SaveData");
    }

    /**
     * @brief Create a Role object.
     * 
     * @param  name         The name of the role.
     * @param  displayName  The display name of the role.
     * @return std::weak_ptr<PERM::Role>  The created role(weak ref).
     * @throws std::invalid_argument      If the role already exists. 
     * @par Example
     * @code
     * PermissionAPI api;
     * auto role = api.createRole("role1", "Role 1");
     * ...
     * if (!role.expired()) {
     *     auto rolePtr = role.lock();
     *     rolePtr->addMember("1145141919810");
     *     api.saveData();
     * }
     * @endcode
     */
    std::weak_ptr<PERM::Role> createRole(const std::string& name, const std::string& displayName) {
        if (funcCreateRole == nullptr) {
            throw std::runtime_error("Function not found");
        }
        std::weak_ptr<PERM::Role> ptr{};
        funcCreateRole(name, displayName, ptr);
        return ptr;
    }

    /**
     * @brief Check if a role exists.
     * 
     * @param  name  The name of the role.
     * @return bool  True If the role exists, false otherwise.
     */
    bool roleExists(const std::string& name) {
        if (funcRoleExists == nullptr) {
            throw std::runtime_error("Function not found");
        }
        return funcRoleExists(name);
    }

    /**
     * @brief Get a role object.
     * 
     * @param  name                       The name of the role.
     * @return std::weak_ptr<PERM::Role>  The role(weak ref).
     * @throws std::invalid_argument      If the role does not exist. 
     */
    std::weak_ptr<PERM::Role> getRole(const std::string& name) {
        if (funcGetRole == nullptr) {
            throw std::runtime_error("Function not found");
        }
        std::weak_ptr<PERM::Role> ptr{};
        funcGetRole(name, ptr);
        return ptr;
    }

    /**
     * @brief Get a role object. If the role does not exist, it will be created.
     * 
     * @param  name                       The name of the role.
     * @return std::weak_ptr<PERM::Role>  The role(weak ref).
     */
    std::weak_ptr<PERM::Role> getOrCreateRole(const std::string& name) {
        if (funcGetOrCreateRole == nullptr) {
            throw std::runtime_error("Function not found");
        }
        std::weak_ptr<PERM::Role> ptr{};
        funcGetOrCreateRole(name, ptr);
        return ptr;
    }

    /**
     * @brief Register an permission.
     *
     * @param name  The name of the permission.
     * @param desc  The description name of the permission.
     */
    void registerPermission(const std::string& name, const std::string& desc) {
        if (funcRegisterPermission == nullptr) {
            throw std::runtime_error("Function not found");
        }
        funcRegisterPermission(name, desc);
    }
    
    /**
     * @brief Delete a permission.
     * 
     * @param    name  The name of the permission.
     * @warning  This function will also delete the permission instances in roles.
     */
    void deletePermission(const std::string& name) {
        if (funcDeletePermission == nullptr) {
            throw std::runtime_error("Function not found");
        }
        funcDeletePermission(name);
    }

    /**
     * @brief Check if a permission exists.
     * 
     * @param  name  The name of the permission.
     * @return bool  True If the permission exists, false otherwise.
     */
    bool permissionExists(const std::string& name) {
        if (funcPermissionExists == nullptr) {
            throw std::runtime_error("Function not found");
        }
        return funcPermissionExists(name);
    }

    /**
     * @brief Check whether the player has the Permission or not.
     * 
     * @param  xuid  The xuid of the player.
     * @param  name  The name of the Permission.
     * @return bool  True If the player has the Permission, false otherwise.
     */
    bool checkPermission(const xuid_t& xuid, const std::string& name) {
        if (funcCheckPermission == nullptr) {
            throw std::runtime_error("Function not found");
        }
        return funcCheckPermission(xuid, name);
    }

    /**
     * @brief Check if a player is a member of a role.
     * 
     * @param  xuid  The xuid of the player.
     * @param  name  The name of the role.
     * @return bool  True If the player is a member of the role, false otherwise.
     */
    bool isMemberOf(const xuid_t& xuid, const std::string& name) {
        if (funcIsMemberOf == nullptr) {
            throw std::runtime_error("Function not found");
        }
        return funcIsMemberOf(xuid, name);
    }

    /**
     * @brief Get the roles of a player.
     * 
     * @param  xuid         The xuid of the player.
     * @return PERM::Roles  The roles of the player.
     */
    PERM::Roles getPlayerRoles(const xuid_t& xuid) {
        if (funcGetPlayerRoles == nullptr) {
            throw std::runtime_error("Function not found");
        }
        PERM::Roles roles;
        funcGetPlayerRoles(xuid, roles);
        return roles;
    }

    /**
     * @brief Get the permissions of a player.
     * 
     * @param  xuid               The xuid of the player.
     * @return PERM::Permissions  The permissions of the player.
     */
    PERM::Permissions getPlayerPermissions(const xuid_t& xuid) {
        if (funcGetPlayerPermissions == nullptr) {
            throw std::runtime_error("Function not found");
        }
        PERM::Permissions permissions;
        funcGetPlayerPermissions(xuid, permissions);
        return permissions;
    }

    /**
     * @brief Save the data.
     */
    void saveData() {
        if (funcSaveData == nullptr) {
            throw std::runtime_error("Function not found");
        }
        funcSaveData();
    }

};

#else

class PermissionAPI {

public:

    /**
     * @brief Create a Role object.
     *
     * @param  name         The name of the role.
     * @param  displayName  The display name of the role.
     * @return std::weak_ptr<PERM::Role>  The created role(weak ref).
     * @throws std::invalid_argument      If the role already exists.
     * @par Example
     * @code
     * PermissionAPI api;
     * auto role = api.createRole("role1", "Role 1");
     * ...
     * if (!role.expired()) {
     *     auto rolePtr = role.lock();
     *     rolePtr->addMember("1145141919810");
     *     api.saveData();
     * }
     * @endcode
     */
    static PERMAPI std::weak_ptr<PERM::Role> createRole(const std::string& name, const std::string& displayName);

    /**
     * @brief Check if a role exists.
     *
     * @param  name  The name of the role.
     * @return bool  True If the role exists, false otherwise.
     */
    static PERMAPI bool roleExists(const std::string& name);

    /**
     * @brief Get a role object.
     *
     * @param  name                       The name of the role.
     * @return std::weak_ptr<PERM::Role>  The role(weak ref).
     * @throws std::invalid_argument      If the role does not exist.
     */
    static PERMAPI std::weak_ptr<PERM::Role> getRole(const std::string& name);

    /**
     * @brief Get a role object. If the role does not exist, it will be created.
     *
     * @param  name                       The name of the role.
     * @return std::weak_ptr<PERM::Role>  The role(weak ref).
     */
    static PERMAPI std::weak_ptr<PERM::Role> getOrCreateRole(const std::string& name);

    /**
     * @brief Register an permission.
     *
     * @param name  The name of the permission.
     * @param desc  The description name of the permission.
     */
    static PERMAPI void registerPermission(const std::string& name, const std::string& desc);

    /**
     * @brief Delete a permission.
     *
     * @param    name  The name of the permission.
     * @warning  This function will also delete the permission instances in roles.
     */
    static PERMAPI void deletePermission(const std::string& name);

    /**
     * @brief Check if a permission exists.
     *
     * @param  name  The name of the permission.
     * @return bool  True If the permission exists, false otherwise.
     */
    static PERMAPI bool permissionExists(const std::string& name);

    /**
     * @brief Check whether the player has the Permission or not.
     *
     * @param  xuid  The xuid of the player.
     * @param  name  The name of the Permission.
     * @return bool  True If the player has the Permission, false otherwise.
     */
    static PERMAPI bool checkPermission(const xuid_t& xuid, const std::string& name);

    /**
     * @brief Check if a player is a member of a role.
     *
     * @param  xuid  The xuid of the player.
     * @param  name  The name of the role.
     * @return bool  True If the player is a member of the role, false otherwise.
     */
    static PERMAPI bool isMemberOf(const xuid_t& xuid, const std::string& name);

    /**
     * @brief Get the roles of a player.
     *
     * @param  xuid         The xuid of the player.
     * @return PERM::Roles  The roles of the player.
     */
    static PERMAPI PERM::Roles getPlayerRoles(const xuid_t& xuid);

    /**
     * @brief Get the permissions of a player.
     *
     * @param  xuid               The xuid of the player.
     * @return PERM::Permissions  The permissions of the player.
     */
    static PERMAPI PERM::Permissions getPlayerPermissions(const xuid_t& xuid);

    /**
     * @brief Save the data.
     */
    static PERMAPI void saveData();
};

#endif