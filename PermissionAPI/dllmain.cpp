#include "pch.h"
#include "Mod.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            ll::registerPlugin("PermissionAPI",
                               "Builtin & Powerful permission API for LiteLoaderBDS",
                               PERM_VER,
                               std::map<std::string, std::string>{
                                   {"Author", "Jasonzyt(LiteLDev)"},
                                   {"GitHub", "https://github.com/LiteLDev/PermissionAPI"},
                                   {"License", "LGPLv3"}
                               });
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

extern "C"
__declspec(dllexport)
void onPostInit() {
    mod.entry();
}
