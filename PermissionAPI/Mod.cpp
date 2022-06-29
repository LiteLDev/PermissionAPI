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
        {"Invalid command. Type '/help perm' to get help.", "无效的命令，输入'/help perm'以获取帮助"},
        {"Failed to process the data file: {}", "处理数据文件失败: {}"},
        {"Failed to read the data file at {}", "处理位于 {} 的数据文件失败"},
        {"§bYour current permission status:", "§b你当前的权限状态:"},
        {"§e- Your Groups:", "§e- 你所在的权限组:"},
        {"§e- Your Abilities:", "§e- 你的能力:"},
        {"[No description]", "[无描述]"},
        {"Group name '{}' contains invalid characters.", "权限组名 '{}' 包含无效字符"},
        {"Group name '{}' has been replaced with '{}'.", "权限组名 '{}' 已改为 '{}'"}
    }}
};

Mod::Mod()
    : logger("Perm") {
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
                "list": {
                    "group": {
                        //"enabled": true,
                        "hide": ["group-a"]
                    },
                    "ability": {
                        //"enabled": true,
                        "hide": ["namespc:ability-a"]
                    }
                },
                "update": {
                    "group": {
                        "only": [
                            {
                                "name": "group-a",
                                "member": true, // = "member": ["add", "rm"]
                                "priority": false, // = "priority": ["set"]
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
        perm.registerAbility("PermissionAPI", "cmd_control", "Access to /perm commands");
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
