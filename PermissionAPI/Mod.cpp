#include "pch.h"
#include <MC/PropertiesSettings.hpp>
#include <MC/I18n.hpp>
#include <MC/Localization.hpp>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include "Mod.h"

void SetupAllCmds(CommandRegistry* reg);

Mod mod;
Logger& logger = mod.logger;

/*
const I18nBase::LangData Mod::defaultLangData = {
    {"zh_CN", {
        {"Invalid command. Type '/help perm' to get help.", "无效的命令, 输入'/help perm'以获取帮助"},
        {"Failed to process the data file: {}", "处理数据文件失败: {}"},
        {"Failed to read the data file at {}", "处理位于 {} 的数据文件失败"},
        {"§b§lYour current permission status:", "§b§l你当前的权限状态:"},
        {"§b§l{}'s current permission status:", "§b§l{} 当前的权限状态:"},
        {"- §eRoles:", "- §e权限组:"},
        {"- §e$1bilities:", "- §e能力:"},
        {"[No description]", "[无描述]"},
        {"Role name '{}' contains invalid characters.", "权限组名 '{}' 包含无效字符"},
        {"Role name '{}' has been replaced with '{}'.", "权限组名 '{}' 已改为 '{}'"},
        {"Internal error. Please try again later.", "内部错误, 请稍后再试"},
        {"You don't have permission to use this command.", "您没有使用该命令的权限"},
        {"Role already exists.", "此权限组已存在"},
        {"Role created.", "权限组创建成功"},
        {"$1bility already exists.", "此能力已存在"},
        {"$1bility created.", "能力创建成功"},
        {"$1bility removed.", "能力删除成功"},
        {"Role removed.", "权限组删除成功"},
        {"Invalid role name.", "无效的权限组名"},
        {"Invalid $1bility name.", "无效的能力名"},
        {"$1bility name examples: 'Namespace:$1bilityName', 'a:b:c:d_e_f.g'", "能力名样例: '命名空间:能力名', 'a:b:c:d_e_f.g'"},
        {"Role does not exist.", "权限组不存在"},
        {"$1bility does not exist.", "能力不存在"},
        {"Role deleted.", "权限组删除成功"},
        {"$1bility deleted.", "能力删除成功"},
        {"No roles.", "无权限组"},
        {"No permissions.", "无能力"},
        {"Member", "成员"},
        {"Player not found", "找不到玩家"},
        {"+ §eMembers§r: None", "+ §e成员§r: 无"},
        {"+ §eMembers§r:", "+ §e成员§r:"},
        {"+ §eName§r: {}", "+ §e名称§r: {}"},
        {"+ §eDisplayName§r: {}", "+ §e显示名称§r: {}"},
        {"+ §ePriority§r: {}", "+ §e优先级§r: {}"},
        {"+ §e$1bilities§r: None", "+ §e能力§r: 无"},
        {"+ §e$1bilities§r:", "+ §e能力§r:"},
        {"※ {}§r is a special role. All the permissions will be enabled by default unless it has been disabled in the config.", "※ {}§r 是一个特殊的权限组, 所有的能力将会默认开启除非它在配置文件中被禁用"},
        {"※ {}§r is a special role. All players are members of this role.", "※ {}§r 是一个特殊的权限组, 所有的玩家都是这个权限组的成员"},
        {"The role is a everyone role. You cannot modify the members of it.", "这是一个\"所有人\"权限组, 你不能修改它的成员"},
        {"Member {}({}) added.", "成员 {}({}) 已添加"},
        {"Member {}({}) removed.", "成员 {}({}) 已移除"},
        {"$1bility {} added.", "能力 {} 已添加"},
        {"$1bility {} set to {}.", "能力 {} 已设置为 {}"},
        {"$1bility {} removed.", "能力 {} 已移除"},
        {"Missing argument: enabled", "找不到命令参数: enabled"},
        {"§aenabled", "§a已启用"},
        {"§cdisabled", "§c已禁用"},
        {"Priority set to {}.", "优先级已设置为 {}"},
        {"Display name set to {}.", "显示名称已设置为 {}"},
        {"JSON parsing error: {}", "JSON解析错误: {}"},
        {"§b§lRole List:", "§b§l权限组列表:"},
        {"§b§l$1bility List:", "§b§l权限能力列表:"},
    }}
};
*/

Mod::Mod()
    : logger(Logger("PermAPI")) {
}

void Mod::entry() {
    logger.info("PermissionAPI v{} loaded! Author: Jasonzyt", PERM_VER.toString(true));
    auto i18n = Translation::loadFromImpl(GetCurrentModule(), LL::getLoaderHandle());
    perm.load();
    // Register plugin permissions
    if (!perm.permInfoList.contains("PermissionAPI:cmd_control")) {
        /*
        {
            "enabled": true,
            "subcommands": {
                "create": {
                    "role": true,
                    "$1bility": true
                },
                "delete": {
                    "role": true,
                    "$1bility": true
                },
                "view": {
                    "role": {
                        //"enabled": true,
                        "only": ["role-a", "everyone"],
                        //"except": ["role-b"] // only and except cant be used together, if they are both set, only will be used.
                    },
                    "$1bility": true,
                    "player": {
                        "only": [
                            "[xuid]"
                        ]
                    }
                },
                "list": true,
                "update": {
                    "role": {
                        "only": [
                            {
                                "name": "role-a",
                                "member": true, // = "member": ["add", "rm"]
                                "priority": true, // = "priority": ["set"]
                                "$1bility": true, // = "permissions": ["add", "rm", "set"]
                            },
                            "role-b", // default allow all operations
                        ]
                    },
                    "player": {
                        "only": [
                            "[xuid]"
                        ]
                    }
                }
            }
        }
        */
        perm.registerPermission("PermissionAPI:cmd_control", "Access to /perm commands");
    }
    // Events
    Event::RegCmdEvent::subscribe([&](const Event::RegCmdEvent& ev) {
        SetupAllCmds(ev.mCommandRegistry);
        return true;
    });
    Schedule::repeat([&] {
        mod.perm.save(); // auto save
    }, 100); // 5s
}
