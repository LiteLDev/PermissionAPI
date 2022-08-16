#include "pch.h"
#include <RegCommandAPI.h>
#include <EventAPI.h>
#include <PlayerInfoAPI.h>
#include "Mod.h"

using namespace RegisterCommandHelper;
using fmt::arg;

/*
/perm create group   <Name> [DisplayName] [Priority]
/perm delete group   <Name>
/perm create ability <Name> [Desc]
/perm delete ability <Name>
/perm update group   <Name> add member   <RealName>
/perm update group   <Name> rm  member   <RealName>
/perm update group   <Name> add ability  <AbName> [Enabled] [ExtraJson(as RawText)]
/perm update group   <Name> set ability  <AbName> <Enabled> [ExtraJson(as RawText)]
/perm update group   <Name> rm  ability  <AbName>
/perm update group   <Name> set priority <Priority>
/perm update group   <Name> set display_name  <DisplayName>
/perm update player  <Name> add group    <GroupName>
/perm update player  <Name> rm  group    <GroupName>
/perm view   player  <Name>
/perm view   group   <Name>
/perm view   ability <Name>
/perm list   group
/perm list   ability
*/

bool DisplayNameIsSameAs(const std::string& name, std::string disp) {
    size_t pos = 0;
    while ((pos = disp.find("§")) != std::string::npos) {
       disp.erase(pos, 3);
    }
    return disp == name;
}

class PermCommand : public Command {

    enum class SubCommands : char {
        None = -1,
        Create,
        Delete,
        Update,
        View,
        List,
    } subcmd = SubCommands::None;
    enum class Action : char {
        None = -1,
        Add,
        Set,
        Remove
    } action = Action::None;
    enum class TargetType : char {
        None = -1,
        Group,
        Ability,
        Player,
        Priority,
        Member,
        DisplayName
    };
    TargetType targetType1 = TargetType::None;
    TargetType targetType2 = TargetType::None;

    std::string name1;
    std::string name2;
    std::string desc;
    std::string displayName;
    CommandRawText extra;
    bool enabled = false;
    int priority = 0;

    bool enabled_set = false;
    bool extra_set = false;
    bool priority_set = false;
    bool displayName_set = false;
    bool desc_set = false;

