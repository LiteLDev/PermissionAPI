#include "pch.h"
#include <RegCommandAPI.h>
#include <EventAPI.h>
#include <PlayerInfoAPI.h>
#include "Mod.h"

using namespace RegisterCommandHelper;
using namespace PERM;
using fmt::arg;

/*
/perm create role   <Name> [DisplayName] [Priority]
/perm delete role   <Name>
/perm create perm   <Name> [Desc]
/perm delete perm   <Name>
/perm update role   <Name> add member   <RealName>
/perm update role   <Name> rm  member   <RealName>
/perm update role   <Name> add perm     <PermName> [Enabled] [ExtraJson(as RawText)]
/perm update role   <Name> set perm     <PermName> <Enabled> [ExtraJson(as RawText)]
/perm update role   <Name> rm  perm     <PermName>
/perm update role   <Name> set priority <Priority>
/perm update role   <Name> set display_name  <DisplayName>
/perm update player <Name> add role     <RoleName>
/perm update player <Name> rm  role     <RoleName>
/perm view   player <Name>
/perm view   role   <Name>
/perm view   perm   <Name>
/perm list   role
/perm list   perm
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
        Role,
        Permission,
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
        auto permissions = mod.perm.getPlayerPermissions(xuid);
        if (permissions.contains("PermissionAPI:cmd_control")) {
            auto& perm = permissions.at("PermissionAPI:cmd_control");
            if (!perm.extra.is_object() || (perm.extra.is_object() && perm.extra.empty())) {
                // Invalid extra data/No extra data
                return true;
            }
            switch (this->subcmd) {
                case SubCommands::None: {
                    if (!perm.extra.contains("view_self")) return true;
                    if (perm.extra.at("view_self").is_boolean()) return perm.extra.at("view_self"); 
                    break;
                }
                case SubCommands::Create: {
                    if (!perm.extra.contains("create")) return true; // No more restrictions, pass
                    if (perm.extra.at("create").is_boolean()) return perm.extra.at("create"); 
                    auto& obj = perm.extra["create"];
                    switch (this->targetType1) {
                        case TargetType::Role:
                            if (!obj.contains("role")) return true; // Default pass
                            if (obj["role"].is_boolean() && obj["role"] == true) return true; // Boolean value is true, pass
                            if (obj["role"].is_object() && obj["role"].contains("enabled") && obj["roles"]["enabled"] == true) return true; // Object value
                            break;
                        case TargetType::Permission:
                            if (!obj.contains("perm")) return true; // Default pass
                            if (obj["perm"].is_boolean() && obj["perm"] == true) return true; // Boolean value is true, pass
                            if (obj["perm"].is_object() && obj["perm"].contains("enabled") && obj["perm"]["enabled"] == true) return true; // Object value
                            break;
                    }
                    break;
                }
                case SubCommands::Delete: {
                    if (!perm.extra.contains("delete")) return true; // No more restrictions, pass
                    if (perm.extra["delete"].is_boolean()) return perm.extra["delete"].get<bool>();
                    auto& obj = perm.extra["delete"];
                    switch (this->targetType1) {
                        case TargetType::Role:
                            if (!obj.contains("role")) return true; // Default pass
                            if (obj["role"].is_boolean() && obj["role"] == true) return true; // Boolean value is true, pass
                            if (obj["role"].is_object() && obj["role"].contains("enabled") && obj["roles"]["enabled"] == true) return true; // Object value
                            break;
                        case TargetType::Permission:
                            if (!obj.contains("perm")) return true; // Default pass
                            if (obj["perm"].is_boolean() && obj["perm"] == true) return true; // Boolean value is true, pass
                            if (obj["perm"].is_object() && obj["perm"].contains("enabled") && obj["perm"]["enabled"] == true) return true; // Object value
                            break;
                    }
                    break;
                }
                case SubCommands::List: {
                    if (!perm.extra.contains("list")) return true; // No more restrictions, pass
                    if (perm.extra["list"].is_boolean()) return perm.extra["list"].get<bool>();
                    auto& obj = perm.extra["list"];
                    switch (this->targetType1) {
                        case TargetType::Role:
                            if (!obj.contains("role")) return true; // Default pass
                            if (obj["role"].is_boolean() && obj["role"] == true) return true; // Boolean value is true, pass
                            if (obj["role"].is_object() && obj["role"].contains("enabled") && obj["roles"]["enabled"] == true) return true; // Object value
                            break;
                        case TargetType::Permission:
                            if (!obj.contains("perm")) return true; // Default pass
                            if (obj["perm"].is_boolean() && obj["perm"] == true) return true; // Boolean value is true, pass
                            if (obj["perm"].is_object() && obj["perm"].contains("enabled") && obj["perm"]["enabled"] == true) return true; // Object value
                            break;
                    }
                    break;
                }
                case SubCommands::View: {
                    if (!perm.extra.contains("view")) return true; // No more restrictions, pass
                    if (perm.extra["view"].is_boolean()) return perm.extra["view"].get<bool>();
                    auto& obj = perm.extra["view"];
                    switch (this->targetType1) {
                        case TargetType::Role:
                            if (!obj.contains("role")) return true; // Default pass
                            if (obj["role"].is_boolean() && obj["role"] == true) return true; // Boolean value is true, pass
                            if (obj["role"].is_object() && obj["role"].contains("enabled") && obj["roles"]["enabled"] == true) {
                                if (obj["role"].contains("only")) {
                                    auto v = obj["role"]["only"];
                                    if (v.is_array()) {
                                        for (auto& s : v) {
                                            if (s.is_string() && s == this->name1) return true;
                                        }
                                    }
                                } else if (obj["role"].contains("except")) {
                                    auto v = obj["role"]["except"];
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
                        case TargetType::Permission:
                            if (!obj.contains("perm")) return true; // Default pass
                            if (obj["perm"].is_boolean() && obj["perm"] == true) return true; // Boolean value is true, pass
                            if (obj["perm"].is_object() && obj["perm"].contains("enabled") && obj["perm"]["enabled"] == true) {
                                if (obj["perm"].contains("only")) {
                                    auto v = obj["perm"]["only"];
                                    if (v.is_array()) {
                                        for (auto& s : v) {
                                            if (s.is_string() && s == this->name1) return true;
                                        }
                                    }
                                } else if (obj["perm"].contains("except")) {
                                    auto v = obj["perm"]["except"];
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
                    if (!perm.extra.contains("update")) return true; // No more restrictions, pass
                    if (perm.extra["update"].is_boolean()) return perm.extra["update"].get<bool>();
                    auto& obj = perm.extra["update"];
                    switch (this->targetType1) {
                        case TargetType::Role:
                            if (!obj.contains("role")) return true; // Default pass
                            if (obj["role"].is_boolean() && obj["role"] == true) return true; // Boolean value is true, pass
                            if (obj["role"].is_object()) {
                                if (obj["role"].contains("only")) {
                                    /*
                                    "only": [
                                        {
                                            "name": "role-a",
                                            "member": true, // = "member": ["add", "rm"]
                                            "priority": true, // = "priority": ["set"]
                                            "permission": true, // = "permissions": ["add", "rm", "set"]
                                            "display_name": true, // = "display_name": ["set"]
                                        },
                                        "role-b", // default allow all operations
                                    ]
                                    */
                                    auto v = obj["role"]["only"];
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
                                                    case TargetType::Permission:
                                                        if (p.contains("perm") && p["perm"].is_boolean() && p["perm"] == true) return true;
                                                        if (p["perm"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Add:
                                                                    if (std::find(p["perm"].begin(), p["perm"].end(), "add") != p["perm"].end()) return true;
                                                                    break;
                                                                case Action::Remove:
                                                                    if (std::find(p["perm"].begin(), p["perm"].end(), "rm") != p["perm"].end()) return true;
                                                                    break;
                                                                case Action::Set:
                                                                    if (std::find(p["perm"].begin(), p["perm"].end(), "set") != p["perm"].end()) return true;
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
                                } else if (obj["role"].contains("except")) {
                                    auto v = obj["role"]["except"];
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
                                                    case TargetType::Permission:
                                                        if (p.contains("perm") && p["perm"].is_boolean() && p["perm"] == true) return false;
                                                        if (p["perm"].is_array()) {
                                                            switch (this->action) {
                                                                case Action::Add:
                                                                    if (std::find(p["perm"].begin(), p["perm"].end(), "add") != p["perm"].end()) return false;
                                                                    break;
                                                                case Action::Remove:
                                                                    if (std::find(p["perm"].begin(), p["perm"].end(), "rm") != p["perm"].end()) return false;
                                                                    break;
                                                                case Action::Set:
                                                                    if (std::find(p["perm"].begin(), p["perm"].end(), "set") != p["perm"].end()) return false;
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
            outp.trAddMessage("permapi.cmd.output.view.player.rolesTitle");
            for (auto& role : mod.perm.getPlayerRoles(pl->getXuid())) {
                outp.trAddMessage("  * " + role->displayName);
            }
            outp.trAddMessage("permapi.cmd.output.view.player.permissionsTitle");
            for (auto& perm : mod.perm.getPlayerPermissions(pl->getXuid())) {
                outp.addMessage("  * " + perm.name + ": " + 
                    (mod.perm.permInfoList.contains(perm.name) ?
                        mod.perm.permInfoList[perm.name].desc : pl->tr("permapi.cmd.output.noDesc")));
            }
            return true;
        } else {
            outp.trError("permapi.cmd.error.invalidCommand");
        }
        return false;
    }

    bool handleCreate(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm create role
            case TargetType::Role: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (mod.perm.roles.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.roleAlreadyExists");
                    break;
                }
                auto role = mod.perm.createRole(this->name1, this->name1);
                if (this->priority_set) role->priority = this->priority;
                if (this->displayName_set) role->displayName = this->displayName;
                mod.perm.save();
                outp.trSuccess("permapi.cmd.output.create.role.success");
                return true;
            }
            // perm create perm
            case TargetType::Permission:
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (mod.perm.permInfoList.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.permissionAlreadyExists");
                    break;
                }
                if (!PermInstance::isValidPermissionName(this->name1)) {
                    outp.trError("permapi.cmd.error.invalidPermissionName");
                    outp.trError("permapi.cmd.output.permissionNameExamples");
                    break;
                }
                mod.perm.registerPermission(this->name1, this->desc);
                outp.trSuccess("permapi.cmd.output.create.perm.success");
                return true;
            default:
                outp.trError("permapi.cmd.error.invalidCommand");
                break;
        }
        return false;
    }

    bool handleDelete(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm delete role
            case TargetType::Role: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.roles.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.roleNotFound");
                    break;
                }
                mod.perm.roles.remove(this->name1);
                mod.perm.save();
                outp.trSuccess("permapi.cmd.output.delete.role.success");
                return true;
            }
            // perm delete perm
            case TargetType::Permission:
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.permInfoList.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.permissionNotFound");
                    break;
                }
                mod.perm.deletePermission(this->name1);
                outp.trSuccess("permapi.cmd.output.delete.perm.success");
                return true;
            default:
                outp.trError("permapi.cmd.error.invalidCommand");
                break;
        }
        return false;
    }

    bool handleList(CommandOrigin const& ori, CommandOutput& outp) const {
        switch (this->targetType1) {
            // perm list role
            case TargetType::Role: {
                if (mod.perm.roles.empty()) {
                    outp.trSuccess("permapi.cmd.output.list.role.none");
                    return true;
                }
                outp.trAddMessage("permapi.cmd.output.list.role.header");
                for (auto& role : mod.perm.roles) {
                    std::string msg = "* " + role->displayName;
                    if (!DisplayNameIsSameAs(role->name, role->displayName)) {
                        // If the display name is different from name, append the name
                        msg += " <" + role->name + ">";
                    }
                    if ((OriginType)ori.getOriginType() == OriginType::Player) {
                        auto pl = ori.getPlayer();
                        if (!pl) {
                            outp.trError("permapi.cmd.error.internal");
                            break;
                        }
                        if (mod.perm.isMemberOf(pl->getXuid(), role->name)) {
                            outp.trAddMessage(msg + " " + pl->tr("permapi.cmd.output.list.role.isMember"));
                            continue;
                        }
                    }
                    outp.trAddMessage(msg);
                }
                return true;
            }
            // perm list perm
            case TargetType::Permission:
                if (mod.perm.permInfoList.empty()) {
                    outp.trSuccess("permapi.cmd.output.list.perm.none");
                    return true;
                }
                outp.trAddMessage("permapi.cmd.output.list.perm.header");
                for (auto& perm : mod.perm.permInfoList) {
                    if (perm.desc.empty()) {
                        if ((OriginType)ori.getOriginType() == OriginType::Player) {
                            auto pl = ori.getPlayer();
                            if (!pl) {
                                outp.trError("permapi.cmd.error.internal");
                                break;
                            }  
                            outp.trAddMessage("* " + perm.name + ": " + pl->tr("permapi.cmd.output.noDesc"));
                            continue;
                        }
                        outp.trAddMessage("* " + perm.name + ": " + tr("permapi.cmd.output.noDesc"));
                        continue;
                    }
                    outp.trAddMessage("* " + perm.name + ": " + perm.desc);
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
            // perm view role
            case TargetType::Role: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.roles.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.roleNotFound");
                    break;
                }
                auto& role = mod.perm.roles[this->name1];
                outp.trAddMessage("permapi.cmd.output.view.role.header");
                outp.trAddMessage("permapi.cmd.output.view.role.info.name",
                                  arg("roleName", role->name));
                outp.trAddMessage("permapi.cmd.output.view.role.info.displayName", role->displayName);
                outp.trAddMessage("permapi.cmd.output.view.role.info.priority", std::to_string(role->priority));
                if (role->getType() != Role::Type::Everyone) {
                    if (role->members.empty()) {
                        outp.trAddMessage("permapi.cmd.output.view.role.info.membersNone");
                    } else {
                        outp.trAddMessage("permapi.cmd.output.view.role.info.members");
                        for (auto& xid : role->members) {
                            outp.trAddMessage("  * " + PlayerInfo::fromXuid(xid) + " (" + xid + ")");
                        }
                    }
                }
                if (role->permissions.empty()) {
                    outp.trAddMessage("permapi.cmd.output.view.role.info.permissionsNone");
                } else {
                    outp.trAddMessage("permapi.cmd.output.view.role.info.permissions");
                    for (auto& perm : role->permissions) {
                        std::string suffix = (perm.enabled ? 
                            "permapi.cmd.output.view.role.enabled" :
                            "permapi.cmd.output.view.role.disabled");
                        if (perm.enabled && perm.extra.is_object() && !perm.extra.empty()) {
                            suffix = "permapi.cmd.output.view.role.enabledWithExtra";
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
                        outp.trAddMessage("  * " + perm.name + " " + suffix);
                    }
                }
                // Special cases
                if (role->getType() == Role::Type::Admin) {
                    outp.trAddMessage("permapi.cmd.output.view.role.note.admin", role->displayName);
                } else if (role->getType() == Role::Type::Everyone) {
                    outp.trAddMessage("permapi.cmd.output.view.role.note.everyone", role->displayName);
                }
                return true;
            }
            // perm view perm
            case TargetType::Permission: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.permInfoList.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.permissionNotFound");
                    break;
                }
                auto& perm = mod.perm.permInfoList[this->name1];
                outp.trAddMessage("permapi.cmd.output.view.perm.header");
                outp.trAddMessage("permapi.cmd.output.view.perm.info.name",
                                  arg("permissionName", perm.name));
                outp.trAddMessage("permapi.cmd.output.view.perm.info.desc", perm.desc);
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
                outp.trAddMessage("permapi.cmd.output.view.player.rolesTitle");
                for (auto& role : mod.perm.getPlayerRoles(target)) {
                    outp.trAddMessage("  * " + role->displayName);
                }
                outp.trAddMessage("permapi.cmd.output.view.player.permissionsTitle");
                for (auto& perm : mod.perm.getPlayerPermissions(target)) {
                    if ((OriginType)ori.getOriginType() == OriginType::Player) {
                        auto pl = ori.getPlayer();
                        if (!pl) {
                            outp.trError("permapi.cmd.error.internal");
                            break;
                        }
                        outp.addMessage("  * " + perm.name + ": " + 
                            (mod.perm.permInfoList.contains(perm.name) ?
                                mod.perm.permInfoList[perm.name].desc : pl->tr("permapi.cmd.output.noDesc")));
                        continue;
                    }
                    outp.addMessage("  * " + perm.name + ": " + 
                        (mod.perm.permInfoList.contains(perm.name) ?
                            mod.perm.permInfoList[perm.name].desc : tr("permapi.cmd.output.noDesc")));
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
            // perm update role
            case TargetType::Role: {
                if (this->name1.empty()) {
                    outp.trError("permapi.cmd.error.invalidCommand");
                    break;
                }
                if (!mod.perm.roles.contains(this->name1)) {
                    outp.trError("permapi.cmd.error.roleNotFound");
                    break;
                }
                auto& role = mod.perm.roles[this->name1];
                switch (this->targetType2) {
                    // perm update role ... member    
                    case TargetType::Member: {
                        if (role->getType() == Role::Type::Everyone) {
                            outp.trError("permapi.cmd.error.modifyMembersOfEveryone");
                            return false;
                        }
                        switch (this->action) {
                            // perm update role add member
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
                                role->addMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.role.member.add.success",
                                               arg("name", this->name2),
                                               arg("xuid", xid),
                                               arg("roleDisplayName", role->displayName));
                                return true;
                            }
                            // perm update role remove member
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
                                role->removeMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.role.member.remove.success",
                                               arg("name", this->name2),
                                               arg("xuid", xid),
                                               arg("roleDisplayName", role->displayName));
                                return true;
                            }
                            default:
                                outp.trError("permapi.cmd.error.invalidCommand");
                                break;
                        }
                        break;
                    }
                    // perm update role ... perm
                    case TargetType::Permission: {
                        switch (this->action) {
                            // perm update role add perm
                            case Action::Add: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                if (!mod.perm.permInfoList.contains(this->name2)) {
                                    outp.trError("permapi.cmd.error.permissionNotFound");
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
                                role->setPermission(this->name2, enabled_set ? this->enabled : true, extraJson);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.role.perm.add.success",
                                               arg("name", this->name2),
                                               arg("roleDisplayName", role->displayName));
                                return true;
                            }
                            // perm update role add perm
                            case Action::Set: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                if (!mod.perm.permInfoList.contains(this->name2)) {
                                    outp.trError("permapi.cmd.error.permissionNotFound");
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
                                role->setPermission(this->name2, this->enabled, extraJson);
                                mod.perm.save();
                                std::string suffix = (this->enabled ? 
                                    "permapi.cmd.output.update.role.perm.set.enabled" :
                                    "permapi.cmd.output.update.role.perm.set.disabled");
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
                                outp.trSuccess("permapi.cmd.output.update.role.perm.set.success",
                                               arg("name", this->name2),
                                               arg("roleDisplayName", role->displayName),
                                               arg("enable", suffix));
                                return true;
                            }
                            // perm update role remove perm
                            case Action::Remove: {
                                if (this->name2.empty()) {
                                    outp.trError("permapi.cmd.error.invalidCommand");
                                    break;
                                }
                                if (!mod.perm.permInfoList.contains(this->name2)) {
                                    outp.trError("permapi.cmd.error.permissionNotFound");
                                    break;
                                }
                                role->removePermission(this->name2);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.role.perm.remove.success",
                                               arg("name", this->name2),
                                               arg("roleDisplayName", role->displayName));
                                return true;
                            }
                            default:
                                outp.trError("permapi.cmd.error.invalidCommand");
                                break;
                        }
                        break;
                    }
                    // perm update role set priority
                    case TargetType::Priority: {
                        if (this->action != Action::Set) {
                            outp.trError("permapi.cmd.error.invalidCommand");
                            break;
                        }
                        if (!this->priority_set) {
                            outp.trError("permapi.cmd.error.missingArgument", "priority");
                            break;
                        }
                        role->priority = this->priority;
                        mod.perm.save();
                        outp.trSuccess("permapi.cmd.output.update.role.priority.set.success",
                                       arg("roleDisplayName", role->displayName),
                                       arg("priority", this->priority));
                        return true;
                    }
                    // perm update role set display_name
                    case TargetType::DisplayName: {
                        if (this->action != Action::Set) {
                            outp.trError("permapi.cmd.error.invalidCommand");
                            break;
                        }
                        if (!this->displayName_set) {
                            outp.trError("permapi.cmd.error.missingArgument", "display_name");
                            break;
                        }
                        role->displayName = this->displayName;
                        mod.perm.save();
                        outp.trSuccess("permapi.cmd.output.update.role.displayName.set.success",
                                       arg("roleName", role->name),
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
                    case TargetType::Role: {
                        if (this->name2.empty()) {
                            outp.trError("permapi.cmd.error.invalidCommand");
                            break;
                        }
                        if (!mod.perm.roles.contains(this->name2)) {
                            outp.trError("permapi.cmd.error.roleNotFound");
                            break;
                        }
                        auto& role = mod.perm.roles[this->name2];
                        switch (this->action) {
                            // perm update player add role
                            case Action::Add: {
                                role->addMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.player.role.add.success",
                                               arg("name", this->name1),
                                               arg("xuid", xid),
                                               arg("roleDisplayName", role->displayName));
                                return true;
                            }
                            // perm update player remove role
                            case Action::Remove: {
                                role->removeMember(xid);
                                mod.perm.save();
                                outp.trSuccess("permapi.cmd.output.update.player.role.remove.success",
                                               arg("name", this->name1),
                                               arg("xuid", xid),
                                               arg("roleDisplayName", role->displayName));
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
        reg->registerCommand("perm", "Permission role system",
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
        
        reg->addEnum<TargetType>("Role", {{"role", TargetType::Role}});
        reg->addEnum<TargetType>("Permission", {{"perm", TargetType::Permission}});
        reg->addEnum<TargetType>("Player", {{"player", TargetType::Player}});
        reg->addEnum<TargetType>("Priority", {{"priority", TargetType::Priority}});
        reg->addEnum<TargetType>("Member", {{"member", TargetType::Member}});
        reg->addEnum<TargetType>("DisplayName", {{"display_name", TargetType::DisplayName}});

        auto target_role1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Role1", "Role");
        auto target_perm1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Permission1", "Permission");
        auto target_player1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Player1", "Player");
        auto target_priority1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Priority1", "Priority");
        auto target_member1 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType1, "Member1", "Member");
        
        auto target_role2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "Role2", "Role");
        auto target_perm2 = makeMandatory<CommandParameterDataType::ENUM>(&PermCommand::targetType2, "Permission2", "Permission");
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
        // perm create role <Name> [DisplayName] [Priority]
        reg->registerOverload<PermCommand>("perm", subcmd_create, target_role1, param_name1, param_displayName, param_priority);
        // perm delete perm <Name> [Desc]
        reg->registerOverload<PermCommand>("perm", subcmd_create, target_perm1, param_name1, param_desc);
        // perm delete role <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_delete, target_role1, param_name1);
        // perm delete perm <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_delete, target_perm1, param_name1);
        // perm update role <Name> add member <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_role1, param_name1, action_add, target_member2, param_name2);
        // perm update role <Name> rm member <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_role1, param_name1, action_remove, target_member2, param_name2);
        // perm update role <Name> add perm <Name> [Enabled] [ExtraJson]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_role1, param_name1, action_add, target_perm2, param_name2, param_enabled, param_extra);
        // perm update role <Name> set perm <Name> [Enabled] [ExtraJson]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_role1, param_name1, action_set, target_perm2, param_name2, param_enabled, param_extra);
        // perm update role <Name> rm perm <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_role1, param_name1, action_remove, target_perm2, param_name2);
        // perm update role <Name> set priority [Priority]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_role1, param_name1, action_set, target_priority2, param_priority);
        // perm update role <Name> set display_name [DisplayName]
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_role1, param_name1, action_set, target_display_name2, param_displayName);
        // perm update player <Name> add role <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_player1, param_name1, action_add, target_role2, param_name2);
        // perm update player <Name> rm role <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_update, target_player1, param_name1, action_remove, target_role2, param_name2);
        // perm view role <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_role1, param_name1);
        // perm view perm <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_perm1, param_name1);
        // perm view player <Name>
        reg->registerOverload<PermCommand>("perm", subcmd_view, target_player1, param_name1);
        // perm list role
        reg->registerOverload<PermCommand>("perm", subcmd_list, target_role1);
        // perm list perm
        reg->registerOverload<PermCommand>("perm", subcmd_list, target_perm1);
    }
};

void SetupAllCmds(CommandRegistry* reg) {
    PermCommand::setup(reg);
}