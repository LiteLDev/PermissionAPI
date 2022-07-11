#include "pch.h"
#include <LLAPI.h>

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
#if defined(SCRIPT_JS)
            LL::registerPlugin("PermissionAPI-Js", "PermissionAPI support for LLSE-JS", PERM_VER,
                               std::map<std::string, std::string>{
                                   {"Author", "Jasonzyt(LiteLDev)"},
                                   {"GitHub", "https://github.com/LiteLDev/PermissionAPI"},
                                   {"License", "AGPLv3"}});
#elif defined(SCRIPT_LUA)
            LL::registerPlugin("PermissionAPI-Lua", "PermissionAPI support for LLSE-LUA", PERM_VER,
                               std::map<std::string, std::string>{
                                   {"Author", "Jasonzyt(LiteLDev)"},
                                   {"GitHub", "https://github.com/LiteLDev/PermissionAPI"},
                                   {"License", "AGPLv3"}});
#endif
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

void entry();

extern "C"
__declspec(dllexport)
void onPostInit() {
    entry();
}