    bool checkPermission(const xuid_t& xuid) const {
        auto abilities = mod.perm.getPlayerAbilities(xuid);
        if (abilities.contains("PermissionAPI:cmd_control")) {
            auto& ab = abilities.at("PermissionAPI:cmd_control");
            if (!ab.extra.is_object() || (ab.extra.is_object() && ab.extra.empty())) {
                // Invalid extra data/No extra data
                return true;
            }
            switch (this->subcmd) {
                case SubCommands::None: {
                    if (!ab.extra.contains("view_self")) return true;
                    if (ab.extra.at("view_self").is_boolean()) return ab.extra.at("view_self"); 
                    break;
                }
                case SubCommands::Create: {
                    if (!ab.extra.contains("create")) return true; // No more restrictions, pass
                    if (ab.extra.at("create").is_boolean()) return ab.extra.at("create"); 
                    auto& obj = ab.extra["create"];
                    switch (this->targetType1) {
                        case TargetType::Group:
                            if (!obj.contains("group")) return true; // Default pass
                            if (obj["group"].is_boolean() && obj["group"] == true) return true; // Boolean value is true, pass
                            if (obj["group"].is_object() && obj["group"].contains("enabled") && obj["groups"]["enabled"] == true) return true; // Object value
                            break;
                        case TargetType::Ability:
                            if (!obj.contains("ability")) return true; // Default pass
                            if (obj["ability"].is_boolean() && obj["ability"] == true) return true; // Boolean value is true, pass
                            if (obj["ability"].is_object() && obj["ability"].contains("enabled") && obj["abilities"]["enabled"] == true) return true; // Object value
                            break;
                    }
                    break;
                }
                case SubCommands::Delete: {
                    if (!ab.extra.contains("delete")) return true; // No more restrictions, pass
                    if (ab.extra["delete"].is_boolean()) return ab.extra["delete"].get<bool>();
                    auto& obj = ab.extra["delete"];
                    switch (this->targetType1) {
                        case TargetType::Group:
                            if (!obj.contains("group")) return true; // Default pass
                            if (obj["group"].is_boolean() && obj["group"] == true) return true; // Boolean value is true, pass
                            if (obj["group"].is_object() && obj["group"].contains("enabled") && obj["groups"]["enabled"] == true) return true; // Object value
                            break;
                        case TargetType::Ability:
                            if (!obj.contains("ability")) return true; // Default pass
                            if (obj["ability"].is_boolean() && obj["ability"] == true) return true; // Boolean value is true, pass
                            if (obj["ability"].is_object() && obj["ability"].contains("enabled") && obj["abilities"]["enabled"] == true) return true; // Object value
                            break;
                    }
                    break;
                }
                case SubCommands::List: {
                    if (!ab.extra.contains("list")) return true; // No more restrictions, pass
                    if (ab.extra["list"].is_boolean()) return ab.extra["list"].get<bool>();
                    auto& obj = ab.extra["list"];
                    switch (this->targetType1) {
                        case TargetType::Group:
                            if (!obj.contains("group")) return true; // Default pass
                            if (obj["group"].is_boolean() && obj["group"] == true) return true; // Boolean value is true, pass
                            if (obj["group"].is_object() && obj["group"].contains("enabled") && obj["groups"]["enabled"] == true) return true; // Object value
                            break;
                        case TargetType::Ability:
                            if (!obj.contains("ability")) return true; // Default pass
                            if (obj["ability"].is_boolean() && obj["ability"] == true) return true; // Boolean value is true, pass
                            if (obj["ability"].is_object() && obj["ability"].contains("enabled") && obj["abilities"]["enabled"] == true) return true; // Object value
                            break;
                    }
                    break;
                }
                case SubCommands::View: {
                    if (!ab.extra.contains("view")) return true; // No more restrictions, pass
                    if (ab.extra["view"].is_boolean()) return ab.extra["view"].get<bool>();
                    auto& obj = ab.extra["view"];
                    switch (this->targetType1) {
                        case TargetType::Group:
                            if (!obj.contains("group")) return true; // Default pass
                            if (obj["group"].is_boolean() && obj["group"] == true) return true; // Boolean value is true, pass
                            if (obj["group"].is_object() && obj["group"].contains("enabled") && obj["groups"]["enabled"] == true) {
                                if (obj["group"].contains("only")) {
                                    auto v = obj["group"]["only"];
                                    if (v.is_array()) {
                                        for (auto& s : v) {
                                            if (s.is_string() && s == this->name1) return true;
                                        }
                                    }
                                } else if (obj["group"].contains("except")) {
                                    auto v = obj["group"]["except"];
                                    if (v.is_array()) {
                                        for (auto& s : v) {
                                            if (s.is_string() && s == this->name1) return false;
                                        }
                                    }
                                    return true;
                                }
                                return true;
                            }
                            break;
                        case TargetType::Ability:
                            if (!obj.contains("ability")) return true; // Default pass
                            if (obj["ability"].is_boolean() && obj["ability"] == true) return true; // Boolean value is true, pass
                            if (obj["ability"].is_object() && obj["ability"].contains("enabled") && obj["abilities"]["enabled"] == true) {
                                if (obj["ability"].contains("only")) {
                                    auto v = obj["ability"]["only"];
                                    if (v.is_array()) {
                                        for (auto& s : v) {
                                            if (s.is_string() && s == this->name1) return true;
                                        }
                                    }
                                } else if (obj["ability"].contains("except")) {
                                    auto v = obj["ability"]["except"];
                                    if (v.is_array()) {
                                        for (auto& s : v) {
                                            if (s.is_string() && s == this->name1) return false;
                                        }
                                    }
                                    return true;
                                }
                                return true;
                            }
                            break;
                        case TargetType::Player:
                            if (this->name1.empty()) return true; // No name, default self, pass
                            if (!obj.contains("player")) return true; // Default pass
                            if (obj["player"].is_boolean() && obj["player"] == true) return true; // Boolean value is true, pass
                            if (obj["player"].is_array()) {
                                for (auto& xid : obj["player"]) {
                                    // If in array, pass
                                    if (xid.is_string() && xid == PlayerInfo::getXuid(this->name1)) return true;
                                }
                                return false;
                            }
                            if (obj["player"].is_string()) {
                                // If string, pass if equal
                                if (obj["player"] == PlayerInfo::getXuid(this->name1)) return true;
                                return false;
                            }
                            break;
                    }
                    break;
                }
                case SubCommands::Update: {
                    if (!ab.extra.contains("update")) return true; // No more restrictions, pass
                    if (ab.extra["update"].is_boolean()) return ab.extra["update"].get<bool>();
                    auto& obj = ab.extra["update"];
                    switch (this->targetType1) {
                        case TargetType::Group:
                            if (!obj.contains("group")) return true; // Default pass
                            if (obj["group"].is_boolean() && obj["group"] == true) return true; // Boolean value is true, pass
                            if (obj["group"].is_object()) {
                                if (obj["group"].contains("only")) {
                                    /*
                                    "only": [
                                        {
                                            "name": "group-a",
                                            "member": true, // = "member": ["add", "rm"]
                                            "priority": true, // = "priority": ["set"]
                                            "ability": true, // = "abilities": ["add", "rm", "set"]
                                            "display_name": true, // = "display_name": ["set"]
                                        },
                                        "group-b", // default allow all operations
                                    ]
                                    */
                                    auto v = obj["group"]["only"];
                                    if (v.is_array()) {
                                        for (auto& p : v) {
                                            if (p.is_string() && p == this->name1) return true;
                                            if (p.is_object() && p.contains("name") && p["name"].is_string() && p["name"] == this->name1) {
                                                switch (this->targetType2) {
                                                    case TargetType::Member:
                                                        if (p.contains("member") && p["member"].is_boolean() && p["member"] == true) return true;
                                                        if (p["member"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Add:
                                                                    if (std::find(p["member"].begin(), p["member"].end(), "add") != p["member"].end()) return true;
                                                                    break;
                                                                case Action::Remove:
                                                                    if (std::find(p["member"].begin(), p["member"].end(), "rm") != p["member"].end()) return true;
                                                                    break;                                                                
                                                            }
                                                        }
                                                        break;
                                                    case TargetType::Priority:
                                                        if (p.contains("priority") && p["priority"].is_boolean() && p["priority"] == true) return true;
                                                        if (p["priority"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Set:
                                                                    if (std::find(p["priority"].begin(), p["priority"].end(), "set") != p["priority"].end()) return true;
                                                                    break;
                                                            }
                                                        }
                                                        break;
                                                    case TargetType::Ability:
                                                        if (p.contains("ability") && p["ability"].is_boolean() && p["ability"] == true) return true;
                                                        if (p["ability"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Add:
                                                                    if (std::find(p["ability"].begin(), p["ability"].end(), "add") != p["ability"].end()) return true;
                                                                    break;
                                                                case Action::Remove:
                                                                    if (std::find(p["ability"].begin(), p["ability"].end(), "rm") != p["ability"].end()) return true;
                                                                    break;
                                                                case Action::Set:
                                                                    if (std::find(p["ability"].begin(), p["ability"].end(), "set") != p["ability"].end()) return true;
                                                                    break;
                                                            }
                                                        }
                                                        break;
                                                    case TargetType::DisplayName:
                                                        if (p.contains("display_name") && p["display_name"].is_boolean() && p["display_name"] == true) return true;
                                                        if (p["display_name"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Set:
                                                                    if (std::find(p["display_name"].begin(), p["display_name"].end(), "set") != p["display_name"].end()) return true;
                                                                    break;
                                                            }
                                                        }
                                                        break;
                                                }
                                            }
                                        }
                                    }
                                } else if (obj["group"].contains("except")) {
                                    auto v = obj["group"]["except"];
                                    if (v.is_array()) {
                                        for (auto& p : v) {
                                            if (p.is_string() && p == this->name1) return false;
                                            if (p.is_object() && p.contains("name") && p["name"].is_string() && p["name"] == this->name1) {
                                                switch (this->targetType2) {
                                                    case TargetType::Member:
                                                        if (p.contains("member") && p["member"].is_boolean() && p["member"] == true) return false;
                                                        if (p["member"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Add:
                                                                    if (std::find(p["member"].begin(), p["member"].end(), "add") != p["member"].end()) return false;
                                                                    break;
                                                                case Action::Remove:
                                                                    if (std::find(p["member"].begin(), p["member"].end(), "rm") != p["member"].end()) return false;
                                                                    break;                                                                
                                                            }
                                                        }
                                                        break;
                                                    case TargetType::Priority:
                                                        if (p.contains("priority") && p["priority"].is_boolean() && p["priority"] == true) return false;
                                                        if (p["priority"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Set:
                                                                    if (std::find(p["priority"].begin(), p["priority"].end(), "set") != p["priority"].end()) return false;
                                                                    break;
                                                            }
                                                        }
                                                        break;
                                                    case TargetType::Ability:
                                                        if (p.contains("ability") && p["ability"].is_boolean() && p["ability"] == true) return false;
                                                        if (p["ability"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Add:
                                                                    if (std::find(p["ability"].begin(), p["ability"].end(), "add") != p["ability"].end()) return false;
                                                                    break;
                                                                case Action::Remove:
                                                                    if (std::find(p["ability"].begin(), p["ability"].end(), "rm") != p["ability"].end()) return false;
                                                                    break;
                                                                case Action::Set:
                                                                    if (std::find(p["ability"].begin(), p["ability"].end(), "set") != p["ability"].end()) return false;
                                                                    break;
                                                            }
                                                        }
                                                        break;
                                                }
                                            }
                                        }
                                        return true;
                                    }
                                }
                            }
                            break;
                        case TargetType::Player:
                            if (!obj.contains("player")) return true;
                            if (obj["player"].is_array()) {
                                for (auto& xid : obj["player"]) {
                                    if (xid.is_string() && xid == PlayerInfo::getXuid(this->name1)) return true;
                                }
                            }
                            break;
                    }
                    break;
                }
            }
        }
        return false;
    }

