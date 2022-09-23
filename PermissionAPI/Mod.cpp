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

Mod::Mod()
    : logger(Logger("PermAPI")) {
}

void Mod::entry() {
    //logger.info("PermissionAPI v{} loaded! Author: Jasonzyt", PERM_VER.toString(true));
    auto i18n = Translation::loadFromImpl(GetCurrentModule(), ll::getLoaderHandle());
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
