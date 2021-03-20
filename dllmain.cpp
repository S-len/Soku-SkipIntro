#pragma warning(disable:4996)

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include "Soku.hpp"

char ConfigPath[1024 + MAX_PATH];

Init_fun CLogoInit;

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

void* __fastcall Setup(void *CScene) {
    SceneEnum Scene = (SceneEnum)GetPrivateProfileIntA("GLOBAL", "scene_id", 0, ConfigPath);

    *Soku::SceneID_New = Scene;

    Soku::Delete(CScene);

    if (Scene == SCENE_CSELECT) {
        LoadPlayerConfig(Soku::P1Info, "P1");
        LoadPlayerConfig(Soku::P2Info, "P2");

        int Type = GetPrivateProfileIntA("GLOBAL", "type", 0, ConfigPath);
        int Subtype = GetPrivateProfileIntA("GLOBAL", "subtype", 0, ConfigPath);

        Soku::GotoCSelect(Type, Subtype);

        return Soku::CreateScene(Scene);
    }
    else if (Scene == SCENE_CTITLE) {
        //Make the game think we came from CharSelect so it skips the intro
        *Soku::SceneID_Old = SCENE_CSELECT;

        void* ret = Soku::CreateScene(Scene);

        MenuEnum Menu = (MenuEnum)GetPrivateProfileIntA("GLOBAL", "menu_id", 0, ConfigPath);
        if (Menu) {
            Soku::AddCMenuObj(Soku::CreateMenu(Menu));
        }

        return ret;
    }
    else return Soku::CreateScene(Scene);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
    return TRUE;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
    if (__argc == 2 && !StrStrI(__argv[1], ".ini")) {
        // don't try to hook anything if we're fast-starting a replay
        return true;
    }

    GetModuleFileNameA(hMyModule, ConfigPath, 1024);
    PathRemoveFileSpecA(ConfigPath);
    PathAppendA(ConfigPath, "SkipIntro.ini");

    CLogoInit = (Init_fun)HookNear(CLOGO_INIT_CALL_ADDR, (DWORD)Setup);
    return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    return TRUE;
}