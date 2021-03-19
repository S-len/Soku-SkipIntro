#pragma warning(disable:4996)

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include "Soku.hpp"

typedef bool (__thiscall* SokuSetup_fun)(void*, Soku::SetupConfig*);

char ConfigPath[1024 + MAX_PATH];

SokuSetup_fun Old_SokuSetup;

inline DWORD HookNear(DWORD addr, DWORD target) {
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 5, PAGE_READWRITE, &oldProtect))
        return 0;

    DWORD old = (*(DWORD*)(addr + 1)) + (addr + 5);
    *((DWORD*)(addr + 1)) = target - (addr + 5);

    if (!VirtualProtect((void*)addr, 5, oldProtect, &oldProtect))
        return 0;

    return old;
}

void LoadPlayerConfig(Soku::PlayerInfo* p, const char* key) {
    p->Character = GetPrivateProfileIntA(key, "character", 0, ConfigPath);
    p->Palette = GetPrivateProfileIntA(key, "palette", 0, ConfigPath);
    p->Deck = GetPrivateProfileIntA(key, "deck", 0, ConfigPath);
}

bool __fastcall Hooked_SokuSetup(void *DxWinHwnd, void *EDX, Soku::SetupConfig *config) {
    SceneEnum Scene = (SceneEnum)GetPrivateProfileIntA("GLOBAL", "scene_id", 0, ConfigPath);
    config->mStartSceneID = Scene;

    bool ret = Old_SokuSetup(DxWinHwnd, config);
    
    if (Scene == SCENE_CSELECT) {
        LoadPlayerConfig(Soku::P1Info, "P1");
        LoadPlayerConfig(Soku::P2Info, "P2");

        int Type = GetPrivateProfileIntA("GLOBAL", "type", 0, ConfigPath);
        int Subtype = GetPrivateProfileIntA("GLOBAL", "subtype", 0, ConfigPath);

        Soku::GotoCSelect(Type, Subtype);
    }
    else if (Scene == SCENE_CTITLE) {
        MenuEnum Menu = (MenuEnum)GetPrivateProfileIntA("GLOBAL", "menu_id", 0, ConfigPath);
        if (Menu) {
            Soku::AddCMenuObj(Soku::CreateMenu(Menu));
        }
    }

    return ret;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
    return TRUE;
}

extern "C" __declspec(dllexport) void Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
    if (__argc == 2 && StrStrI(__argv[1], "rep")) {
        // don't try to hook anything if we're fast-starting a replay
        return;
    }

    GetModuleFileNameA(hMyModule, ConfigPath, 1024);
    PathRemoveFileSpecA(ConfigPath);
    PathAppendA(ConfigPath, "SkipIntro.ini");

    Old_SokuSetup = (SokuSetup_fun)HookNear(SOKUSETUP_CALL_ADDR, (DWORD)Hooked_SokuSetup);
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    return TRUE;
}