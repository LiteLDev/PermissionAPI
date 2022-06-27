#include "Foundation.hpp"
#include "PermMember.hpp"
#include "PermAbility.hpp"

class PermGroup {

protected:

    PermMembers members; ///< The members of the group. Do not access it directly.

public:

    PermAbilities abilities;
    std::string name;
    int priority = 0;

    virtual ~PermGroup() = default;

    virtual bool hasAbility(const std::string& name) const {
        return this->abilities.contains(name) && this->abilities.at(name).enabled;
    }
    virtual void setAbility(const std::string& name, bool enabled, const nlohmann::json& extra = {}) {
        if (!this->abilities.contains(name)) {
            this->abilities.emplace_back(name, enabled, extra);
        } else {
            this->abilities.at(name).enabled = enabled;
        }
    }

    virtual bool hasMember(const xuid_t& xuid) const = 0;    
    virtual void addMember(const xuid_t& xuid) {
        this->members.push_back(xuid);
    }
    virtual void removeMember(const xuid_t& xuid) {
        this->members.erase(std::remove(this->members.begin(), this->members.end(), xuid), this->members.end());
    }


    virtual PermMembers& getMembers() {
        return this->members;
    }

};

class PermGroups : public PermContainer<PermGroup> {

    using Base = PermContainer<PermGroup>;

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
        std::sort(result.begin(), result.end(), [greater](const PermGroup& a, const PermGroup& b) {
            return greater ? a.priority > b.priority : a.priority < b.priority;
        });
        return result;
    }

    PermGroups& operator=(const Base& other) {
        return (PermGroups&)((Base&)*this) = other;
    }

};