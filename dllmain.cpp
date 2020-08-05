#pragma warning(disable:4996)

#include <iostream>
#include <windows.h>
#include <shlwapi.h>

#define DAT_B(ADDR) *((byte*)ADDR)
#define DAT_W(ADDR) *((short*)ADDR)
#define DAT_DW(ADDR) *((int*)ADDR)

#define CLOGO_INIT_CALL_ADDR 0x041e477

#define CTITLE_SIZE ((void*)0x698)
#define CSELECT_SIZE ((void*)0x50c0)

typedef void*(__thiscall * CSceneInit_fun)(void*);

enum {
    SCENE_INTRO,
    SCENE_UNKNOWN,
    SCENE_CTITLE,
    SCENE_CSELECT
};

struct PlayerInfo {
    int Character;
    byte padding1;
    byte Palette;
    byte padding2;
    byte Deck;
};

int* SceneIDNew = (int*)0x8A0040;
int* SceneIDOld = (int*)0x8A0044;

HANDLE* LGThread = (HANDLE*)0x089fff4;
auto LoadGraphicsFun = (void (*)())0x408410;

//Apprently you have to use Soku's new and delete
auto SokuNew = (void* (__cdecl *)(void*))0x81fbdc;
auto SokuDelete = (void (__cdecl *)(void*))0x81F6FA; 

auto CTitleInit = (CSceneInit_fun)0x0427d00;
auto CSelectInit = (CSceneInit_fun)0x0424280;

int* CSelectType = (int*)0x898690;
int* CSelectSubtype = (int*)0x898688;

PlayerInfo* P1Info = (PlayerInfo*)0x0899d10;
PlayerInfo* P2Info = (PlayerInfo*)0x0899d30;

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

void GotoCSelectTrimmed(int Type, int Subtype) {
    auto FUN_043e8f0 = (int(*)(int))0x043e8f0;

    *CSelectSubtype = Subtype;
    *CSelectType = Type;

    //Input ptr related stuff
    DAT_DW(0x0898680) = 0x08986a8;
    DAT_B(0x0898678) = 0xff;

    //???
    //Crashes after stage select without it though
    FUN_043e8f0(2);
    return;
}

void LoadPlayerConfig(PlayerInfo* p, const char* key) {
    p->Character = GetPrivateProfileIntA(key, "character", 0, ConfigPath);
    p->Palette = GetPrivateProfileIntA(key, "palette", 0, ConfigPath);
    p->Deck = GetPrivateProfileIntA(key, "deck", 0, ConfigPath);
}

void* __fastcall Setup(void *CScene) {
    int Scene = GetPrivateProfileIntA("GLOBAL", "scene_id", 0, ConfigPath);

    *SceneIDNew = Scene;

    if (Scene == SCENE_CSELECT) {
        SokuDelete(CScene);

        LoadPlayerConfig(P1Info, "P1");
        LoadPlayerConfig(P2Info, "P2");

        int Type = GetPrivateProfileIntA("GLOBAL", "type", 0, ConfigPath);
        int Subtype = GetPrivateProfileIntA("GLOBAL", "subtype", 0, ConfigPath);

        GotoCSelectTrimmed(Type, Subtype);

        CScene = SokuNew(CSELECT_SIZE);

        return CSelectInit(CScene);
    }
    else if (Scene == SCENE_CTITLE) {
        SokuDelete(CScene);

        *SceneIDOld = SCENE_CSELECT;

        CScene = SokuNew(CTITLE_SIZE);

        return CTitleInit(CScene);
    }
    else {
        *LGThread = CreateThread((LPSECURITY_ATTRIBUTES)0x0, 0, (LPTHREAD_START_ROUTINE)LoadGraphicsFun, (LPVOID)0x0, 0, (LPDWORD)0x089fff8);

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