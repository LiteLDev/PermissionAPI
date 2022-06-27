#include <MC/PropertiesSettings.hpp>
#include <MC/I18n.hpp>
#include <MC/Localization.hpp>
#include <EventAPI.h>
#include "Mod.h"

Mod mod;
Logger& logger = mod.logger;

const I18N::LangData Mod::defaultLangData = {
    {"zh_CN", {
        {"Invalid command. Type '/help perm' to get help.", "无效的命令，输入'/help perm'以获取帮助"},
        {"", ""}
    }}
};

Mod::Mod()
    : logger("Perm") {
}

void Mod::entry() {
    logger.info("PermissionAPI v{} loaded! Author: Jasonzyt");
    auto& i18n = *Translation::load(LANG_FILE, "en_US", Mod::defaultLangData);
    perm.load();

    Event::ServerStartedEvent::subscribe([&](const Event::ServerStartedEvent& ev) { 
        auto lang = I18n::getCurrentLanguage().get()->getFullLanguageCode();
        logger.debug("Switch language: {}", lang);
        i18n.defaultLangCode = lang;
        return true; 
    });
}
