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
HCURSOR outCursor{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HWND bHwnd{ nullptr };
HDC PaintDC{ nullptr };
PAINTSTRUCT bPaint{};
MSG bMsg{};
BOOL bRet{ 0 };

POINT cur_pos{};

UINT bTimer{ 0 };

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
int mins = 0;
int secs = 0;

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

///////////////////////////////////////////////////////////////

std::vector<dll::Asset>vFields;
std::vector<dll::Asset>vPlatforms;
dirs assets_dir = dirs::stop;

dll::Creature Hero{ nullptr };
std::vector<dll::Creature>vEvils;

std::vector<dll::Creature>vHeroShots;
std::vector<dll::Creature>vEvilShots;

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

    mins = 0;
    secs = 0;
    bTimer = 0;

    if (!vFields.empty())
        for (int i = 0; i < vFields.size(); ++i)ClrMem(&vFields[i]);
    vFields.clear();
    for (float sx = -scr_width; sx < 2 * scr_width; sx += scr_width) vFields.push_back(dll::FieldFactory(assets::field, sx, 50.0f));
    
    if (!vPlatforms.empty())
        for (int i = 0; i < vPlatforms.size(); ++i)ClrMem(&vPlatforms[i]);
    vPlatforms.clear();

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClrMem(&vEvils[i]);
    vEvils.clear();

    if (!vEvilShots.empty())
        for (int i = 0; i < vEvilShots.size(); ++i)ClrMem(&vEvilShots[i]);
    vEvilShots.clear();

    if (!vHeroShots.empty())
        for (int i = 0; i < vHeroShots.size(); ++i)ClrMem(&vHeroShots[i]);
    vHeroShots.clear();

    if (Hero)ClrMem(&Hero);
    Hero = dll::CreatureFactory(types::hero, 100.0f, ground);
}

void GameOver()
{
    KillTimer(bHwnd, bTimer);
    PlaySound(NULL, NULL, NULL);


    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}

