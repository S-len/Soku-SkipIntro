#pragma once
#include <Windows.h>

#define SOKUSETUP_CALL_ADDR 0x7fb871

#define DAT_B(ADDR) *((byte*)ADDR)
#define DAT_W(ADDR) *((short*)ADDR)
#define DAT_DW(ADDR) *((int*)ADDR)

enum SceneEnum {
    SCENE_INTRO,
    SCENE_UNKNOWN1,
    SCENE_CTITLE,
    SCENE_CSELECT,
    SCENE_EMPTY1,
    SCENE_UNKNOWN2,
    SCENE_UNKNOWN3,
    SCENE_EMPTY2,
    SCENE_UNKNOWN4,
    SCENE_UNKNOWN5,
    SCENE_UNKNOWN6,
    SCENE_EMPTY3,
    SCENE_UNKNOWN7,
    SCENE_UNKNOWN8,
    SCENE_UNKNOWN9,
    SCENE_UNKNOWN10,
    SCENE_UNKNOWN11,
    SCENE_UNKNOWN12,
    SCENE_EMPTY4,
    SCENE_EMPTY5,
    SCENE_EMPTY6,
    SCENE_EMPTY7,
    SCENE_UNKNOWN13,
    SCENE_COUNT
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

typedef void* (__thiscall* Init_fun)(void*);

//Not related to SokuEngine
namespace Soku {
    struct InitInfo {
        size_t Size;
        Init_fun Init;
    };

    InitInfo Menus[] = {
        {0x0   , (Init_fun)0x0      },  //None
        {0x118C, (Init_fun)0x0448760},  //Connect
        {0x544 , (Init_fun)0x044b890},  //Replay
        {0x1F4 , (Init_fun)0x0449d80},  //Music Room
        {0x1674, (Init_fun)0x044c510},  //Result
        {0xD9C , (Init_fun)0x0453390},  //Profile
        {0x2B0 , (Init_fun)0x0445a20},  //Config
    };

    struct PlayerInfo {
        int Character;
        byte padding1;
        byte Palette;
        byte padding2;
        byte Deck;
    };

    struct SetupConfig {
        HWND hwnd;
        HINSTANCE hInstance;
        int field_0x8;
        int field_0xc;
        int field_0x10;
        void* CSceneManagerPtr;
        int mStartSceneID;
        void* CFadePtr;
        bool mDxInitFlag;
        bool HandleInputFlag;
        bool HandleSoundFlag;
        bool field_0x23;
        bool mShowCursorFlag;
        bool field_0x25;
        bool field_0x26;
        bool field_0x27;
    };

    //Apprently you have to use Soku's new and delete
    auto New = (void* (__cdecl*)(DWORD))0x81fbdc;
    auto Delete = (void(__cdecl*)(void*))0x81F6FA;

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