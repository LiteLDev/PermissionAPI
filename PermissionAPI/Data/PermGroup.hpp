#pragma once
#include "Foundation.hpp"
#include "PermMember.hpp"
#include "PermAbility.hpp"

class PermGroup {

public:

    enum class Type : char {
        None = 0,
        General = 1,
        Everyone = 2,
        Admin = 3
    };

    PermMembers members;
    PermAbilities abilities;
    std::string name;
    std::string displayName;
    int priority = 0;

    static constexpr std::string_view groupNameInvalidChars = "@#[]{}<>()/|\\$%^&*!~`\"\'+=?\n\t\r\f\v ";

    virtual ~PermGroup() = default;

    virtual bool hasAbility(const std::string& name) const {
        return this->abilities.contains(name) && this->abilities.at(name).enabled;
    }
    virtual void setAbility(const std::string& name, bool enabled = true, const nlohmann::json& extra = nlohmann::json()) {
        if (!this->abilities.contains(name)) {
            this->abilities.push_back({name, enabled, extra});
        } else {
            this->abilities.at(name).enabled = enabled;
            if (!extra.is_null()) {
                if (extra.is_object()) {
                    this->abilities.at(name).extra = extra;
                } else {
                    throw std::runtime_error("Failed to set the ability: the extra data is not a json object");
                }
            }
        }
    }
    virtual void removeAbility(const std::string& name) {
        this->abilities.remove(name);
    }
    virtual bool abilityDefined(const std::string& name) {
        return this->abilities.contains(name);
    }

    virtual bool hasMember(const xuid_t& xuid) const = 0;
    virtual void addMember(const xuid_t& xuid) = 0;
    virtual void removeMember(const xuid_t& xuid) = 0;


    virtual PermMembers& getMembers() {
        return this->members;
    }
    virtual Type getType() const = 0;

    /**
     * @brief Validate the group data
     * 
     * @return bool  True if changed, false otherwise.
     */
    virtual bool validate() {
        if (this->name.find_first_of(PermGroup::groupNameInvalidChars.data()) != std::string::npos) {
            for (auto& ch : this->name) {
                if (PermGroup::groupNameInvalidChars.find(ch) != std::string::npos) {
                    ch = '-';
                }
            }
            return true;
        }
        return false;
    }

    static bool isValidGroupName(const std::string& name) {
        return name.find_first_of(PermGroup::groupNameInvalidChars.data()) == std::string::npos;
    }

};

class GeneralPermGroup : public PermGroup {

    using Base = PermGroup;

public:

    virtual bool hasMember(const xuid_t& xuid) const {
        return this->members.contains(xuid);
    }
    virtual void addMember(const xuid_t& xuid) {
        this->members.push_back(xuid);
    }
    virtual void removeMember(const xuid_t& xuid) {
        this->members.erase(std::remove(this->members.begin(), this->members.end(), xuid), this->members.end());
    }

    virtual Type getType() const {
        return Type::General;
    }

};

class EveryonePermGroup : public PermGroup {

    using Base = PermGroup;

public:

    virtual bool hasMember(const xuid_t& xuid) const {
        return true;
    }
    virtual void addMember(const xuid_t& xuid) {
        throw std::runtime_error("You cannot add a member to a everyone permission group");
    }
    virtual void removeMember(const xuid_t& xuid) {
        throw std::runtime_error("You cannot remove a member from a everyone permission group");
    }

    virtual Type getType() const {
        return Type::Everyone;
    }

};

class AdminPermGroup : public PermGroup {

    using Base = PermGroup;

public:

    virtual bool hasAbility(const std::string& name) const {
        if (!this->abilities.contains(name)) {
            return true;
        } else {
            return this->abilities.at(name).enabled;
        }
    }

    virtual bool hasMember(const xuid_t& xuid) const {
        return this->members.contains(xuid);
    }
    virtual void addMember(const xuid_t& xuid) {
        this->members.push_back(xuid);
    }
    virtual void removeMember(const xuid_t& xuid) {
        this->members.erase(std::remove(this->members.begin(), this->members.end(), xuid), this->members.end());
    }

    virtual Type getType() const {
        return Type::Admin;
    }

};

class PermGroups : public PermPtrContainer<PermGroup> {

    using Base = PermPtrContainer<PermGroup>;

public:

    PermGroups()
        : Base() {
    }
    PermGroups(const Base& base)
        : Base(base) {
    }
    PermGroups(Base&& base)
        : Base(base) {
    }
    PermGroups(const PermGroups& other) = default;
    PermGroups(PermGroups&& other) = default;

    PermGroups sortByPriority(bool greater = false) const {
        PermGroups result;
        for (auto& group : *this) {
            result.push_back(group);
        }
        std::sort(result.begin(), result.end(), 
            [greater](const std::shared_ptr<PermGroup>& a, const std::shared_ptr<PermGroup>& b) {
                return greater ? a->priority > b->priority : a->priority < b->priority;
        });
        return result;
    }

    std::shared_ptr<PermGroup>& operator[](const std::string& name) {
        PermGroup* ptr = nullptr;
        if (name == "everyone")
            ptr = new EveryonePermGroup;
        else if (name == "admin")
            ptr = new AdminPermGroup;
        else
            ptr = new GeneralPermGroup;
        auto def = std::shared_ptr<PermGroup>(ptr);
        def->name = name;
        return this->getOrCreate(name, def);
    }

    PermGroups& operator=(const PermGroups& other) = default;
    PermGroups& operator=(PermGroups&& other) = default;
    PermGroups& operator=(const Base& other) {
        return (PermGroups&)((Base&)*this) = other;
    }

};