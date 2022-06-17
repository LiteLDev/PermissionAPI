#include <Utils/FileHelper.h>
#include "Data.h"

#define SEARCH_VECTOR(vector, value) (std::find(vector.begin(), vector.end(), value) != vector.end())

nlohmann::json Permission::defaultData = {
    {"abilityInfo", {}},
    {"groups", {
        {"everyone", {
            {"abilities", {}},
            {"members", {}},
            {"priority", 0}
        }}
    }}
};

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
            // AbilityInfo
            auto& jAbilityInfo = j["abilityInfo"];
            for (auto it = jAbilityInfo.begin(); it < jAbilityInfo.end(); it++) {
                PermAbilityInfo info;
                info.name = it.key();
                info.desc = it.value()["description"];
                this->abilityInfo.push_back(info);
            }
            // AbilityGroups
            auto& jGroups = j["groups"];
            for (auto it = jGroups.begin(); it < jGroups.end(); it++) {
                PermGroup group;
                group.name = it.key();
                it.value()["members"].get_to(group.members);
                it.value()["priority"].get_to(group.priority);
                auto& jAbilities = it.value()["abilities"];
                for (auto ablt = jAbilities.begin(); ablt < jAbilities.end(); ablt++) {
                    PermAbility ab;
                    ab.name = ablt.key();
                    auto& val = ablt.value();
                    val["enabled"].get_to(ab.enabled);
                    ab.raw = val;
                    group.abilities[ab.name] = ab;
                }
                this->groups[group.name] = group;
            }
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
        auto j = nlohmann::json();
        // AbilityInfo
        auto& jAbilities = j["abilities"];
        for (auto& info : this->abilityInfo) {
            jAbilities[info.name] = {
                {"description", info.desc}
            };
        }
        // AbilityGroups
        auto& jGroups = j["groups"];
        for (auto& [name, group] : this->groups) {
            auto& g = jGroups[name];
            g["members"] = group.members;
            g["priority"] = group.priority;
            auto& jAbilities = g["abilities"];
            for (auto& [name, ability] : group.abilities) {
                if (ability.enabled) {
                    ability.raw["enabled"] = true;
                }
                jAbilities[name] = ability.raw;
            }
        }
        WriteAllFile(DATA_FILE, j.dump(4));
    } catch (const std::exception& e) {
        logger.error("Failed to save the data file: {}", e.what());
    }
}

void Permission::registerAbility(const std::string& nspc, const std::string& name, const std::string& desc) {
    // Check validity
    if (nspc.empty() || name.empty() || desc.empty() ||
        nspc.find(':') != std::string::npos  // namespace cannot contain ':'
        ) {
        throw std::invalid_argument("Failed to register the ability: invalid arguments");
        return;
    }
    // Check if the ability already exists
    for (auto& i : this->abilityInfo) {
        if (i.name == name) {
            throw std::invalid_argument("Failed to register the ability: the ability already exists");
            return;
        }
    }
    PermAbilityInfo info;
    info.name = nspc + ':' + name;
    info.desc = desc;
    this->abilityInfo.push_back(info);
    this->save();
}

PermAbilityInfo Permission::getAbilityInfo(const std::string& name) const {
    for (auto& i : this->abilityInfo) {
        if (i.name == name) {
            return i;
        }
    }
    throw std::out_of_range("Failed to get the ability info: the ability does not exist");
}

PermGroup& Permission::createGroup(const std::string& name, const std::vector<xuid_t>& members) {
    // Check validity
    if (name.empty()) {
        throw std::invalid_argument("Failed to create the group: invalid arguments");
    }
    // Check if the group already exists
    for (auto& i : this->groups) {
        if (i.first == name) {
            throw std::invalid_argument("Failed to create the group: the group already exists");
        }
    }
    PermGroup group;
    group.name = name;
    group.members = members;
    this->groups[name] = group;
    this->save();
    return this->groups[name];
}

PermGroup& Permission::getGroup(const std::string& name) {
    auto it = this->groups.find(name);
    if (it == this->groups.end()) {
        throw std::out_of_range("Failed to get the group: the group does not exist");
    }
    return it->second;
}

bool Permission::checkAbility(const xuid_t& xuid, const std::string& name) const {
    int currentPriority = INT_MIN;
    bool result = false;
    // Special case: everyone
    if (this->groups.count("everyone") && this->groups.at("everyone").priority >= currentPriority) {
        auto& group = this->groups.at("everyone");
        if (group.abilities.count(name)) {
            // Special case: if the priority is the same, result = (result && enabled)
            if (group.priority == currentPriority) {
                result = (result && group.abilities.at(name).enabled);
            } else {
                currentPriority = group.priority;
                result = this->groups.at("everyone").abilities.at(name).enabled;
            }
        }
    }
    for (auto& [name, group] : this->groups) {
        if (group.priority < currentPriority) {
            continue;
        }
        if (SEARCH_VECTOR(group.members, xuid)) { // If is in the group
            if (group.abilities.count(name)) {
                // Special case: if the priority is the same, result = (result && enabled)
                if (group.priority == currentPriority) {
                    result = (result && group.abilities.at(name).enabled);
                } else {
                    currentPriority = group.priority;
                    result = group.abilities.at(name).enabled;
                }
            }
        }
    }
    return result;
}

bool Permission::checkIsMember(const xuid_t& xuid, const std::string& groupName) const {
    auto it = this->groups.find(groupName);
    if (it == this->groups.end()) {
        throw std::out_of_range("Failed to check if the user is in the group: the group does not exist");
    }
    return SEARCH_VECTOR(it->second.members, xuid);
}