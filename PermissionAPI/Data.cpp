#include "pch.h"
#include <Utils/FileHelper.h>
#include "Data/JsonHelper.hpp"
#include "Mod.h"

void PERM::Role::setPermission(const std::string& name, bool enabled, const nlohmann::json& extra) {
    if (!mod.perm.permInfoList.contains(name)) {
        throw std::invalid_argument("Permission " + name + " is not registered!");
    }
    if (!this->permissions.contains(name)) {
        this->permissions.push_back({name, enabled, extra});
    } else {
        this->permissions.at(name).enabled = enabled;
        if (!extra.is_null()) {
            if (extra.is_object()) {
                this->permissions.at(name).extra = extra;
            } else {
                throw std::runtime_error("Failed to set the permission: the extra data is not a json object");
            }
        }
    }
}

using namespace PERM;

nlohmann::json Permission::defaultData = {
    {"permissions", nlohmann::json::object()},
    {"roles", {
        {"everyone", {
            {"displayName", "§7everyone"}, 
            {"permissions", nlohmann::json::object()},
            {"priority", 0}
        }},
        {"admin", {
            {"displayName", "§cadmin"}, 
            {"permissions", nlohmann::json::object()},
            {"members", nlohmann::json::array()},
            {"priority", 2147483647}
        }}
    }}
};

bool Permission::validateData() {
    bool result = false;
    for (auto& role : this->roles) {
        auto oldName = role->name;
        auto changed = role->validate();
        if (changed) {
            logger.warn(tr("Role name '{}' contains invalid characters.", oldName));
            logger.warn(tr("Role name '{}' has been replaced with '{}'.", oldName, role->name));
            result = true;
        }
        for (const auto& ab : role->getPermissions()) {
            if (!permInfoList.contains(ab.name)) {
                permInfoList.push_back({ab.name, ""});
                result = true;
            }
        }
    }
    return result;
}

void Permission::load() {
    if (!fs::exists(DATA_FILE)) {
        fs::create_directories(PLUGIN_DIR);
        std::fstream file(DATA_FILE, std::ios::out | std::ios::app);
        file << std::setw(4) << defaultData;
    }
    auto res = ReadAllFile(DATA_FILE);
    if (res.has_value()) {
        try {
            auto j = nlohmann::json::parse(res.value());
            from_json(j["permissions"], this->permInfoList);
            from_json(j["roles"], this->roles);
            this->validateData();
        } catch (const std::exception& e) {
            logger.error(tr("permapi.data.process.fail", e.what()));
        }
    } else {
        logger.error(tr("permapi.data.read.fail", DATA_FILE));
        return;
    }
}

void Permission::save() {
    try {
        this->validateData();
        nlohmann::json j = nlohmann::json::object();
        to_json(j["permissions"], this->permInfoList);
        to_json(j["roles"], this->roles);
        WriteAllFile(DATA_FILE, j.dump(4));
    } catch (const std::exception& e) {
        logger.error(tr("permapi.data.save.fail", e.what()));
    }
}

std::shared_ptr<Role> Permission::createRole(const std::string& name, const std::string& displayName) {
    if (this->roles.contains(name)) {
        throw std::invalid_argument("Role already exists");
    }
    if (!Role::isValidRoleName(name)) {
        throw std::invalid_argument("Invalid role name: " + name);
    }
    Role* role = nullptr;
    if (name == "everyone") {
        role = new EveryoneRole;
    } else if (name == "admin") {
        role = new AdminRole;
    } else {
        role = new GeneralRole;
    }
    role->name = name;
    role->displayName = displayName;
    auto& ret = this->roles[name] = std::shared_ptr<Role>(role);
    save();
    return ret;
}

std::shared_ptr<Role> Permission::getRole(const std::string& name) {
    if (!this->roles.contains(name)) {
        throw std::invalid_argument("Role not found");
    }
    return this->roles.at(name);
}

std::shared_ptr<Role> Permission::getOrCreateRole(const std::string& name) {
    return (this->roles.contains(name) ? getRole(name) : createRole(name, name));
}

void Permission::deleteRole(const std::string& name) {
    if (!this->roles.contains(name)) {
        throw std::invalid_argument("Role not found");
    }
    this->roles.remove(name);
}

void Permission::registerPermission(const std::string& name, const std::string& desc) {
    if (!PermInstance::isValidPermissionName(name)) {
        throw std::invalid_argument("Invalid permission name: " + name);
    }
    if (this->permInfoList.contains(name)) {
        throw std::invalid_argument("The permission already exists");
    }
    this->permInfoList[name] = PermInfo{name, desc};
    this->save();
}

void Permission::deletePermission(const std::string& name) {
    if (!this->permInfoList.contains(name)) {
        throw std::invalid_argument("Permission not found");
    }
    this->permInfoList.remove(name);
    for (auto& role : this->roles) {
        if (role->getType() != Role::Type::Admin) {
            if (role->permissionExists(name)) {
                role->removePermission(name);
            }
        }
    }
    this->save();
}

bool Permission::checkPermission(const xuid_t& xuid, const std::string& name) const {
    return this->getPlayerPermissions(xuid).contains(name);
}

bool Permission::isMemberOf(const xuid_t& xuid, const std::string& roleName) const {
    return this->roles.contains(roleName) && this->roles.at(roleName)->hasMember(xuid);
}

Roles Permission::getPlayerRoles(const xuid_t& xuid) const {
    Roles result;
    for (auto& role : roles) {
        if (role->hasMember(xuid)) {
            result.push_back(role);
        }
    }
    return result.sortByPriority(true);
}

Permissions Permission::getPlayerPermissions(const xuid_t& xuid) const {
    Permissions result;
    auto playerRoles = this->getPlayerRoles(xuid);
    for (auto& role : playerRoles.sortByPriority()) {
        switch (role->getType()) {
            case Role::Type::Admin: {
                for (const auto& perm : role->getPermissions()) {
                    if (perm.enabled) {
                        if (result.contains(perm.name)) {
                            auto& ext = result[perm.name].extra;
                            if (ext.is_object()) {
                                ext.merge_patch(perm.extra);
                            } else {
                                ext = perm.extra;
                            }
                        } else {
                            result.push_back(perm);
                        }
                    } else {
                        if (result.contains(perm.name)) {
                            result.remove(perm.name);
                        }
                    }
                }
                for (auto& perm : this->permInfoList) {
                    if (!result.contains(perm.name)) {
                        result.push_back({perm.name, true});
                    }
                }
                break;
            }
            default: {
                for (const auto& perm : role->getPermissions()) {
                    if (perm.enabled) {
                        if (result.contains(perm.name)) {
                            auto& ext = result[perm.name].extra;
                            if (ext.is_object()) {
                                ext.merge_patch(perm.extra);
                            } else {
                                ext = perm.extra;
                            }
                        } else {
                            result.push_back(perm);
                        }
                    } else {
                        if (result.contains(perm.name)) {
                            result.remove(perm.name);
                        }
                    }
                }
            }
        }
    }
    return result;
}