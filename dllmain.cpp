#pragma warning(disable:4996)

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

#define SOKU_MODULE	extern "C" __declspec(dllexport) void
#define DAT_B(ADDR) *((byte*)ADDR)
#define DAT_W(ADDR) *((short*)ADDR)
#define DAT_DW(ADDR) *((int*)ADDR)

#define CTITLE_INIT_ADDR 0x0427d00
#define CTITLE_INIT_CALL_ADDR 0x041e525

typedef DWORD(__thiscall * CTitleInit_fun)(void*);

enum {
    SCENE_INTRO,
    SCENE_UNKNOWN,
    SCENE_CTITLE,
    SCENE_CSELECT
};

auto PlayBGM = (void (*)(const char*))0x043ff10;
auto LoadGraphicsFun = (void (*)())0x408410;
int* SceneIDNew = (int*)0x8A0040;
int* SceneIDOld = (int*)0x8A0044;
HANDLE* LGThread = (HANDLE*)0x089fff4;

int* CSelectType = (int*)0x898690;
int* CSelectSubtype = (int*)0x898688;

struct PlayerInfo {
    int Character;
    byte padding1;
    byte Palette;
    byte padding2;
    byte Deck;
};

PlayerInfo* P1Info = (PlayerInfo*)0x0899d10;
PlayerInfo* P2Info = (PlayerInfo*)0x0899d30;

char ConfigPath[1024 + MAX_PATH] = "./Modules/SkipIntro/SkipIntro.ini";

CTitleInit_fun CTitleInit;

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

extern "C" __declspec(dllexport) void EntryPoint()
{
    int Scene = GetPrivateProfileIntA("GLOBAL", "scene_id", 0, ConfigPath);

    if (Scene == SCENE_CSELECT) {
        LoadPlayerConfig(P1Info, "P1");
        LoadPlayerConfig(P2Info, "P2");

        int Type = GetPrivateProfileIntA("GLOBAL", "type", 0, ConfigPath);
        int Subtype = GetPrivateProfileIntA("GLOBAL", "subtype", 0, ConfigPath);

        GotoCSelectTrimmed(Type, Subtype);
    }
    else if (Scene == SCENE_CTITLE) {
        //PatchMan::MultiPatch().AddNOPs(0x00428144, 12)
        //                      .AddPatch(0x004278ac, "\x90\x90\x6a\x00", 4)
        //                      .Toggle(true);
        //All of this used to be necessary (and buggy), then I realized 
        //the game auto-skips the intro stuff when you come from CSelect.
        *SceneIDOld = SCENE_CSELECT;
    }
    *SceneIDNew = Scene;

    *LGThread = CreateThread((LPSECURITY_ATTRIBUTES)0x0, 0, (LPTHREAD_START_ROUTINE)LoadGraphicsFun, (LPVOID)0x0, 0, (LPDWORD)0x089fff8);
}