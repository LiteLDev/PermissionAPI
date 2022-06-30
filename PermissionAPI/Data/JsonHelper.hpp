#pragma once
#include "PermGroup.hpp"

template <typename T>
void from_json(const nlohmann::json& j, PermVector<T>& v);
template <typename T>
void to_json(nlohmann::json& j, const PermVector<T>& v);

template <typename T>
void from_json(const nlohmann::json& j, PermContainer<T>& v);
template <typename T>
void to_json(nlohmann::json& j, const PermContainer<T>& v);

void from_json(const nlohmann::json& j, PermAbility& v);
void to_json(nlohmann::json& j, const PermAbility& v);

void from_json(const nlohmann::json& j, PermAbilities& v);
void to_json(nlohmann::json& j, const PermAbilities& v);

void from_json(const nlohmann::json& j, PermAbilitiesInfo& v);
void to_json(nlohmann::json& j, const PermAbilitiesInfo& v);

void from_json(const nlohmann::json& j, GeneralPermGroup& v);
void to_json(nlohmann::json& j, const GeneralPermGroup& v);

void from_json(const nlohmann::json& j, EveryonePermGroup& v);
void to_json(nlohmann::json& j, const EveryonePermGroup& v);

void from_json(const nlohmann::json& j, AdminPermGroup& v);
void to_json(nlohmann::json& j, const AdminPermGroup& v);

void from_json(const nlohmann::json& j, PermGroups& v);
void to_json(nlohmann::json& j, const PermGroups& v);



// PermVector<T>
template <typename T>
inline void from_json(const nlohmann::json& j, PermVector<T>& v) {
    v = j.get<std::vector<T>>();
}
template <typename T>
inline void to_json(nlohmann::json& j, const PermVector<T>& v) {
    j = (std::vector<T>)v;
}

// PermContainer<T>
template <typename T>
inline void from_json(const nlohmann::json& j, PermContainer<T>& v) {
    v = j.get<std::vector<T>>();
}
template <typename T>
inline void to_json(nlohmann::json& j, const PermContainer<T>& v) {
    j = (std::vector<T>)v;
}

// PermAbility
inline void from_json(const nlohmann::json& j, PermAbility& v) {
    if (j.is_boolean()) { // convenience
        v.enabled = j.get<bool>();
        return;
    }
    if (j.contains("enabled")) {
        v.enabled = j["enabled"];
        auto copy = j;
        copy.erase("enabled");
        v.extra = copy;
    } else {
        throw std::runtime_error("Failed to load the ability: the json object does not contain the 'enabled' field");
    }
}
inline void to_json(nlohmann::json& j, const PermAbility& v) {
    j["enabled"] = v.enabled;
    if (v.extra.is_object() && !v.extra.empty()) {
        j.merge_patch(v.extra);
    }
}

// PermAbilities
inline void from_json(const nlohmann::json& j, PermAbilities& v) {
    v.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        auto a = it.value().get<PermAbility>();
        a.name = it.key();
        v.push_back(a);
    }
}
inline void to_json(nlohmann::json& j, const PermAbilities& v) {
    for (auto& it : v) {
        j[it.name] = it;
    }
}

// PermAbilitiesInfo
inline void from_json(const nlohmann::json& j, PermAbilitiesInfo& v) {
    v.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        PermAbilityInfo info;
        info.name = it.key();
        info.desc = it.value()["desc"];
    }
}
inline void to_json(nlohmann::json& j, const PermAbilitiesInfo& v) {
    for (auto& it : v) {
        j[it.name] = {
            {"desc", it.desc}
        };
    }
}

// GeneralPermGroup
inline void from_json(const nlohmann::json& j, GeneralPermGroup& v) {
    if (j.contains("members")) v.members = j["members"].get<PermMembers>();
    if (j.contains("priority")) v.priority = j["priority"];
    if (j.contains("displayName")) v.displayName = j["displayName"];
    if (j.contains("abilities")) v.abilities = j["abilities"].get<PermAbilities>();
}
inline void to_json(nlohmann::json& j, const GeneralPermGroup& v) {
    j["members"] = v.members;
    j["priority"] = v.priority;
    j["displayName"] = v.displayName;
    j["abilities"] = v.abilities;
}

// EveryonePermGroup
inline void from_json(const nlohmann::json& j, EveryonePermGroup& v) {
    if (j.contains("priority")) v.priority = j["priority"];
    if (j.contains("displayName")) v.displayName = j["displayName"];
    if (j.contains("abilities")) v.abilities = j["abilities"].get<PermAbilities>();
}
inline void to_json(nlohmann::json& j, const EveryonePermGroup& v) {
    j["priority"] = v.priority;
    j["displayName"] = v.displayName;
    j["abilities"] = v.abilities;
}

// AdminPermGroup
inline void from_json(const nlohmann::json& j, AdminPermGroup& v) {
    if (j.contains("members")) v.members = j["members"].get<PermMembers>();
    if (j.contains("priority")) v.priority = j["priority"];
    if (j.contains("displayName")) v.displayName = j["displayName"];
    if (j.contains("abilities")) v.abilities = j["abilities"].get<PermAbilities>();
}
inline void to_json(nlohmann::json& j, const AdminPermGroup& v) {
    j["members"] = v.members;
    j["priority"] = v.priority;
    j["displayName"] = v.displayName;
    j["abilities"] = v.abilities;
}

// PermGroups
inline void from_json(const nlohmann::json& j, PermGroups& v) {
    v.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        auto& name = it.key();
        if (name == "everyone") {
            EveryonePermGroup group;
            from_json(it.value(), group);
            group.name = name;
            v.push_back(group);
        } else if (name == "admin") {
            AdminPermGroup group;
            from_json(it.value(), group);
            group.name = name;
            v.push_back(group);
        } else {
            GeneralPermGroup group;
            from_json(it.value(), group);
            group.name = name;
            v.push_back(group);
        }
        if (v.back().name.empty()) {
            throw std::runtime_error("Failed to load the perm group: the name of the group is empty!");
        }
        if (v.back().displayName.empty()) {
            v.back().displayName = name;
        }
    }
}
inline void to_json(nlohmann::json& j, const PermGroups& v) {
    for (auto& it : v) {
        switch (it.getType()) {
            case PermGroup::Type::General:
                j[it.name] = *(GeneralPermGroup*)it.get();
                break;
            case PermGroup::Type::Admin:
                j[it.name] = *(AdminPermGroup*)it.get();
                break;
            case PermGroup::Type::Everyone:
                j[it.name] = *(EveryonePermGroup*)it.get();
                break;
            default:
                break;
        }
    }
}
