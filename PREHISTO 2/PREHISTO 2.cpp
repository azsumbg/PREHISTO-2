#include "framework.h"
#include "PREHISTO 2.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include <ErrH.h>
#include <FCheck.h>
#include <D2BMPLOADER.h>
#include <Engine.h>
#include <gifresizer.h>
#include <chrono>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "errh.lib")
#pragma comment (lib, "fcheck.lib")
#pragma comment (lib, "d2bmploader.lib")
#pragma comment (lib, "engine.lib")
#pragma comment (lib, "gifresizer.lib")

constexpr wchar_t bWinClassName[]{ L"PREHISTO_2_1" };

constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };

constexpr int mNew{ 1001 };
constexpr int mTurbo{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int no_record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int record{ 2003 };

////////////////////////////////////////////////////

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HICON bIcon{ nullptr };
HCURSOR bCursor{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HWND bHwnd{ nullptr };
HDC PaintDC{ nullptr };
PAINTSTRUCT bPaint{};
MSG bMsg{};
BOOL bRet{ 0 };

POINT cur_pos{};

D2D1_RECT_F b1Rect{ 10.0f, 0, scr_height / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_height / 3, 0, scr_height * 2/ 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_height * 2 / 3, 0, scr_height  - 50.0f, 50.0f };

D2D1_RECT_F b1TxtRect{ 40.0f, 10.0f, scr_height / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2TxtRect{ scr_height / 3 + 30.0f, 10.0f, scr_height * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3TxtRect{ scr_height * 2 / 3 + 30.0f, 10.0f, scr_height - 50.0f, 50.0f };

bool pause = false;
bool in_client = true;
bool show_help = false;
bool sound = true;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;
bool name_set = false;

wchar_t current_player[16]{ L"PREHISTORIK" };

dll::RANDIt RandGen;

int level = 1;
int score = 0;


///////////////////////////////////////////////////////////////

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

ID2D1SolidColorBrush* statBckgBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmText{ nullptr };
IDWriteTextFormat* midText{ nullptr };
IDWriteTextFormat* bigText{ nullptr };

ID2D1Bitmap* bmpField{ nullptr };
ID2D1Bitmap* bmpAxe{ nullptr };
ID2D1Bitmap* bmpGold{ nullptr };
ID2D1Bitmap* bmpPotion{ nullptr };
ID2D1Bitmap* bmpRIP{ nullptr };
ID2D1Bitmap* bmpTree{ nullptr };
ID2D1Bitmap* bmpPlatform1{ nullptr };
ID2D1Bitmap* bmpPlatform2{ nullptr };
ID2D1Bitmap* bmpPlatform3{ nullptr };
ID2D1Bitmap* bmpFireball[16]{ nullptr };
ID2D1Bitmap* bmpIntro[32]{ nullptr };

ID2D1Bitmap* bmpHeroL[4]{ nullptr };
ID2D1Bitmap* bmpHeroR[4]{ nullptr };

ID2D1Bitmap* bmpEvil1L[11]{ nullptr };
ID2D1Bitmap* bmpEvil1R[11]{ nullptr };

ID2D1Bitmap* bmpEvil2L[28]{ nullptr };
ID2D1Bitmap* bmpEvil2R[28]{ nullptr };

ID2D1Bitmap* bmpEvil3L[20]{ nullptr };
ID2D1Bitmap* bmpEvil3R[20]{ nullptr };

ID2D1Bitmap* bmpEvil4L[5]{ nullptr };
ID2D1Bitmap* bmpEvil4R[5]{ nullptr };

ID2D1Bitmap* bmpEvil5L[16]{ nullptr };
ID2D1Bitmap* bmpEvil5R[16]{ nullptr };

ID2D1Bitmap* bmpEvil6L[24]{ nullptr };
ID2D1Bitmap* bmpEvil6R[24]{ nullptr };

//////////////////////////////////////////////////////////////

template<typename T> concept CanBeReleased = requires(T check)
{
    check.Release();
};
template<CanBeReleased T> bool ClrMem(T** var)
{
    if ((*var))
    {
        (*var)->Release();
        (*var) = nullptr;
        return true;
    }
    return false;
};
void LogErr(LPCWSTR what)
{
    std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
    err << what << L" Time stamp of occurrence: " << std::chrono::system_clock::now() << std::endl;
    err.close();
}
void ReleaseResources()
{
    if (!ClrMem(&iFactory))LogErr(L"Error releasing D2D1 iFactory !");
    if (!ClrMem(&Draw))LogErr(L"Error releasing D2D1 Draw !");
    if (!ClrMem(&b1BckgBrush))LogErr(L"Error releasing D2D1 b1BckgBrush !");
    if (!ClrMem(&b2BckgBrush))LogErr(L"Error releasing D2D1 b2BckgBrush !");
    if (!ClrMem(&b3BckgBrush))LogErr(L"Error releasing D2D1 b3BckgBrush !");
    if (!ClrMem(&statBckgBrush))LogErr(L"Error releasing D2D1 statBckgBrush !");
    if (!ClrMem(&txtBrush))LogErr(L"Error releasing D2D1 txtBrush !");
    if (!ClrMem(&hgltBrush))LogErr(L"Error releasing D2D1 hgltBrush !");
    if (!ClrMem(&inactBrush))LogErr(L"Error releasing D2D1 inactBrush !");

    if (!ClrMem(&iWriteFactory))LogErr(L"Error releasing D2D1 iWriteFactory !");
    if (!ClrMem(&nrmText))LogErr(L"Error releasing D2D1 nrmTextFormat !");
    if (!ClrMem(&midText))LogErr(L"Error releasing D2D1 midTextFormat !");
    if (!ClrMem(&bigText))LogErr(L"Error releasing D2D1 bigTextFormat !");

    if (!ClrMem(&bmpField))LogErr(L"Error releasing D2D1 bmpField !");
    if (!ClrMem(&bmpAxe))LogErr(L"Error releasing D2D1 bmpAxe !");
    if (!ClrMem(&bmpGold))LogErr(L"Error releasing D2D1 bmpGold !");
    if (!ClrMem(&bmpPotion))LogErr(L"Error releasing D2D1 bmpPotion !");
    if (!ClrMem(&bmpRIP))LogErr(L"Error releasing D2D1 bmpRIP !");
    if (!ClrMem(&bmpTree))LogErr(L"Error releasing D2D1 bmpTree !");
    if (!ClrMem(&bmpPlatform1))LogErr(L"Error releasing D2D1 bmpPlatform1 !");
    if (!ClrMem(&bmpPlatform2))LogErr(L"Error releasing D2D1 bmpPlatform2 !");
    if (!ClrMem(&bmpPlatform3))LogErr(L"Error releasing D2D1 bmpPlatform3 !");
    for (int i = 0; i < 16; ++i)if (!ClrMem(&bmpFireball[i]))LogErr(L"Error releasing D2D1 bmpFireBall !");
    for (int i = 0; i < 32; ++i)if (!ClrMem(&bmpIntro[i]))LogErr(L"Error releasing D2D1 bmpIntro !");

    for (int i = 0; i < 4; ++i)if (!ClrMem(&bmpHeroL[i]))LogErr(L"Error releasing D2D1 bmpHeroL !");
    for (int i = 0; i < 4; ++i)if (!ClrMem(&bmpHeroR[i]))LogErr(L"Error releasing D2D1 bmpHeroR !");

    for (int i = 0; i < 11; ++i)if (!ClrMem(&bmpEvil1L[i]))LogErr(L"Error releasing D2D1 bmpEvil1L !");
    for (int i = 0; i < 11; ++i)if (!ClrMem(&bmpEvil1R[i]))LogErr(L"Error releasing D2D1 bmpEvil1R !");

    for (int i = 0; i < 28; ++i)if (!ClrMem(&bmpEvil2L[i]))LogErr(L"Error releasing D2D1 bmpEvil2L !");
    for (int i = 0; i < 28; ++i)if (!ClrMem(&bmpEvil2R[i]))LogErr(L"Error releasing D2D1 bmpEvil2R !");

    for (int i = 0; i < 20; ++i)if (!ClrMem(&bmpEvil3L[i]))LogErr(L"Error releasing D2D1 bmpEvil3L !");
    for (int i = 0; i < 20; ++i)if (!ClrMem(&bmpEvil3R[i]))LogErr(L"Error releasing D2D1 bmpEvil3R !");

    for (int i = 0; i < 5; ++i)if (!ClrMem(&bmpEvil4L[i]))LogErr(L"Error releasing D2D1 bmpEvil4L !");
    for (int i = 0; i < 5; ++i)if (!ClrMem(&bmpEvil4R[i]))LogErr(L"Error releasing D2D1 bmpEvil4R !");

    for (int i = 0; i < 16; ++i)if (!ClrMem(&bmpEvil5L[i]))LogErr(L"Error releasing D2D1 bmpEvil5L !");
    for (int i = 0; i < 16; ++i)if (!ClrMem(&bmpEvil5R[i]))LogErr(L"Error releasing D2D1 bmpEvil5R !");

    for (int i = 0; i < 24; ++i)if (!ClrMem(&bmpEvil6L[i]))LogErr(L"Error releasing D2D1 bmpEvil6L !");
    for (int i = 0; i < 24; ++i)if (!ClrMem(&bmpEvil6R[i]))LogErr(L"Error releasing D2D1 bmpEvil6R !");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"КРИТИЧНА ГРЕШКА !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    ReleaseResources();
    std::remove(tmp_file);
    exit(1);
}
void InitGame()
{
    wcscpy_s(current_player, L"PREHISTORIK");
    name_set = false;
    score = 0;
    level = 1;

    //////////////////////////////////////////


}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);


    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}




int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)ErrExit(eClass);




    ReleaseResources();
    std::remove(tmp_file);

    return (int) bMsg.wParam;
}