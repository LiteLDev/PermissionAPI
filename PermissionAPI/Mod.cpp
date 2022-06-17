#include "Mod.h"

Mod mod;

Mod::Mod()
    : logger("Perm") {
}

void Mod::entry() {
    logger.info("PermissionAPI v{} loaded! Author: Jasonzyt");
}
