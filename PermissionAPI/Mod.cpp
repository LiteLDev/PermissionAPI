#include "pch.h"
#include <MC/PropertiesSettings.hpp>
#include <MC/I18n.hpp>
#include <MC/Localization.hpp>
#include <EventAPI.h>
#include "Mod.h"

void SetupAllCmds(CommandRegistry* reg);

Mod mod;
Logger& logger = mod.logger;

const I18N::LangData Mod::defaultLangData = {
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
        {"Player not found", "找不到玩家"}
    }}
};

Mod::Mod()
    : logger(Logger("Perm")) {
}

void Mod::entry() {
    logger.info("PermissionAPI v{} loaded! Author: Jasonzyt");
    auto& i18n = *Translation::load(LANG_FILE, "en_US", Mod::defaultLangData);
    perm.load();
    // Register plugin permissions
    if (perm.abilitiesInfo.contains("PermissionAPI:cmd_control")) {
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
        i18n.defaultLangCode = lang;
        return true;
    });
    Event::RegCmdEvent::subscribe([&](const Event::RegCmdEvent& ev) {
        SetupAllCmds(ev.mCommandRegistry);
        return true;
    });
}
