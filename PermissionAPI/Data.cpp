#include <Utils/FileHelper.h>
#include "Data/JsonHelper.hpp"
#include "Data.h"

nlohmann::json Permission::defaultData = {
    {"abilitiesInfo", {}},
    {"groups", {
        {"everyone", {
            {"displayName", "§7everyone"},
            {"abilities", {}},
            {"priority", 0}
        }},
        {"admin", {
            {"abilities", {}},
            {"members", {}},
            {"priority", 2147483647}
        }}
    }}
};

bool Permission::validateData() {
    bool result = true;
    for (auto& group : this->groups) {
        auto oldName = group.name;
        auto changed = group.validate();
        if (changed) {
            logger.warn("Group name '{}' contains invalid characters.", group.name);
            logger.warn("Group name '{}' has been replaced with '{}'.", oldName, group.name);
        }
    }
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

void Permission::registerAbility(const std::string& nspc, const std::string& name, const std::string& desc) {
    if (nspc.find_first_of(":") != std::string::npos) {
        throw std::invalid_argument("The namespace cannot contain ':'");
    }
    auto fullName = nspc + ":" + name;
    if (this->abilitiesInfo.find(fullName) != this->abilitiesInfo.end()) {
        throw std::invalid_argument("The ability already exists");
    }
    this->abilitiesInfo[fullName] = PermAbilityInfo{fullName, desc};
}

bool Permission::checkAbility(const xuid_t& xuid, const std::string& name) const {
    return this->getPlayerAbilities(xuid).contains(name);
}

bool Permission::isMemberOf(const xuid_t& xuid, const std::string& groupName) const {
    return this->groups.contains(groupName) && this->groups.at(groupName).hasMember(xuid);
}

PermGroups Permission::getPlayerGroups(const xuid_t& xuid) const {
    PermGroups result;
    for (auto& group : groups) {
        if (group.hasMember(xuid)) {
            result.push_back(group);
        }
    }
    return result.sortByPriority(true);
}

PermAbilities Permission::getPlayerAbilities(const xuid_t& xuid) const {
    PermAbilities result;
    auto playerGroups = this->getPlayerGroups(xuid);
    for (auto& group : playerGroups.sortByPriority()) {
        for (auto& ability : group.abilities) {
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