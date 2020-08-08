#pragma once
#include <Windows.h>

#define CLOGO_INIT_CALL_ADDR 0x041e477

#define CTITLE_SIZE 0x698
#define CSELECT_SIZE 0x50c0

#define DAT_B(ADDR) *((byte*)ADDR)
#define DAT_W(ADDR) *((short*)ADDR)
#define DAT_DW(ADDR) *((int*)ADDR)

enum SceneEnum {
    SCENE_INTRO,
    SCENE_UNKNOWN,
    SCENE_CTITLE,
    SCENE_CSELECT
};

enum MenuEnum {
    MENU_NONE,
    MENU_CONNECT,
    MENU_REPLAY,
    MENU_MUSICROOM,
    MENU_RESULT,
    MENU_PROFILE,
    MENU_CONFIG,
    MENU_COUNT
};


typedef void* (__thiscall* CMenuInit_fun)(void*);
typedef void* (__thiscall* CSceneInit_fun)(void*);

//Not related to SokuEngine
namespace Soku {
    struct CMenuInfo {
        size_t Size;
        CMenuInit_fun Init;
    };

    struct PlayerInfo {
        int Character;
        byte padding1;
        byte Palette;
        byte padding2;
        byte Deck;
    };

    CMenuInfo Menus[MENU_COUNT] = {
        {0x0, (CMenuInit_fun)0x0},          //None
        {0x118C, (CMenuInit_fun)0x0448760}, //Connect
        {0x544, (CMenuInit_fun)0x044b890},  //Replay
        {0x1F4, (CMenuInit_fun)0x0449d80},  //Music Room
        {0x1674, (CMenuInit_fun)0x044c510}, //Result
        {0xD9C, (CMenuInit_fun)0x0453390},  //Profile
        {0x2B0, (CMenuInit_fun)0x0445a20},  //Config
    };

    int* SceneID_New = (int*)0x8A0040;
    int* SceneID_Old = (int*)0x8A0044;

    HANDLE* LGThread = (HANDLE*)0x89fff4;
    auto LoadGraphicsFun = (void (*)())0x408410;

    //Apprently you have to use Soku's new and delete
    auto New = (void* (__cdecl*)(DWORD))0x81fbdc;
    auto Delete = (void(__cdecl*)(void*))0x81F6FA;

    auto CTitleInit = (CSceneInit_fun)0x427d00;
    auto CSelectInit = (CSceneInit_fun)0x424280;

    auto AddCMenuObj = (void (*)(void*))0x43e130;

    int* CSelectType = (int*)0x898690;
    int* CSelectSubtype = (int*)0x898688;

    PlayerInfo* P1Info = (PlayerInfo*)0x899d10;
    PlayerInfo* P2Info = (PlayerInfo*)0x899d30;

    void GotoCSelect(int Type, int Subtype) {
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

    inline void* CreateMenu(MenuEnum id) {
        return Menus[id].Init(New(Menus[id].Size));
    }
}