#pragma once
#include "Role.hpp"

template <typename T>
void from_json(const nlohmann::json& j, PERM::PermVector<T>& v);
template <typename T>
void to_json(nlohmann::json& j, const PERM::PermVector<T>& v);

template <typename T>
void from_json(const nlohmann::json& j, PERM::PermContainer<T>& v);
template <typename T>
void to_json(nlohmann::json& j, const PERM::PermContainer<T>& v);

void from_json(const nlohmann::json& j, PERM::PermInstance& v);
void to_json(nlohmann::json& j, const PERM::PermInstance& v);

void from_json(const nlohmann::json& j, PERM::Permissions& v);
void to_json(nlohmann::json& j, const PERM::Permissions& v);

void from_json(const nlohmann::json& j, PERM::PermInfoList& v);
void to_json(nlohmann::json& j, const PERM::PermInfoList& v);

void from_json(const nlohmann::json& j, PERM::GeneralRole& v);
void to_json(nlohmann::json& j, const PERM::GeneralRole& v);

void from_json(const nlohmann::json& j, PERM::EveryoneRole& v);
void to_json(nlohmann::json& j, const PERM::EveryoneRole& v);

void from_json(const nlohmann::json& j, PERM::AdminRole& v);
void to_json(nlohmann::json& j, const PERM::AdminRole& v);

void from_json(const nlohmann::json& j, PERM::Roles& v);
void to_json(nlohmann::json& j, const PERM::Roles& v);



// PermVector<T>
template <typename T>
inline void from_json(const nlohmann::json& j, PERM::PermVector<T>& v) {
    v = j.get<std::vector<T>>();
}
template <typename T>
inline void to_json(nlohmann::json& j, const PERM::PermVector<T>& v) {
    j = (std::vector<T>)v;
}

// PermContainer<T>
template <typename T>
inline void from_json(const nlohmann::json& j, PERM::PermContainer<T>& v) {
    v = j.get<std::vector<T>>();
}
template <typename T>
inline void to_json(nlohmann::json& j, const PERM::PermContainer<T>& v) {
    j = (std::vector<T>)v;
}

// PermInstance
inline void from_json(const nlohmann::json& j, PERM::PermInstance& v) {
    if (j.is_boolean()) {
        v.enabled = j.get<bool>();
        return;
    }
    if (j.contains("enabled")) {
        v.enabled = j["enabled"];
        auto copy = j;
        copy.erase("enabled");
        v.extra = copy;
    } else {
        throw std::runtime_error("Failed to load the permission instance: the json object does not contain the 'enabled' field");
    }
}
inline void to_json(nlohmann::json& j, const PERM::PermInstance& v) {
    if (!v.extra.is_object() || (v.extra.is_object() && v.extra.empty())) {
        j = v.enabled;
        return;
    }
    j["enabled"] = v.enabled;
    if (v.extra.is_object() && !v.extra.empty()) {
        j.merge_patch(v.extra);
    }
}

// Permissions
inline void from_json(const nlohmann::json& j, PERM::Permissions& v) {
    v.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        PERM::PermInstance a;
        from_json(it.value(), a);
        a.name = it.key();
        v.push_back(a);
    }
}
inline void to_json(nlohmann::json& j, const PERM::Permissions& v) {
    j = nlohmann::json::object();
    for (auto& it : v) {
        to_json(j[it.name], it);
    }
}

// PermInfoList
inline void from_json(const nlohmann::json& j, PERM::PermInfoList& v) {
    v.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        PERM::PermInfo info;
        info.name = it.key();
        info.desc = it.value()["desc"];
    }
}
inline void to_json(nlohmann::json& j, const PERM::PermInfoList& v) {
    j = nlohmann::json::object();
    for (auto& it : v) {
        j[it.name] = {
            {"desc", it.desc}
        };
    }
}

// GeneralRole
inline void from_json(const nlohmann::json& j, PERM::GeneralRole& v) {
    if (j.contains("members")) v.getMembers() = j["members"].get<PERM::Members>();
    if (j.contains("priority")) v.priority = j["priority"];
    if (j.contains("displayName")) v.displayName = j["displayName"];
    if (j.contains("permissions")) from_json(j["permissions"], v.getPermissions());
}
inline void to_json(nlohmann::json& j, const PERM::GeneralRole& v) {
    j["members"] = v.getMembers();
    j["priority"] = v.priority;
    j["displayName"] = v.displayName;
    to_json(j["permissions"], v.getPermissions());
}

// EveryoneRole
inline void from_json(const nlohmann::json& j, PERM::EveryoneRole& v) {
    if (j.contains("priority")) v.priority = j["priority"];
    if (j.contains("displayName")) v.displayName = j["displayName"];
    if (j.contains("permissions")) from_json(j["permissions"], v.getPermissions());
}
inline void to_json(nlohmann::json& j, const PERM::EveryoneRole& v) {
    j["priority"] = v.priority;
    j["displayName"] = v.displayName;
    to_json(j["permissions"], v.getPermissions());
}

// AdminRole
inline void from_json(const nlohmann::json& j, PERM::AdminRole& v) {
    if (j.contains("members")) v.getMembers() = j["members"].get<PERM::Members>();
    if (j.contains("priority")) v.priority = j["priority"];
    if (j.contains("displayName")) v.displayName = j["displayName"];
    if (j.contains("permissions")) from_json(j["permissions"], v.getPermissions());
}
inline void to_json(nlohmann::json& j, const PERM::AdminRole& v) {
    j["members"] = v.getMembers();
    j["priority"] = v.priority;
    j["displayName"] = v.displayName;
    to_json(j["permissions"], v.getPermissions());
}

// Roles
inline void from_json(const nlohmann::json& j, PERM::Roles& v) {
    v.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        auto& name = it.key();
        if (name == "everyone") {
            PERM::EveryoneRole role;
            from_json(it.value(), role);
            role.name = name;
            v.push_back(std::make_shared<PERM::EveryoneRole>(role));
        } else if (name == "admin") {
            PERM::AdminRole role;
            from_json(it.value(), role);
            role.name = name;
            v.push_back(std::make_shared<PERM::AdminRole>(role));
        } else {
            PERM::GeneralRole role;
            from_json(it.value(), role);
            role.name = name;
            v.push_back(std::make_shared<PERM::GeneralRole>(role));
        }
        if (v.back()->name.empty()) {
            throw std::runtime_error("Failed to load the perm role: the name of the role is empty!");
        }
        if (v.back()->displayName.empty()) {
            v.back()->displayName = name;
        }
    }
}
inline void to_json(nlohmann::json& j, const PERM::Roles& v) {
    for (auto& it : v) {
        switch (it->getType()) {
            case PERM::Role::Type::General:
                to_json(j[it->name], *(PERM::GeneralRole*)it.get());
                break;
            case PERM::Role::Type::Admin:
                to_json(j[it->name], *(PERM::AdminRole*)it.get());
                break;
            case PERM::Role::Type::Everyone:
                to_json(j[it->name], *(PERM::EveryoneRole*)it.get());
                break;
            default:
                break;
        }
    }
}
