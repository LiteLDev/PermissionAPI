#include "Mod.h"

Mod mod;
Logger& logger = mod.logger;

Mod::Mod()
    : logger("Perm") {
}

void Mod::entry() {
    logger.info("PermissionAPI v{} loaded! Author: Jasonzyt");
    perm.load();
}
