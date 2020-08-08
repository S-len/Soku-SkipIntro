#pragma warning(disable:4996)

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include "Soku.hpp"

char ConfigPath[1024 + MAX_PATH];

CSceneInit_fun CLogoInit;

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
    int Scene = GetPrivateProfileIntA("GLOBAL", "scene_id", 0, ConfigPath);

    *Soku::SceneID_New = Scene;

    if (Scene == SCENE_CSELECT) {
        Soku::Delete(CScene);

        LoadPlayerConfig(Soku::P1Info, "P1");
        LoadPlayerConfig(Soku::P2Info, "P2");

        int Type = GetPrivateProfileIntA("GLOBAL", "type", 0, ConfigPath);
        int Subtype = GetPrivateProfileIntA("GLOBAL", "subtype", 0, ConfigPath);

        Soku::GotoCSelect(Type, Subtype);

        CScene = Soku::New(CSELECT_SIZE);

        return Soku::CSelectInit(CScene);
    }
    else if (Scene == SCENE_CTITLE) {
        Soku::Delete(CScene);

        //Make the game think we came from CharSelect so it skips the intro
        *Soku::SceneID_Old = SCENE_CSELECT;

        CScene = Soku::New(CTITLE_SIZE);

        void* ret = Soku::CTitleInit(CScene);

        MenuEnum Menu = (MenuEnum)GetPrivateProfileIntA("GLOBAL", "menu_id", 0, ConfigPath);
        if (Menu) {
            Soku::AddCMenuObj(Soku::CreateMenu(Menu));
        }

        return ret;
    }
    else {
        *Soku::LGThread = CreateThread((LPSECURITY_ATTRIBUTES)0x0, 0, (LPTHREAD_START_ROUTINE)Soku::LoadGraphicsFun, (LPVOID)0x0, 0, (LPDWORD)0x089fff8);

        return CLogoInit(CScene);
    }
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
    return TRUE;
}

extern "C" __declspec(dllexport) void Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
    GetModuleFileNameA(hMyModule, ConfigPath, 1024);
    PathRemoveFileSpecA(ConfigPath);
    PathAppendA(ConfigPath, "SkipIntro.ini");

    CLogoInit = (CSceneInit_fun)HookNear(CLOGO_INIT_CALL_ADDR, (DWORD)Setup);
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    return TRUE;
}