    bool handleNone(CommandOrigin const& ori, CommandOutput& outp) const {
        // perm
        if ((OriginType)ori.getOriginType() == OriginType::Player) {
            auto pl = ori.getPlayer();
            outp.trAddMessage("permapi.cmd.output.view.player.yourStatus");
            outp.trAddMessage("permapi.cmd.output.view.player.groupsTitle");
            for (auto& group : mod.perm.getPlayerGroups(pl->getXuid())) {
                outp.trAddMessage("  * " + group->displayName);
            }
            outp.trAddMessage("permapi.cmd.output.view.player.abilitiesTitle");
            for (auto& ability : mod.perm.getPlayerAbilities(pl->getXuid())) {
                outp.addMessage("  * " + ability.name + ": " + 
                    (mod.perm.abilitiesInfo.contains(ability.name) ?
                        mod.perm.abilitiesInfo[ability.name].desc : pl->tr("permapi.cmd.output.noDesc")));
            }
            return true;
        } else {
            outp.trError("permapi.cmd.error.invalidCommand");
        }
        return false;
    }

    bool handleCreate(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm create group
            case TargetType::Group: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (mod.perm.groups.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.groupAlreadyExists");
                    break;
                }
                auto group = mod.perm.createGroup(this->name1, this->name1);
                if (this->priority_set) group->priority = this->priority;
                if (this->displayName_set) group->displayName = this->displayName;
                mod.perm.save();
                outp.trSuccess("permapi.cmd.output.create.group.success");
                return true;
            }
            // perm create ability
            case TargetType::Ability:
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (mod.perm.abilitiesInfo.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.abilityAlreadyExists");
                    break;
                }
                if (!PermAbility::isValidAbilityName(this->name1)) {
                    outp.trError("permapi.cmd.error.invalidAbilityName");
                    outp.trError("permapi.cmd.output.abilityNameExamples");
                    break;
                }
                mod.perm.registerAbility(this->name1, this->desc);
                outp.trSuccess("permapi.cmd.output.create.ability.success");
                return true;
            default:
                outp.trError("permapi.cmd.error.invalidCommand");
                break;
        }
        return false;
    }

    bool handleDelete(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm delete group
            case TargetType::Group: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.groups.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.groupNotFound");
                    break;
                }
                mod.perm.groups.remove(this->name1);
                mod.perm.save();
                outp.trSuccess("permapi.cmd.output.delete.group.success");
                return true;
            }
            // perm delete ability
            case TargetType::Ability:
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.abilitiesInfo.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.abilityNotFound");
                    break;
                }
                mod.perm.deleteAbility(this->name1);
                outp.trSuccess("permapi.cmd.output.delete.ability.success");
                return true;
            default:
                outp.trError("permapi.cmd.error.invalidCommand");
                break;
        }
        return false;
    }

    bool handleList(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm list group
            case TargetType::Group: {
                if (mod.perm.groups.empty()) {
                    outp.trSuccess("permapi.cmd.output.list.group.none");
                    return true;
                }
                outp.trAddMessage("permapi.cmd.output.list.group.header");
                for (auto& group : mod.perm.groups) {
                    std::string msg = "* " + group->displayName;
                    if (!DisplayNameIsSameAs(group->name, group->displayName)) {
                        // If the display name is different from name, append the name
                        msg += " <" + group->name + ">";
                    }
                    if ((OriginType)ori.getOriginType() == OriginType::Player) {
                        auto pl = ori.getPlayer();
                        if (!pl) {
                            outp.trError("permapi.cmd.error.internal");
                            break;
                        }
                        if (mod.perm.isMemberOf(pl->getXuid(), group->name)) {
                            outp.trAddMessage(msg + " " + pl->tr("permapi.cmd.output.list.group.isMember"));
                            continue;
                        }
                    }
                    outp.trAddMessage(msg);
                }
                return true;
            }
            // perm list ability
            case TargetType::Ability:
                if (mod.perm.abilitiesInfo.empty()) {
                    outp.trSuccess("permapi.cmd.output.list.ability.none");
                    return true;
                }
                outp.trAddMessage("permapi.cmd.output.list.ability.header");
                for (auto& ability : mod.perm.abilitiesInfo) {
                    if (ability.desc.empty()) {
                        if ((OriginType)ori.getOriginType() == OriginType::Player) {
                            auto pl = ori.getPlayer();
                            if (!pl) {
                                outp.trError("permapi.cmd.error.internal");
                                break;
                            }  
                            outp.trAddMessage("* " + ability.name + ": " + pl->tr("permapi.cmd.output.noDesc"));
                            continue;
                        }
                        outp.trAddMessage("* " + ability.name + ": " + tr("permapi.cmd.output.noDesc"));
                        continue;
                    }
                    outp.trAddMessage("* " + ability.name + ": " + ability.desc);
                }
                return true;
            default:
                outp.trError("permapi.cmd.error.invalidCommand");
                break;
        }
        return false;
    }
    
    bool handleView(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm view group
            case TargetType::Group: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.groups.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.groupNotFound");
                    break;
                }
                auto& group = mod.perm.groups[this->name1];
                outp.trAddMessage("permapi.cmd.output.view.group.header");
                outp.trAddMessage("permapi.cmd.output.view.group.info.name",
                                  arg("groupName", group->name));
                outp.trAddMessage("permapi.cmd.output.view.group.info.displayName", group->displayName);
                outp.trAddMessage("permapi.cmd.output.view.group.info.priority", std::to_string(group->priority));
                if (group->getType() != PermGroup::Type::Everyone) {
                    if (group->members.empty()) {
                        outp.trAddMessage("permapi.cmd.output.view.group.info.membersNone");
                    } else {
                        outp.trAddMessage("permapi.cmd.output.view.group.info.members");
                        for (auto& xid : group->members) {
                            outp.trAddMessage("  * " + PlayerInfo::fromXuid(xid) + " (" + xid + ")");
                        }
                    }
                }
                if (group->abilities.empty()) {
                    outp.trAddMessage("permapi.cmd.output.view.group.info.abilitiesNone");
                } else {
                    outp.trAddMessage("permapi.cmd.output.view.group.info.abilities");
                    for (auto& ability : group->abilities) {
                        std::string suffix = (ability.enabled ? 
                            "permapi.cmd.output.view.group.enabled" :
                            "permapi.cmd.output.view.group.disabled");
                        if (ability.enabled && ability.extra.is_object() && !ability.extra.empty()) {
                            suffix = "permapi.cmd.output.view.group.enabledWithExtra";
                        }
                        if ((OriginType)ori.getOriginType() == OriginType::Player) {
                            auto pl = ori.getPlayer();
                            if (!pl) {
                                outp.trError("permapi.cmd.error.internal");
                                break;
                            }
                            suffix = pl->tr(suffix);
                        } else {
                            suffix = tr(suffix);
                        }
                        outp.trAddMessage("  * " + ability.name + " " + suffix);
                    }
                }
                // Special cases
                if (group->getType() == PermGroup::Type::Admin) {
                    outp.trAddMessage("permapi.cmd.output.view.group.note.admin", group->displayName);
                } else if (group->getType() == PermGroup::Type::Everyone) {
                    outp.trAddMessage("permapi.cmd.output.view.group.note.everyone", group->displayName);
                }
                return true;
            }
            // perm view ability
            case TargetType::Ability: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.abilitiesInfo.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.abilityNotFound");
                    break;
                }
                auto& ability = mod.perm.abilitiesInfo[this->name1];
                outp.trAddMessage("permapi.cmd.output.view.ability.header");
                outp.trAddMessage("permapi.cmd.output.view.ability.info.name",
                                  arg("abilityName", ability.name));
                outp.trAddMessage("permapi.cmd.output.view.ability.info.desc", ability.desc);
                return true;
            }
            // perm view player
            case TargetType::Player: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                auto target = PlayerInfo::getXuid(this->name1);
                if (target.empty()) {
                    outp.trError("permapi.cmd.error.playerNotFound");
                    break;
                }
                outp.trAddMessage("permapi.cmd.output.view.player.status", this->name1);
                outp.trAddMessage("permapi.cmd.output.view.player.groupsTitle");
                for (auto& group : mod.perm.getPlayerGroups(target)) {
                    outp.trAddMessage("  * " + group->displayName);
                }
                outp.trAddMessage("permapi.cmd.output.view.player.abilitiesTitle");
                for (auto& ability : mod.perm.getPlayerAbilities(target)) {
                    if ((OriginType)ori.getOriginType() == OriginType::Player) {
                        auto pl = ori.getPlayer();
                        if (!pl) {
                            outp.trError("permapi.cmd.error.internal");
                            break;
                        }
                        outp.addMessage("  * " + ability.name + ": " + 
                            (mod.perm.abilitiesInfo.contains(ability.name) ?
                                mod.perm.abilitiesInfo[ability.name].desc : pl->tr("permapi.cmd.output.noDesc")));
                        continue;
                    }
                    outp.addMessage("  * " + ability.name + ": " + 
                        (mod.perm.abilitiesInfo.contains(ability.name) ?
                            mod.perm.abilitiesInfo[ability.name].desc : tr("permapi.cmd.output.noDesc")));
                }
                return true;
            }
            default:
                outp.trError("permapi.cmd.error.invalidCommand");
                break;
        }
        return false;
    }

    bool handleUpdate(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm update group
            case TargetType::Group: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.groups.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.groupNotFound");
                    break;
                }
                auto& group = mod.perm.groups[this->name1];
                switch (this->targetType2) {
                    // perm update group ... member    
                    case TargetType::Member: {
                        if (group->getType() == PermGroup::Type::Everyone) {
                            outp.trError("permapi.cmd.error.modifyMembersOfEveryone");
                            return false;
                        }
                        switch (this->action) {
                            // perm update group add member
                            case Action::Add: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                auto xid = PlayerInfo::getXuid(this->name2);
                                if (xid.empty()) {
                                    outp.trError("permapi.cmd.error.playerNotFound");
                                    return false;     
                                }
                                group->addMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.group.member.add.success",
                                               arg("name", this->name2),
                                               arg("xuid", xid),
                                               arg("groupDisplayName", group->displayName));
                                return true;
                            }
                            // perm update group remove member
                            case Action::Remove: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                auto xid = PlayerInfo::getXuid(this->name2);
                                if (xid.empty()) {
                                    outp.trError("permapi.cmd.error.playerNotFound");
                                    return false;
                                }
                                group->removeMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.group.member.remove.success",
                                               arg("name", this->name2),
                                               arg("xuid", xid),
                                               arg("groupDisplayName", group->displayName));
                                return true;
                            }
                            default:
                                outp.trError("permapi.cmd.error.invalidCommand");
                                break;
                        }
                        break;
                    }
                    // perm update group ... ability
                    case TargetType::Ability: {
                        switch (this->action) {
                            // perm update group add ability
                            case Action::Add: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                if (!mod.perm.abilitiesInfo.contains(this->name2)) {
                                    outp.trError("permapi.cmd.error.abilityNotFound");
                                    break;
                                }
                                nlohmann::json extraJson;
                                if (this->extra_set) {
                                    try {
                                        extraJson = nlohmann::json::parse(this->extra.getText());
                                    } catch (const std::exception& e) {
                                        outp.trError("permapi.cmd.error.jsonParsingError", e.what());
                                        return false;
                                    }
                                }
                                group->setAbility(this->name2, enabled_set ? this->enabled : true, extraJson);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.group.ability.add.success",
                                               arg("name", this->name2),
                                               arg("groupDisplayName", group->displayName));
                                return true;
                            }
                            // perm update group add ability
                            case Action::Set: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                if (!mod.perm.abilitiesInfo.contains(this->name2)) {
                                    outp.trError("permapi.cmd.error.abilityNotFound");
                                    break;
                                }
                                if (!this->enabled_set) {
                                    outp.trError("permapi.cmd.error.missingArgument", "enabled");
                                    break;
                                }
                                nlohmann::json extraJson;
                                if (this->extra_set) {
                                    try {
                                        extraJson = nlohmann::json::parse(this->extra.getText());
                                    }
                                    catch (const std::exception& e) {
                                        outp.trError("permapi.cmd.error.jsonParsingError", e.what());
                                        return false;
                                    }
                                }
                                group->setAbility(this->name2, this->enabled, extraJson);
                                mod.perm.save();
                                std::string suffix = (this->enabled ? 
                                    "permapi.cmd.output.update.group.ability.set.enabled" :
                                    "permapi.cmd.output.update.group.ability.set.disabled");
                                if ((OriginType)ori.getOriginType() == OriginType::Player) {
                                    auto pl = ori.getPlayer();
                                    if (!pl) {
                                        outp.trError("permapi.cmd.error.internal");
                                        break;
                                    }
                                    suffix = pl->tr(suffix);
                                } else {
                                    suffix = tr(suffix);
                                }
                                outp.trSuccess("permapi.cmd.output.update.group.ability.set.success",
                                               arg("name", this->name2),
                                               arg("groupDisplayName", group->displayName),
                                               arg("enable", suffix));
                                return true;
                            }
                            // perm update group remove ability
                            case Action::Remove: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                if (!mod.perm.abilitiesInfo.contains(this->name2)) {
                                    outp.trError("permapi.cmd.error.abilityNotFound");
                                    break;
                                }
                                group->removeAbility(this->name2);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.group.ability.remove.success",
                                               arg("name", this->name2),
                                               arg("groupDisplayName", group->displayName));
                                return true;
                            }
                            default:
                                outp.trError("permapi.cmd.error.invalidCommand");
                                break;
                        }
                        break;
                    }
                    // perm update group set priority
                    case TargetType::Priority: {
                        if (this->action != Action::Set) {
                            outp.trError("permapi.cmd.error.invalidCommand");
                            break;
                        }
                        if (!this->priority_set) {
                            outp.trError("permapi.cmd.error.missingArgument", "priority");
                            break;
                        }
                        group->priority = this->priority;
                        mod.perm.save();
                        outp.trSuccess("permapi.cmd.output.update.group.priority.set.success",
                                       arg("groupDisplayName", group->displayName),
                                       arg("priority", this->priority));
                        return true;
                    }
                    // perm update group set display_name
                    case TargetType::DisplayName: {
                        if (this->action != Action::Set) {
                            outp.trError("permapi.cmd.error.invalidCommand");
                            break;
                        }
                        if (!this->displayName_set) {
                            outp.trError("permapi.cmd.error.missingArgument", "display_name");
                            break;
                        }
                        group->displayName = this->displayName;
                        mod.perm.save();
                        outp.trSuccess("permapi.cmd.output.update.group.displayName.set.success",
                                       arg("groupName", group->name),
                                       arg("displayName", this->displayName));
                        return true;
                    }
                    default:
                        outp.trError("permapi.cmd.error.invalidCommand");
                        break;
                }
                break;
            }
            // perm update player
            case TargetType::Player: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                auto xid = PlayerInfo::getXuid(this->name1);
                if (xid.empty()) {
                    outp.trError("permapi.cmd.error.playerNotFound");
                    return false;
                }
                switch (this->targetType2) {
                    case TargetType::Group: {
                        if (this->name2.empty()) {
                            outp.trError("permapi.cmd.error.invalidCommand");
                            break;
                        }
                        if (!mod.perm.groups.contains(this->name2)) {
                            outp.trError("permapi.cmd.error.groupNotFound");
                            break;
                        }
                        auto& group = mod.perm.groups[this->name2];
                        switch (this->action) {
                            // perm update player add group
                            case Action::Add: {
                                group->addMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.player.group.add.success",
                                               arg("name", this->name1),
                                               arg("xuid", xid),
                                               arg("groupDisplayName", group->displayName));
                                return true;
                            }
                            // perm update player remove group
                            case Action::Remove: {
                                group->removeMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.player.group.remove.success",
                                               arg("name", this->name1),
                                               arg("xuid", xid),
                                               arg("groupDisplayName", group->displayName));
                                return true;
                            }
                            default:
                                outp.trError("permapi.cmd.error.invalidCommand");
                                break;
                        }
                        break;
                    }
                    default:
                        outp.trError("permapi.cmd.error.invalidCommand");
                        break;
                }
                break;
            }
        }
        return false;
    }

