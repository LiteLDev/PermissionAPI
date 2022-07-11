#include "pch.h"
#include <Utils/FileHelper.h>
#include "Data/JsonHelper.hpp"
#include "Data.h"

nlohmann::json Permission::defaultData = {
    {"abilitiesInfo", nlohmann::json::object()},
    {"groups", {
        {"everyone", {
            {"displayName", "§7everyone"}, 
            {"abilities", nlohmann::json::object()},
            {"priority", 0}
        }},
        {"admin", {
            {"displayName", "§cadmin"}, 
            {"abilities", nlohmann::json::object()},
            {"members", nlohmann::json::array()},
            {"priority", 2147483647}
        }}
    }}
};

bool Permission::validateData() {
    bool result = false;
    for (auto& group : this->groups) {
        auto oldName = group->name;
        auto changed = group->validate();
        if (changed) {
            logger.warn("Group name '{}' contains invalid characters.", oldName);
            logger.warn("Group name '{}' has been replaced with '{}'.", oldName, group->name);
            result = true;
        }
        for (auto& ab : group->abilities) {
            if (!abilitiesInfo.contains(ab.name)) {
                abilitiesInfo.push_back({ ab.name, "" });
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
            // AbilitiesInfo
            j["abilitiesInfo"].get_to(this->abilitiesInfo);
            // Groups
            j["groups"].get_to(this->groups);
            this->validateData();
        } catch (const std::exception& e) {
            logger.error("Failed to process the data file: {}", e.what());
        }
    } else {
        logger.error("Failed to read the data file at {}", DATA_FILE);
        return;
    }
}

void Permission::save() {
    try {
        this->validateData();
        nlohmann::json j{
            {"abilitiesInfo", this->abilitiesInfo},
            {"groups", this->groups}
        };
        WriteAllFile(DATA_FILE, j.dump(4));
    } catch (const std::exception& e) {
        logger.error("Failed to save the data file: {}", e.what());
    }
}

std::shared_ptr<PermGroup> Permission::createGroup(const std::string& name, const std::string& displayName) {
    if (this->groups.contains(name)) {
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
    auto& ret = this->groups[name] = std::shared_ptr<PermGroup>(group);
    save();
    return ret;
}

std::shared_ptr<PermGroup> Permission::getGroup(const std::string& name) {
    if (!this->groups.contains(name)) {
        throw std::invalid_argument("Group not found");
    }
    return this->groups.at(name);
}

std::shared_ptr<PermGroup> Permission::getOrCreateGroup(const std::string& name) {
    return (this->groups.contains(name) ? getGroup(name) : createGroup(name, name));
}

void Permission::registerAbility(const std::string& name, const std::string& desc) {
    if (!PermAbility::isValidAbilityName(name)) {
        throw std::invalid_argument("Invalid ability name: " + name);
    }
    if (this->abilitiesInfo.contains(name)) {
        throw std::invalid_argument("The ability already exists");
    }
    this->abilitiesInfo[name] = PermAbilityInfo{name, desc};
    this->save();
}

void Permission::deleteAbility(const std::string& name) {
    if (!this->abilitiesInfo.contains(name)) {
        throw std::invalid_argument("Ability not found");
    }
    this->abilitiesInfo.remove(name);
    for (auto& group : this->groups) {
        if (group->getType() != PermGroup::Type::Admin) {
            if (group->abilityDefined(name)) {
                group->removeAbility(name);
            }
        }
    }
    this->save();
}

bool Permission::checkAbility(const xuid_t& xuid, const std::string& name) const {
    return this->getPlayerAbilities(xuid).contains(name);
}

bool Permission::isMemberOf(const xuid_t& xuid, const std::string& groupName) const {
    return this->groups.contains(groupName) && this->groups.at(groupName)->hasMember(xuid);
}

PermGroups Permission::getPlayerGroups(const xuid_t& xuid) const {
    PermGroups result;
    for (auto& group : groups) {
        if (group->hasMember(xuid)) {
            result.push_back(group);
        }
    }
    return result.sortByPriority(true);
}

PermAbilities Permission::getPlayerAbilities(const xuid_t& xuid) const {
    PermAbilities result;
    if (this->isMemberOf(xuid, "admin")) {
        for (auto& info : abilitiesInfo) {
            result.push_back(PermAbility{info.name, true});
        }
        return result;
    }
    auto playerGroups = this->getPlayerGroups(xuid);
    for (auto& group : playerGroups.sortByPriority()) {
        for (auto& ability : group->abilities) {
            if (ability.enabled) {
                if (result.contains(ability.name)) {
                    auto& ext = result[ability.name].extra;
                    if (ext.is_object()) ext.merge_patch(ability.extra);
                    else ext = ability.extra;
                } else {
                    result.push_back(ability);
                }
            } else {
                if (result.contains(ability.name))
                    result.remove(ability.name);
            }
        }
    }
    return result;
}