INT_PTR CALLBACK bDlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(bIcon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Името си ли забрави ?", L"Забраватор", MB_OK | MB_APPLMODAL | MB_ICONQUESTION);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK bWinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        if (bIns)
        {
            SetTimer(hwnd, bTimer, 1000, NULL);
            
            bBar = CreateMenu();
            bMain = CreateMenu();
            bStore = CreateMenu();

            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

            AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
            AppendMenu(bMain, MF_STRING, mTurbo, L"Следващо ниво");
            AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bMain, MF_STRING, mExit, L"Изход");

            AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
            AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
            AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");

            SetMenu(hwnd, bBar);
            InitGame();

        }
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, губиш прогреса по тази игра !\n\nНаистина ли излизаш ?", L"Изход !",
            MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(10, 10, 10)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_TIMER:
        if (pause)break;
        mins = secs / 60;
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }

                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }

                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }

                SetCursor(outCursor); 
                return true;
            }
            else
            {
                if (b1Hglt || b2Hglt || b3Hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b1Hglt = false;
                    b2Hglt = false;
                    b3Hglt = false;
                }
                SetCursor(bCursor);
            }
            
            return true;
        }
        else
        {
            if (in_client)
            {
                in_client = false;
                pause = true;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L".\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            SetCursor(LoadCursor(NULL, IDC_ARROW));
           
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартирш, губиш прогреса по тази игра !\n\nНаистина ли рестартираш ?", L"Рестарт !",
                MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;

        case mTurbo:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Наистина ли увеличаваш скоростта ?", L"Турбо !",
                MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO) == IDNO)
            {
                pause = false;
                break;
            }
            ++level;
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        }
        break;

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) <= 50)
        {
            if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
            {
                if (name_set)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                    break;
                }
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &bDlgProc) == IDOK)name_set = true;
                break;
            }
            if (LOWORD(lParam) >= b2Rect.left && LOWORD(lParam) <= b2Rect.right)
            {
                mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    sound = true;
                    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }
            }
        }

        break;

    case WM_KEYDOWN:
        if (!Hero)break;
        if (Hero->jump)
        {
            
            if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
            break;
        }
        switch(wParam)
        {
        case VK_LEFT:
            Hero->dir = dirs::left;
            Hero->state = states::move;
            break;

        case VK_RIGHT:
            Hero->dir = dirs::right;
            Hero->state = states::move;
            break;

        case VK_UP:
            Hero->jump = true;
            Hero->state = states::move;
            break;

        case VK_DOWN:
            Hero->dir = dirs::stop;
            Hero->state = states::stop;
            break;

        }
        break;

        
    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int win_x = GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2.0f);
    int win_y = 20;

    if (GetSystemMetrics(SM_CXSCREEN) < win_x + (int)(scr_width) || GetSystemMetrics(SM_CYSCREEN) < win_y + (int)(scr_height))
        ErrExit(eScreen);
    bIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
    if (!bIcon)ErrExit(eIcon);
    bCursor = LoadCursorFromFile(L".\\res\\main.ani");
    outCursor = LoadCursorFromFile(L".\\res\\out.ani");
    if (!bCursor || !outCursor)ErrExit(eCursor);

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = bWinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
    bWinClass.hIcon = bIcon;
    bWinClass.hCursor = bCursor;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"PREHISTO 2 !", WS_CAPTION | WS_SYSMENU, win_x, win_y, (int)(scr_width), (int)(scr_height),
        NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogErr(L"Error creating D2D1 Factory !");
            ErrExit(eD2D);
        }

        if (iFactory)
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
        if (hr != S_OK)
        {
            LogErr(L"Error creating D2D1 HwndRenderTarget !");
            ErrExit(eD2D);
        }

        if (Draw)
        {
            D2D1_GRADIENT_STOP gStops[2]{};

            ID2D1GradientStopCollection* gColl = nullptr;

            gStops[0].position = 0;
            gStops[0].color = D2D1::ColorF(D2D1::ColorF::Peru);
            gStops[1].position = 1.0f;
            gStops[1].color = D2D1::ColorF(D2D1::ColorF::DarkSalmon);

            hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
            if (hr != S_OK)
            {
                LogErr(L"Error creating D2D1 GradientStopCollection !");
                ErrExit(eD2D);
            }

            if (gColl)
            {
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left +
                    (b1Rect.right - b1Rect.left) / 2, 25.0f), D2D1::Point2F(0, 0), (b1Rect.right - b1Rect.left) / 2, 25.0f),
                    gColl, &b1BckgBrush);
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
                    (b2Rect.right - b2Rect.left) / 2, 25.0f), D2D1::Point2F(0, 0), (b2Rect.right - b2Rect.left) / 2, 25.0f),
                    gColl, &b2BckgBrush);
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
                    (b3Rect.right - b3Rect.left) / 2, 25.0f), D2D1::Point2F(0, 0), (b3Rect.right - b3Rect.left) / 2, 25.0f),
                    gColl, &b3BckgBrush);

                if (hr != S_OK)
                {
                    LogErr(L"Error creating D2D1 ButBckgBrushes !");
                    ErrExit(eD2D);
                }
                ClrMem(&gColl);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkKhaki), &statBckgBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Chartreuse), &txtBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumSpringGreen), &hgltBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumSlateBlue), &inactBrush);
            
            if (hr != S_OK)
            {
                LogErr(L"Error creating D2D1 TxtBrushes !");
                ErrExit(eD2D);
            }

            bmpField = Load(L".\\res\\img\\field\\field.png", Draw);
            if (!bmpField)
            {
                LogErr(L"Error loading bmpField !");
                ErrExit(eD2D);
            }
            bmpAxe = Load(L".\\res\\img\\field\\axe.png", Draw);
            if (!bmpAxe)
            {
                LogErr(L"Error loading bmpAxe !");
                ErrExit(eD2D);
            }
            bmpGold = Load(L".\\res\\img\\field\\gold.png", Draw);
            if (!bmpGold)
            {
                LogErr(L"Error loading bmpGold !");
                ErrExit(eD2D);
            }
            bmpPotion = Load(L".\\res\\img\\field\\potion.png", Draw);
            if (!bmpPotion)
            {
                LogErr(L"Error loading bmpPotion !");
                ErrExit(eD2D);
            }
            bmpRIP = Load(L".\\res\\img\\field\\rip.png", Draw);
            if (!bmpRIP)
            {
                LogErr(L"Error loading bmpRIP !");
                ErrExit(eD2D);
            }
            bmpTree = Load(L".\\res\\img\\field\\tree.png", Draw);
            if (!bmpTree)
            {
                LogErr(L"Error loading bmpTree !");
                ErrExit(eD2D);
            }
            bmpPlatform1 = Load(L".\\res\\img\\field\\platform1.png", Draw);
            if (!bmpPlatform1)
            {
                LogErr(L"Error loading bmpPlatform1 !");
                ErrExit(eD2D);
            }
            bmpPlatform2 = Load(L".\\res\\img\\field\\platform2.png", Draw);
            if (!bmpPlatform2)
            {
                LogErr(L"Error loading bmpPlatform2 !");
                ErrExit(eD2D);
            }
            bmpPlatform3 = Load(L".\\res\\img\\field\\platform3.png", Draw);
            if (!bmpPlatform3)
            {
                LogErr(L"Error loading bmpPlatform3 !");
                ErrExit(eD2D);
            }

            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\field\\fireball\\" };
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpFireball[i] = Load(name, Draw);
                if (!bmpFireball[i])
                {
                    LogErr(L"Error loading bmpFireball !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 32; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\field\\intro\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpIntro[i] = Load(name, Draw);
                if (!bmpIntro[i])
                {
                    LogErr(L"Error loading bmpIntro !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 4; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\hero\\l\\" };
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpHeroL[i] = Load(name, Draw);
                if (!bmpHeroL[i])
                {
                    LogErr(L"Error loading bmpHeroL !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 4; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\hero\\r\\" };
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpHeroR[i] = Load(name, Draw);
                if (!bmpHeroR[i])
                {
                    LogErr(L"Error loading bmpHeroR !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 11; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil1\\l\\" };
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil1L[i] = Load(name, Draw);
                if (!bmpEvil1L[i])
                {
                    LogErr(L"Error loading bmpEvil1L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 11; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil1\\r\\" };
                wchar_t add[5] = L"\0";
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil1R[i] = Load(name, Draw);
                if (!bmpEvil1R[i])
                {
                    LogErr(L"Error loading bmpEvil1R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 28; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil2\\l\\0" };
                wchar_t add[5] = L"\0";
                
                if (i < 10)wcscat_s(name, L"0");
                
                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil2L[i] = Load(name, Draw);
                if (!bmpEvil2L[i])
                {
                    LogErr(L"Error loading bmpEvil2L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 28; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil2\\r\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil2R[i] = Load(name, Draw);
                if (!bmpEvil2R[i])
                {
                    LogErr(L"Error loading bmpEvil2R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 20; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil3\\l\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil3L[i] = Load(name, Draw);
                if (!bmpEvil3L[i])
                {
                    LogErr(L"Error loading bmpEvil3L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 20; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil3\\r\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil3R[i] = Load(name, Draw);
                if (!bmpEvil3R[i])
                {
                    LogErr(L"Error loading bmpEvil3R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 5; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil4\\l\\0" };
                wchar_t add[5] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil4L[i] = Load(name, Draw);
                if (!bmpEvil4L[i])
                {
                    LogErr(L"Error loading bmpEvil4L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 5; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil4\\r\\0" };
                wchar_t add[5] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil4R[i] = Load(name, Draw);
                if (!bmpEvil4R[i])
                {
                    LogErr(L"Error loading bmpEvil4R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil5\\l\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil5L[i] = Load(name, Draw);
                if (!bmpEvil5L[i])
                {
                    LogErr(L"Error loading bmpEvil5L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil5\\r\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil5R[i] = Load(name, Draw);
                if (!bmpEvil5R[i])
                {
                    LogErr(L"Error loading bmpEvil5R !");
                    ErrExit(eD2D);
                }
            }

            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil6\\l\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil6L[i] = Load(name, Draw);
                if (!bmpEvil6L[i])
                {
                    LogErr(L"Error loading bmpEvil6L !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[150]{ L".\\res\\img\\evil6\\r\\0" };
                wchar_t add[5] = L"\0";

                if (i < 10)wcscat_s(name, L"0");

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil6R[i] = Load(name, Draw);
                if (!bmpEvil6R[i])
                {
                    LogErr(L"Error loading bmpEvil6R !");
                    ErrExit(eD2D);
                }
            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogErr(L"Error creating D2D1 WriteFactory !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, 14, L"", &nrmText);
            hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, 32, L"", &midText);
            hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, 64, L"", &bigText);
            if (hr != S_OK)
            {
                LogErr(L"Error creating D2D1 WriteTextFormats !");
                ErrExit(eD2D);
            }
        }
    }

    int intro_frame = 0;
    int frame_delay = 4;
    bool up_ok = false;
    bool down_ok = false;

    wchar_t up_txt[13]{ L"PREHISTO 2 !" };
    int up_let_count = 0;
    float up_txt_y_pos{ -20.0f };

    wchar_t down_txt[14]{ L"dev. Daniel !" };
    int down_let_count = 0;
    float down_txt_y_pos{ scr_height };

    if (Draw && bigText && txtBrush)
    {
        mciSendString(L"play .\\res\\snd\\entry.wav", NULL, NULL, NULL);

        while (!up_ok || !down_ok)
        {
            Draw->BeginDraw();

            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            --frame_delay;
            if (frame_delay <= 0)
                {
                    frame_delay = 4;
                    ++intro_frame;
                    if (intro_frame >= 32)intro_frame = 0;
                }
            
            Draw->DrawTextW(up_txt, up_let_count, bigText, D2D1::RectF(150.0f, up_txt_y_pos,
                scr_width, up_txt_y_pos + 50.0f), txtBrush);
            if (!up_ok)
            {
                up_txt_y_pos += 2.5f;
                if (up_let_count + 1 < 13)++up_let_count;
                if (up_txt_y_pos >= scr_height / 2 - 100.0f && !up_ok)
                {
                    up_ok = true;
                    mciSendString(L"play .\\res\\snd\\boom.wav", NULL, NULL, NULL);
                    mciSendString(L"play .\\res\\snd\\entry.wav", NULL, NULL, NULL);
                }
            }
            if (up_ok)
            {
                Draw->DrawTextW(down_txt, down_let_count, bigText, D2D1::RectF(300.0f, down_txt_y_pos, scr_width, down_txt_y_pos
                    + 50.0f), txtBrush);
                down_txt_y_pos -= 2.5f;
                if (down_let_count + 1 < 14)++down_let_count;
                if (down_txt_y_pos <= scr_height / 2 + 100.0f && !down_ok)
                {
                    down_ok = true;
                    mciSendString(L"play .\\res\\snd\\boom.wav", NULL, NULL, NULL);
                }
            }
            Draw->EndDraw();

            Sleep(10);
        }

        PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_SYNC);
    }

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)ErrExit(eClass);


    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);

            TranslateMessage(&bMsg);
            DispatchMessageW(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;

            static int intro_frame = 0;
            static int frame_delay = 4;

            if (Draw && bigText && txtBrush)
            {
                Draw->BeginDraw();
                Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
                --frame_delay;
                if (frame_delay <= 0)
                    {
                        frame_delay = 4;
                        ++intro_frame;
                        if (intro_frame >= 32)intro_frame = 0;
                    }
                Draw->DrawTextW(L"ПАУЗА", 6, bigText, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 50.0f,
                    scr_width, scr_height), txtBrush);
                Draw->EndDraw();
                continue;
            }
        }

        ///////////////////////////////////////////////////////////

        if (Hero)
        {
            switch (Hero->dir)
            {
            case dirs::right:
                assets_dir = dirs::left;
                break;

            case dirs::up_right:
                assets_dir = dirs::left;
                break;

            case dirs::down_right:
                assets_dir = dirs::left;
                break;

            case dirs::left:
                assets_dir = dirs::right;
                break;

            case dirs::up_left:
                assets_dir = dirs::right;
                break;

            case dirs::down_left:
                assets_dir = dirs::right;
                break;

            case dirs::stop:
                assets_dir = dirs::stop;
                break;
            }

            if (Hero->state == states::move || Hero->jump)Hero->Move((float)(level));
        }

        if (!vFields.empty() && assets_dir != dirs::stop)
        {
            for (std::vector<dll::Asset>::iterator field = vFields.begin(); field < vFields.end(); ++field)
            {
                if (!(*field)->Move((float)(level), assets_dir))
                {
                    (*field)->Release();
                    vFields.erase(field);
                    break;
                }
            }
            if (vFields.size() < 3)
            {
                if (assets_dir == dirs::right)vFields.insert(vFields.begin(), dll::FieldFactory(assets::field,
                    vFields.front()->start.x - scr_width, 50.0f));
                else vFields.push_back(dll::FieldFactory(assets::field, vFields.back()->end.x, 50.0f));
            }
        }

        if (vPlatforms.size() < 3 && RandGen(0, 500) == 66)
            vPlatforms.push_back(dll::FieldFactory(static_cast<assets>(RandGen(0, 2)), scr_width + (float)(RandGen(160, 250)),
                ground - (float)(RandGen(0, 50))));
        
        if (!vPlatforms.empty() && assets_dir != dirs::stop)
        {
            for (std::vector<dll::Asset>::iterator plat = vPlatforms.begin(); plat < vPlatforms.end(); ++plat)
            {
                if (!(*plat)->Move((float)(level), assets_dir))
                {
                    (*plat)->Release();
                    vPlatforms.erase(plat);
                    break;
                }
            }
        }

        if (Hero)
        {
            if (!Hero->jump)
            {
                if (Hero->state != states::stop)Hero->state = states::fall;

                if(!vPlatforms.empty())
                {
                    for (std::vector<dll::Asset>::iterator pl = vPlatforms.begin(); pl < vPlatforms.end(); ++pl)
                    {
                        if ((abs(Hero->center.x - (*pl)->center.x) <= Hero->x_radius + (*pl)->x_radius)
                            && (abs(Hero->center.y - (*pl)->center.y) <= Hero->y_radius + (*pl)->y_radius))
                        {
                            Hero->state = states::move;
                            break;
                        }
                    }
                }

                if (Hero->state == states::fall)
                {
                    Hero->start.y += (float)(level);
                    Hero->SetEdges();
                    if (Hero->start.y >= ground)
                    {
                        Hero->state = states::move;
                        if (Hero->start.y > ground)
                        {
                            Hero->start.y = ground;
                            Hero->SetEdges();
                        }
                    }
                }
            }
            else if (!vPlatforms.empty())
            {
                for (std::vector<dll::Asset>::iterator pl = vPlatforms.begin(); pl < vPlatforms.end(); ++pl)
                {
                    if ((abs(Hero->center.x - (*pl)->center.x) <= Hero->x_radius + (*pl)->x_radius)
                        && (abs(Hero->center.y - (*pl)->center.y) <= Hero->y_radius + (*pl)->y_radius))
                    {
                        Hero->state = states::move;
                        if (Hero->center.y >= (*pl)->start.y)
                        {
                            Hero->start.y = (*pl)->start.y - Hero->GetHeight();
                            Hero->SetEdges();
                            Hero->ResetJump();
                        }
                        break;
                    }
               
                }
            }
        }
        
        /////////////////////////////////////////////////////////////

        if (vEvils.size() < 4 + level && RandGen(0, 200) == 66)
        {
            vEvils.push_back(dll::CreatureFactory(static_cast<types>(RandGen(0, 6)), scr_width + (float)(RandGen(50, 100)),
                sky + (float)(RandGen(150, 300))));
            vEvils.back()->dir = dirs::left;
        }

        if (!vEvils.empty() && Hero)
        {
            for (std::vector<dll::Creature>::iterator ev = vEvils.begin(); ev < vEvils.end(); ++ev)
            {
                dll::BAG<dll::FIELD> PlatBag(vPlatforms.size());

                if (!vPlatforms.empty())
                    for (int i = 0; i < vPlatforms.size(); ++i)PlatBag.push_back(*vPlatforms[i]);
                
                if ((*ev)->type == types::evil2 || (*ev)->type == types::evil3)
                {
                    if ((*ev)->dir == dirs::left && (*ev)->start.x <= -scr_width)(*ev)->dir = dirs::right;
                    if ((*ev)->dir == dirs::right && (*ev)->end.x >= 2 * scr_width)(*ev)->dir = dirs::left;

                    if ((*ev)->Dispatcher(Hero->center, PlatBag) == states::attack_finished)
                    {
                        if (abs(Hero->center.x - (*ev)->center.x) <= Hero->x_radius + (*ev)->x_radius
                            && abs(Hero->center.y - (*ev)->center.y) <= Hero->y_radius + (*ev)->y_radius)
                            Hero->lifes -= 40;
                        else vEvilShots.push_back(dll::CreatureFactory(types::fire, (*ev)->center.x, (*ev)->center.y,
                            Hero->center.x, Hero->center.y));
                        
                        (*ev)->state = states::move;
                    }
                    else (*ev)->Move((float)(level));
                }
                else
                {
                    switch ((*ev)->Dispatcher(Hero->center, PlatBag))
                    {
                    case states::move:
                        (*ev)->Move((float)(level));
                        break;

                    case states::fall:
                        (*ev)->Move((float)(level));
                        break;

                    case states::stop:
                        (*ev)->dir = dirs::stop;
                        (*ev)->Move((float)(level));
                        break;

                    case states::attack_finished:
                        if (abs(Hero->center.x - (*ev)->center.x) <= Hero->x_radius + (*ev)->x_radius
                            && abs(Hero->center.y - (*ev)->center.y) <= Hero->y_radius + (*ev)->y_radius)
                            Hero->lifes -= 20;
                        (*ev)->state = states::move;
                        break;

                    }
                }
            }
        }
      
        if (!vEvilShots.empty())
        {
            for (std::vector<dll::Creature>::iterator shot = vEvilShots.begin(); shot < vEvilShots.end(); ++shot)
            {
                if (!(*shot)->Move((float)(level), 0, 0))
                {
                    (*shot)->Release();
                    vEvilShots.erase(shot);
                    break;
                }
            }
        }

        // DRAW THINGS **********************************************

        Draw->BeginDraw();

        if (b1BckgBrush && b2BckgBrush && b3BckgBrush && statBckgBrush && txtBrush && hgltBrush && inactBrush && nrmText)
        {
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, (b1Rect.right - b1Rect.left) / 2, 25.0f), b1BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, (b2Rect.right - b2Rect.left) / 2, 25.0f), b2BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, (b3Rect.right - b3Rect.left) / 2, 25.0f), b3BckgBrush);

            if (name_set)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1TxtRect, inactBrush);
            else
            {
                if (!b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1TxtRect, txtBrush);
                else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1TxtRect, hgltBrush);
            }
            if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2TxtRect, txtBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2TxtRect, hgltBrush);
            if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3TxtRect, txtBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3TxtRect, hgltBrush);
        }

        if (!vFields.empty())
        {
            for (int i = 0; i < vFields.size(); ++i)
            {
                if ((vFields[i]->start.x >= 0 && vFields[i]->start.x <= scr_width) ||
                    (vFields[i]->end.x >= 0 && vFields[i]->end.x <= scr_width))
                    Draw->DrawBitmap(bmpField, D2D1::RectF(vFields[i]->start.x, vFields[i]->start.y,
                        vFields[i]->end.x, vFields[i]->end.y));
            }
        }

        //////////////////////////////////

        if (!vPlatforms.empty())
        {
            for (std::vector<dll::Asset>::iterator plat = vPlatforms.begin(); plat < vPlatforms.end(); ++plat)
            {
                switch ((*plat)->type)
                {
                case assets::platform1:
                    Draw->DrawBitmap(bmpPlatform1, D2D1::RectF((*plat)->start.x, (*plat)->start.y, (*plat)->end.x, (*plat)->end.y));
                    break;

                case assets::platform2:
                    Draw->DrawBitmap(bmpPlatform2, D2D1::RectF((*plat)->start.x, (*plat)->start.y, (*plat)->end.x, (*plat)->end.y));
                    break;

                case assets::platform3:
                    Draw->DrawBitmap(bmpPlatform3, D2D1::RectF((*plat)->start.x, (*plat)->start.y, (*plat)->end.x, (*plat)->end.y));
                    break;

                }
            }
        }
        
        if (Hero)
        {
            if (Hero->dir == dirs::right || Hero->dir == dirs::up_right || Hero->dir == dirs::down_right
                || Hero->dir == dirs::stop)Draw->DrawBitmap(bmpHeroR[Hero->GetFrame()], D2D1::RectF(Hero->start.x,
                    Hero->start.y, Hero->end.x, Hero->end.y));
            else Draw->DrawBitmap(bmpHeroL[Hero->GetFrame()], D2D1::RectF(Hero->start.x,
                Hero->start.y, Hero->end.x, Hero->end.y));

            if (hgltBrush)
                Draw->DrawLine(D2D1::Point2F(Hero->start.x, Hero->end.y + 5.0f),
                    D2D1::Point2F(Hero->start.x + (float)(Hero->lifes / 2), Hero->end.y + 5.0f), hgltBrush, 5.0f);
        }

        if (!vEvils.empty())
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                switch ((*evil)->type)
                {
                case types::evil1:
                    if ((*evil)->dir == dirs::left || (*evil)->dir == dirs::up_left || (*evil)->dir == dirs::down_left)
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil1L[aframe], Resizer(bmpEvil1L[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    else
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil1R[aframe], Resizer(bmpEvil1R[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    break;

                case types::evil2:
                    if ((*evil)->dir == dirs::left || (*evil)->dir == dirs::up_left || (*evil)->dir == dirs::down_left)
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil2L[aframe], Resizer(bmpEvil2L[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    else
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil2R[aframe], Resizer(bmpEvil2R[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    break;

                case types::evil3:
                    if ((*evil)->dir == dirs::left || (*evil)->dir == dirs::up_left || (*evil)->dir == dirs::down_left)
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil3L[aframe], Resizer(bmpEvil3L[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    else
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil3R[aframe], Resizer(bmpEvil3R[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    break;

                case types::evil4:
                    if ((*evil)->dir == dirs::left || (*evil)->dir == dirs::up_left || (*evil)->dir == dirs::down_left)
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil4L[aframe], Resizer(bmpEvil4L[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    else
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil4R[aframe], Resizer(bmpEvil4R[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    break;

                case types::evil5:
                    if ((*evil)->dir == dirs::left || (*evil)->dir == dirs::up_left || (*evil)->dir == dirs::down_left)
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil5L[aframe], Resizer(bmpEvil5L[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    else
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil5R[aframe], Resizer(bmpEvil5R[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    break;

                case types::evil6:
                    if ((*evil)->dir == dirs::left || (*evil)->dir == dirs::up_left || (*evil)->dir == dirs::down_left)
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil6L[aframe], Resizer(bmpEvil6L[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    else
                    {
                        int aframe = (*evil)->GetFrame();
                        Draw->DrawBitmap(bmpEvil6R[aframe], Resizer(bmpEvil6R[aframe], (*evil)->start.x, (*evil)->start.y));
                        break;
                    }
                    break;
                }
            }
        }

        if (!vEvilShots.empty())
        {
            for (int i = 0; i < vEvilShots.size(); ++i)
                Draw->DrawBitmap(bmpFireball[vEvilShots[i]->GetFrame()], D2D1::RectF(vEvilShots[i]->start.x,
                    vEvilShots[i]->start.y, vEvilShots[i]->end.x, vEvilShots[i]->end.y));
        }

        //////////////////////////////////////////////////////////////

        Draw->EndDraw();
    }

    ReleaseResources();
    std::remove(tmp_file);

    return (int) bMsg.wParam;
}