public:

    void execute(CommandOrigin const& ori, CommandOutput& outp) const {
        try {
            outp.setLanguageCode(ori);
            auto oriType = (OriginType)ori.getOriginType();
            if (oriType == OriginType::Player) {
                auto pl = ori.getPlayer();
                if (!pl) {
                    outp.trError("permapi.cmd.error.internal");
                    return;
                }
                if (!checkPermission(pl->getXuid())) {
                    outp.trError("permapi.cmd.error.permissionDenied");
                    return;
                }
            }
            bool success = false;
            switch (this->subcmd) {
                case SubCommands::None:
                    success = this->handleNone(ori, outp);
                    break;
                case SubCommands::Create:
                    success = this->handleCreate(ori, outp);
                    break;
                case SubCommands::Delete:
                    success = this->handleDelete(ori, outp);
                    break;
                case SubCommands::List:
                    success = this->handleList(ori, outp);
                    break;
                case SubCommands::View:
                    success = this->handleView(ori, outp);
                    break;
                case SubCommands::Update:
                    success = this->handleUpdate(ori, outp);
                    break;
            }
            if (success) {
                outp.success();
            }
        } catch (const std::exception& e) {
            auto oriType = (OriginType)ori.getOriginType();
            if (oriType == OriginType::Player) {
                outp.trError("permapi.cmd.error.internal");
            } else {
                outp.trError("permapi.cmd.error.internal");
                outp.trError(e.what());
            }
        }
    }

    static void setup(CommandRegistry* reg) {
        if (reg == nullptr) {
            return;
        }
        reg->registerCommand("perm", "Permission group system",
                             CommandPermissionLevel::Any,
                             {CommandFlagValue::None},
                             {(CommandFlagValue)0x80});

        reg->addEnum<SubCommands>("Create", {{"create", SubCommands::Create}});
        reg->addEnum<SubCommands>("Delete", {{"delete", SubCommands::Delete}});
        reg->addEnum<SubCommands>("Update", {{"update", SubCommands::Update}});
        reg->addEnum<SubCommands>("View", {{"view", SubCommands::View}});
        reg->addEnum<SubCommands>("List", {{"list", SubCommands::List}});

        auto subcmd_create = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "Create", "Create");
        auto subcmd_delete = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "Delete", "Delete");
        auto subcmd_update = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "Update", "Update");
        auto subcmd_view = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "View", "View");
        auto subcmd_list = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::subcmd, "List", "List");

        reg->addEnum<Action>("Add", {{"add", Action::Add}});
        reg->addEnum<Action>("Set", {{"set", Action::Set}});
        reg->addEnum<Action>("Remove", {{"rm", Action::Remove}});

        auto action_add = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::action, "Add", "Add");
        auto action_set = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::action, "Set", "Set");
        auto action_remove = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::action, "Remove", "Remove");
        
        reg->addEnum<TargetType>("Group", {{"group", TargetType::Group}});
        reg->addEnum<TargetType>("Ability", {{"ability", TargetType::Ability}});
        reg->addEnum<TargetType>("Player", {{"player", TargetType::Player}});
        reg->addEnum<TargetType>("Priority", {{"priority", TargetType::Priority}});
        reg->addEnum<TargetType>("Member", {{"member", TargetType::Member}});
        reg->addEnum<TargetType>("DisplayName", {{"display_name", TargetType::DisplayName}});

        auto target_group1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Group1", "Group");
        auto target_ability1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Ability1", "Ability");
        auto target_player1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Player1", "Player");
        auto target_priority1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Priority1", "Priority");
        auto target_member1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Member1", "Member");
        
        auto target_group2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "Group2", "Group");
        auto target_ability2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "Ability2", "Ability");
        auto target_player2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "Player2", "Player");
        auto target_priority2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "Priority2", "Priority");
        auto target_member2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "Member2", "Member");
        auto target_display_name2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "DisplayName2", "DisplayName");

        auto param_name1 = makeMandatory(&PermCommand::name1, "name1");
        auto param_name2 = makeMandatory(&PermCommand::name2, "name2");
        auto param_enabled = makeOptional(&PermCommand::enabled, "enabled", &PermCommand::enabled_set);
        auto param_priority = makeOptional(&PermCommand::priority, "priority", &PermCommand::priority_set);
        auto param_displayName = makeOptional(&PermCommand::displayName, "display name", &PermCommand::displayName_set);
        auto param_desc = makeOptional(&PermCommand::desc, "description", &PermCommand::desc_set);
        auto param_extra= makeOptional(&PermCommand::extra, "extraJson", &PermCommand::extra_set);

        reg->registerOverload<PermCommand>("perm");
        // perm create group <Name> [DisplayName] [Priority]
        reg->registerOverload<PermCommand>("perm", subcmd_create, target_group1, param_name1, param_displayName, param_priority);
        // perm delete ability <Name> [Desc]
        reg->registerOverload<PermCommand>("perm", subcmd_create, target_ability1, param_name1, param_desc);
        // perm delete group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_delete, target_group1, param_name1);
        // perm delete ability <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_delete, target_ability1, param_name1);
        // perm update group <Name> add member <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group1, param_name1, action_add, target_member2, param_name2);
        // perm update group <Name> rm member <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group1, param_name1, action_remove, target_member2, param_name2);
        // perm update group <Name> add ability <Name> [Enabled] [ExtraJson]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group1, param_name1, action_add, target_ability2, param_name2, param_enabled, param_extra);
        // perm update group <Name> set ability <Name> [Enabled] [ExtraJson]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group1, param_name1, action_set, target_ability2, param_name2, param_enabled, param_extra);
        // perm update group <Name> rm ability <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group1, param_name1, action_remove, target_ability2, param_name2);
        // perm update group <Name> set priority [Priority]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group1, param_name1, action_set, target_priority2, param_priority);
        // perm update group <Name> set display_name [DisplayName]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_group1, param_name1, action_set, target_display_name2, param_displayName);
        // perm update player <Name> add group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_player1, param_name1, action_add, target_group2, param_name2);
        // perm update player <Name> rm group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_player1, param_name1, action_remove, target_group2, param_name2);
        // perm view group <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_group1, param_name1);
        // perm view ability <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_ability1, param_name1);
        // perm view player <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_player1, param_name1);
        // perm list group
        reg->registerOverload<PermCommand>("perm", subcmd_list, target_group1);
        // perm list ability
        reg->registerOverload<PermCommand>("perm", subcmd_list, target_ability1);
    }
};

void SetupAllCmds(CommandRegistry* reg) {
    PermCommand::setup(reg);
}