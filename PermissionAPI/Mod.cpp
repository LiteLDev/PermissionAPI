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

const I18nBase::LangData Mod::defaultLangData = {
    {"zh_CN", {
        {"Invalid command. Type '/help perm' to get help.", "无效的命令, 输入'/help perm'以获取帮助"},
        {"Failed to process the data file: {}", "处理数据文件失败: {}"},
        {"Failed to read the data file at {}", "处理位于 {} 的数据文件失败"},
        {"§b§lYour current permission status:", "§b§l你当前的权限状态:"},
        {"§b§l{}'s current permission status:", "§b§l{} 当前的权限状态:"},
        {"- §eGroups:", "- §e权限组:"},
        {"- §eAbilities:", "- §e能力:"},
        {"[No description]", "[无描述]"},
        {"Group name '{}' contains invalid characters.", "权限组名 '{}' 包含无效字符"},
        {"Group name '{}' has been replaced with '{}'.", "权限组名 '{}' 已改为 '{}'"},
        {"Internal error. Please try again later.", "内部错误, 请稍后再试"},
        {"You don't have permission to use this command.", "您没有使用该命令的权限"},
        {"Group already exists.", "此权限组已存在"},
        {"Group created.", "权限组创建成功"},
        {"Ability already exists.", "此能力已存在"},
        {"Ability created.", "能力创建成功"},
        {"Ability removed.", "能力删除成功"},
        {"Group removed.", "权限组删除成功"},
        {"Invalid group name.", "无效的权限组名"},
        {"Invalid ability name.", "无效的能力名"},
        {"Ability name examples: 'Namespace:AbilityName', 'a:b:c:d_e_f.g'", "能力名样例: '命名空间:能力名', 'a:b:c:d_e_f.g'"},
        {"Group does not exist.", "权限组不存在"},
        {"Ability does not exist.", "能力不存在"},
        {"Group deleted.", "权限组删除成功"},
        {"Ability deleted.", "能力删除成功"},
        {"No groups.", "无权限组"},
        {"No abilities.", "无能力"},
        {"Member", "成员"},
        {"Player not found", "找不到玩家"},
        {"+ §eMembers§r: None", "+ §e成员§r: 无"},
        {"+ §eMembers§r:", "+ §e成员§r:"},
        {"+ §eName§r: {}", "+ §e名称§r: {}"},
        {"+ §eDisplayName§r: {}", "+ §e显示名称§r: {}"},
        {"+ §ePriority§r: {}", "+ §e优先级§r: {}"},
        {"+ §eAbilities§r: None", "+ §e能力§r: 无"},
        {"+ §eAbilities§r:", "+ §e能力§r:"},
        {"※ {}§r is a special group. All the abilities will be enabled by default unless it has been disabled in the config.", "※ {}§r 是一个特殊的权限组, 所有的能力将会默认开启除非它在配置文件中被禁用"},
        {"※ {}§r is a special group. All players are members of this group.", "※ {}§r 是一个特殊的权限组, 所有的玩家都是这个权限组的成员"},
        {"The group is a everyone group. You cannot modify the members of it.", "这是一个\"所有人\"权限组, 你不能修改它的成员"},
        {"Member {}({}) added.", "成员 {}({}) 已添加"},
        {"Member {}({}) removed.", "成员 {}({}) 已移除"},
        {"Ability {} added.", "能力 {} 已添加"},
        {"Ability {} set to {}.", "能力 {} 已设置为 {}"},
        {"Ability {} removed.", "能力 {} 已移除"},
        {"Missing argument: enabled", "找不到命令参数: enabled"},
        {"§aenabled", "§a已启用"},
        {"§cdisabled", "§c已禁用"},
        {"Priority set to {}.", "优先级已设置为 {}"},
        {"Display name set to {}.", "显示名称已设置为 {}"},
        {"JSON parsing error: {}", "JSON解析错误: {}"},
        {"§b§lGroup List:", "§b§l权限组列表:"},
        {"§b§lAbility List:", "§b§l权限能力列表:"},
    }}
};

Mod::Mod()
    : logger(Logger("Perm")) {
}

void Mod::entry() {
    logger.info("PermissionAPI v{} loaded! Author: Jasonzyt", PERM_VER.toString(true));
    auto i18n = Translation::load(LANG_FILE, "en_US", Mod::defaultLangData);
    perm.load();
    // Register plugin permissions
    if (!perm.abilitiesInfo.contains("PermissionAPI:cmd_control")) {
        /*
        {
            "enabled": true,
            "subcommands": {
                "create": {
                    "group": true,
                    "ability": true
                },
                "delete": {
                    "group": true,
                    "ability": true
                },
                "view": {
                    "group": {
                        //"enabled": true,
                        "only": ["group-a", "everyone"],
                        //"except": ["group-b"] // only and except cant be used together, if they are both set, only will be used.
                    },
                    "ability": true,
                    "player": {
                        "only": [
                            "[xuid]"
                        ]
                    }
                },
                "list": true,
                "update": {
                    "group": {
                        "only": [
                            {
                                "name": "group-a",
                                "member": true, // = "member": ["add", "rm"]
                                "priority": true, // = "priority": ["set"]
                                "ability": true, // = "abilities": ["add", "rm", "set"]
                            },
                            "group-b", // default allow all operations
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
        perm.registerAbility("PermissionAPI:cmd_control", "Access to /perm commands");
    }
    // Events
    Event::ServerStartedEvent::subscribe([&](const Event::ServerStartedEvent& ev) { 
        auto lang = I18n::getCurrentLanguage().get()->getFullLanguageCode();
        logger.debug("Switch language: {}", lang);
        if (!i18n) {
            i18n->defaultLocaleName = lang;
        }
        return true;
    });
    Event::RegCmdEvent::subscribe([&](const Event::RegCmdEvent& ev) {
        SetupAllCmds(ev.mCommandRegistry);
        return true;
    });
    Schedule::repeat([&] {
        mod.perm.save(); // auto save
    }, 100); // 5s
}
