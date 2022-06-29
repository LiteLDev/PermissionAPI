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
            this->abilities.emplace_back(name, enabled, extra);
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

class PermGroupWrapper : public std::shared_ptr<PermGroup> {

    using Base = std::shared_ptr<PermGroup>;

public:

    PermMembers& members;
    PermAbilities& abilities;
    std::string& name;
    std::string& displayName;
    int& priority;

    PermGroupWrapper()
        : Base(new GeneralPermGroup())
        , members(this->get()->getMembers())
        , abilities(this->get()->abilities)
        , name(this->get()->name)
        , displayName(this->get()->displayName)
        , priority(this->get()->priority) {
    }
    PermGroupWrapper(PermGroup* group)
        : Base(group)
        , members(group->getMembers())
        , abilities(group->abilities)
        , name(group->name)
        , displayName(group->displayName)
        , priority(group->priority) {
    }
    PermGroupWrapper(const GeneralPermGroup& group)
        : Base(new GeneralPermGroup(group))
        , members(this->get()->getMembers())
        , abilities(this->get()->abilities)
        , name(this->get()->name)
        , displayName(this->get()->displayName)
        , priority(this->get()->priority) {
    }
    PermGroupWrapper(const EveryonePermGroup& group)
        : Base(new EveryonePermGroup(group))
        , members(this->get()->getMembers())
        , abilities(this->get()->abilities)
        , name(this->get()->name)
        , displayName(this->get()->displayName)
        , priority(this->get()->priority) {
    }
    PermGroupWrapper(const AdminPermGroup& group)
        : Base(new AdminPermGroup(group))
        , members(this->get()->getMembers())
        , abilities(this->get()->abilities)
        , name(this->get()->name)
        , displayName(this->get()->displayName)
        , priority(this->get()->priority) {
    }

    virtual bool hasAbility(const std::string& name) const {
        return this->get()->hasAbility(name);
    }
    virtual void setAbility(const std::string& name, bool enabled = true, const nlohmann::json& extra = nlohmann::json()) {
        this->get()->setAbility(name, enabled, extra);
    }
    virtual void removeAbility(const std::string& name) {
        this->get()->removeAbility(name);
    }

    virtual bool hasMember(const xuid_t& xuid) const {
        return this->get()->hasMember(xuid);
    }
    virtual void addMember(const xuid_t& xuid) {
        this->get()->addMember(xuid);
    }
    virtual void removeMember(const xuid_t& xuid) {
        this->get()->removeMember(xuid);
    }

    virtual PermMembers& getMembers() {
        return this->get()->getMembers();
    }
    virtual PermGroup::Type getType() const {
        return this->get()->getType();
    }

    virtual bool validate() {
        return this->get()->validate();
    }

};

class PermGroups : public PermContainer<PermGroupWrapper> {

    using Base = PermContainer<PermGroupWrapper>;

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