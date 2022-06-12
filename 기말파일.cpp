#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <stdlib.h>
#include <time.h>
#include <atlImage.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Brain MeltDown";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{//실행된 프로세스의 시작 주소  //
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;
    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = MAKEINTRESOURCE(NULL);
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&WndClass);
    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1200, 800, NULL,
        NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return Message.wParam;
}

CImage background[2]{};
CImage stage1background[3]{};
CImage start[7]{};
CImage finish[2]{};
CImage stage1block[20]{}; //블록 및 발판

CImage stage2background[3]{};
CImage stage2block[20]{};
POINT xy{};
RECT wr{};
CImage yellowsprite[10]{};
CImage purplesprite[10]{};
CImage monstersprite[4]{};

struct character
{
    int xPos;
    int xsize = 150;
    int ysize = 150;
    int movedir = 0;
    RECT rect;
    bool jump = false;
    bool down = false;
    int xcount;
    int jumpcount;

    int plusjumpcount;
    bool plusjump;

    int downcount;
    int savey;

    bool posstate;
    bool checkstate;
    bool pystate;
    bool state = true;
    bool xmove;
};

struct monster
{
    RECT rect{};
    int xPos;
    int xsize;
    int ysize;
    int movingcount;
    bool dir = true;
};

character yellow{};
character purple{};

int locatex = 0; // 임시 노라 또는 보라가 될 x값

int ypcheck(character yellow, character purple);
int blockcheck(character cha1, RECT cblock[50]);
int pyblockcheck(character cha1, RECT cblock[20]);
bool obstaclecheck(character cha1, RECT obstacle[20]);
bool monstercheck(character cha1, monster m[2]);

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC, memDC;
    static HBITMAP hbit;

    RECT Rblock[20] = { {0,300,600,900}, {800,300,1400,1000},{640,600,760,620}, {1400,500,1900,1000}, {1900,200,2500,800}
    ,{2500,420,3700,770}, {3880,190,4400,290}, {4400,350,5000,850} ,{5300,10,5650,420}, {5900,300,6500,900} };//0왼쪽 큰덩어리 1오른쪽 작은 덩어리 2 중간 돌

    static RECT R2block[20] = { {100,500,600,1500},{600,500,1100,1500}, {1180, 425, 1305, 475},{1395,375,1520,425},{2150,-350,2750,750 }
    ,{2750,-350,3350,750 },{3380,250,3580,310}, {3600,0,4200,1200},{4200,0,4800,1200},{4900,600,5050,1450},{5100,500,5250,1350},{6130,-450,6730,750},{6730,-450,7330,750} };

    RECT pblock[20] = { { 1620, 330,1680,360} , {1730,270,1790,300},{1840,210,1900,240},{1950,270,2010,300}, {2060,220,2120,250},{5350, 450,5410,480},
        {5480, 410,5540,440}, {5610,370,5670,400}, {5740,330,5800,360},{5870,290,5930,320},{6020,250,6080,280} };
    RECT yblock[20] = { {1620,420,1680,450},{1730,470,1790,500},{1840,410,1900,440}, {1950,360,2010,390},{2060,310,2120,340 }, {5350, 450,5410,480},
        {5480, 410,5540,440}, {5610,370,5670,400}, {5740,330,5800,360},{5870,290,5930,320},{6020,250,6080,280} };

    RECT Rstart[5]{ {505,212,671,240},{505,258,671,284},{100,100,600,310}, {230,130,300,230} , {500,145,570,245}, };//0이 start, 1이 exit 2는 key 3 purplestart 4yellowstart
    static RECT obstacle1[20] = {2050,620, 2120,700};
    RECT obstacle2[20] = { {2440,210,2460,250},{2740,210,2760,250},{3040,210,3060,250} };//장애물 충돌체크 할 위치
    static RECT Rsmallblock[20] = { {3700,400,3775,440},{3825,300,3900,340},{5050,500, 5125,540}, {5175,500,5250,540} };

    RECT rect{ 0,0,0,0 };
    RECT continuerect = { 510,415,695,450 };

    RECT roundfinishcheck[2] = { {6300,0,6600,800}, {6900,0,7200,800 }};

    static monster round1[2]{};
    static monster round2[2]{};
    static RECT c1block[50] = { { 30,500,540,550}, {610,590,705,650}, {775,610,960,680}, {910,545,1080,680}, {1080,620,1310,650} ,{1345,570,1477,600}, {1477,530,1850,580}, {1850,665,2025,700},
        {2025,590,2120,630}, {2075,520,2140,540}, {2130,440,2320,490}, {2320,550,2400,570}, {2480,550,2970,570}, {2970,530,3060,550}, {3060,510,3150,530}, {3150,495,3220,510}, {3220,475,3600,500},
        {3650,400,3730,420},{3755,300,3845,320},{3845,190,4300,220},{4300,370,4615,430},{4645,530,4713,540},{4700,490,4930,500},{4990,490,5040,500},{5115,490,5170,500}, {5900,490,6480,500} };
    static RECT c2block[50] = { {100,500,1050,550}, {1100,425,1250,475}, {1300,375,1440,425},{2070,250 ,3270, 300},{3300,250,3500,300},{3520,650,4720,700},
        {4820,600,4970,650},{5020,500,5170,550}, {6050, 200,7250,250} };
    static RECT copyc1[50]{};
    static RECT pcblock[20] = { { 1540, 330,1600,360} , {1650,270,1710,300},{1760,210,1820,240},{1870,270,1930,300}, {1970,220,2040,250},{5270, 450,5340,480},
        {5400, 410,5480,440}, {5530,370,5590,400}, {5680,330,5720,360},{5790,290,5850,320},{5940,250,6000,280} };
    static RECT ycblock[20] = { {1540,420,1600,450},{1650,470,1710,500},{1760,410,1830,440}, {1870,360,1940,390},{1970,310,2040,340 }, {5270, 450,5340,480},
        {5400, 410,5480,440}, {5530,370,5590,400}, {5680,330,5720,360},{5790,290,5850,320},{5940,250,6000,280} };

    static RECT changeblock = {6150,170,6250,200};
    static RECT slideblock{};
    static RECT checkpoint[20] = { {450,380,550,500},{1750,400,1850,520},{4200,70,4300,190},{950,380,1050,500},{2300,130,2400,250},{5080,390,5180,510} };
    static int hour, min, sec;
    static bool Bstart = true, Bstage1 = false, Bstage2 = false, Bkeytimer = false, Bdie = false, B1roundfinish, B1yellow, B1purple, B2roundfinish, B2yellow, B2purple, BYorP, BYorPmovecheck, Bsame1, Bsame2;
    static bool B6, Bfinal;
    static int count = 0, count6, deathcount, finishcount, checkcount,smallcount;
    static BOOL KeyBuffer[256];
    static TCHAR Tdeathcount[50]{}, Ttime[50]{};
    switch (uMsg) {
    case WM_CREATE:
        GetClientRect(hWnd, &wr);

        background[0].Load(TEXT("기말과제/background1.bmp"));
        start[0].Load(TEXT("기말과제/start.png"));
        start[2].Load(TEXT("기말과제/키.png"));
        start[3].Load(TEXT("기말과제/purplestart.png"));
        start[4].Load(TEXT("기말과제/yellowstart.png"));
        start[5].Load(TEXT("기말과제/death.png"));
        start[6].Load(TEXT("기말과제/체크포인트.png"));

        finish[0].Load(TEXT("기말과제/stage1clear.png"));
        finish[1].Load(TEXT("기말과제/stage2clear.png"));

        stage1background[0].Load(TEXT("기말과제/배경.png"));
        stage1background[1].Load(TEXT("기말과제/구름.png"));
        stage1background[2].Load(TEXT("기말과제/구름2.png"));
        stage1block[0].Load(TEXT("기말과제/block1.png"));
        stage1block[1].Load(TEXT("기말과제/block2.png"));
        stage1block[2].Load(TEXT("기말과제/block3.png"));
        stage1block[3].Load(TEXT("기말과제/block4.png"));
        stage1block[4].Load(TEXT("기말과제/block5.png"));
        stage1block[5].Load(TEXT("기말과제/block6.png"));
        stage1block[6].Load(TEXT("기말과제/발판.png"));
        stage1block[7].Load(TEXT("기말과제/block7.png"));
        stage1block[8].Load(TEXT("기말과제/block8.png"));
        stage1block[9].Load(TEXT("기말과제/동시발판.png"));
        stage1block[10].Load(TEXT("기말과제/block9.png"));

        stage2background[0].Load(TEXT("기말과제/배경2.png"));
        stage2block[0].Load(TEXT("기말과제/2-block1.png"));
        stage2block[1].Load(TEXT("기말과제/2-block2.png"));
        stage2block[2].Load(TEXT("기말과제/보라발판.png"));
        stage2block[3].Load(TEXT("기말과제/노라발판.png"));
        stage2block[4].Load(TEXT("기말과제/2-block3.png"));
        stage2block[5].Load(TEXT("기말과제/가시.png"));
        stage2block[6].Load(TEXT("기말과제/2-block4.png"));
        stage2block[7].Load(TEXT("기말과제/2-block5.png"));

        yellowsprite[0].Load(TEXT("기말과제/ys redwalk.png"));
        yellowsprite[1].Load(TEXT("기말과제/ys redwalkreverse.png"));
        yellowsprite[2].Load(TEXT("기말과제/ys redjump.png"));
        yellowsprite[3].Load(TEXT("기말과제/ys redjumpreverse.png"));
        yellowsprite[4].Load(TEXT("기말과제/ys reddown.png"));
        yellowsprite[5].Load(TEXT("기말과제/ys reddownreverse.png"));

        purplesprite[0].Load(TEXT("기말과제/ys purplewalk.png"));
        purplesprite[1].Load(TEXT("기말과제/ys purplewalkreverse.png"));
        purplesprite[2].Load(TEXT("기말과제/ys puprplejump.png"));
        purplesprite[3].Load(TEXT("기말과제/ys puprplejumpreverse.png"));
        purplesprite[4].Load(TEXT("기말과제/ys purpledown.png"));
        purplesprite[5].Load(TEXT("기말과제/ys purpledownreverse.png"));

        monstersprite[0].Load(TEXT("기말과제/stage1monster.png"));
        monstersprite[1].Load(TEXT("기말과제/stage1monsterreverse.png"));
        monstersprite[2].Load(TEXT("기말과제/stage2monster.png"));
        monstersprite[3].Load(TEXT("기말과제/stage2monsterreverse.png"));

        yellow.xPos = 600;
        yellow.rect = { 225,380,290,490 };

        purple.xPos = 1200;
        purple.rect = { 80,360,140,480 };

        round1[0].rect = { 2500,480,2600,560 };
        round1[1].rect = { 3300,390,3400,470 };
        round2[0].rect = { 3600,570,3700,650 };
        round2[1].rect = { 4200,570,4300,650 };
        break;
    case WM_LBUTTONDOWN:
        hDC = GetDC(hWnd);
        xy.x = LOWORD(lParam);
        xy.y = HIWORD(lParam);
        if (Bstart)
        {
            if (PtInRect(&Rstart[0], xy))
            {
                Bstart = false; Bstage1 = true; Bstage2 = false; Bkeytimer = true;
                SetTimer(hWnd, 0, 100, NULL);
                SetTimer(hWnd, 1, 0, NULL);
                SetTimer(hWnd, 2, 1000, NULL);
                SetTimer(hWnd, 3, 100, NULL);
            }
            if (PtInRect(&Rstart[1], xy))
            {
                if (MessageBox(hWnd, L"Do you want to reall exit game?", L"BrainMeltDown", MB_YESNO) == IDYES)
                    PostQuitMessage(0);
            }
        }
        if (Bdie)
        {
            if (PtInRect(&continuerect, xy))
            {
                if (checkcount == 0)
                    locatex = 220;
                if (checkcount == 1)
                    locatex = 1500;
                if (checkcount == 2)
                    locatex = 4000;
                if (checkcount == 3)
                    locatex = 800;
                if (checkcount == 4)
                    locatex = 2050;
                if (checkcount == 5)
                    locatex = 4800;

                yellow.rect.left = 120;
                yellow.rect.top = 0;
                yellow.rect.right = yellow.rect.left + 60;
                yellow.rect.bottom = yellow.rect.top + 110;

                purple.rect.left = 60;
                purple.rect.top = 0;
                purple.rect.right = purple.rect.left + 60;
                purple.rect.bottom = purple.rect.top + 130;
                yellow.state = true;
                purple.state = true;
                Bdie = false;
                BYorPmovecheck = false;
            }
        }
        ReleaseDC(hWnd, hDC);
        InvalidateRgn(hWnd, NULL, FALSE);
        break;
    case WM_KEYDOWN:
        hDC = GetDC(hWnd);
        KeyBuffer[wParam] = true;
        //switch (wParam)
        //{
        //case VK_RETURN:
        //    break;
        //}
        ReleaseDC(hWnd, hDC);
        InvalidateRgn(hWnd, NULL, FALSE);
        break;
    case WM_KEYUP:
        KeyBuffer[wParam] = false;
        yellow.xcount = 0;
        purple.xcount = 0;
        if (yellow.movedir == 0)
            yellow.xPos = 600;
        else if (yellow.movedir == 1)
            yellow.xPos = 0;
        if (purple.movedir == 0)
            purple.xPos = 1200;
        else if (purple.movedir == 1)
            purple.xPos = 0;
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_TIMER:
        hDC = GetDC(hWnd);
        switch (wParam)
        {
        case 0:
            ++count;
            if (count == 60)
            {
                KillTimer(hWnd, 0);
                Bkeytimer = false;
            }
            break;
        case 1:
            if (Bstage1)
            {
                for (int i = 0; i < 50; ++i)
                {
                    copyc1[i].left = c1block[i].left - locatex+40;
                    copyc1[i].right = c1block[i].right - locatex + 30;
                    copyc1[i].top = c1block[i].top+10;
                    copyc1[i].bottom = c1block[i].bottom+50;
                }
                slideblock.left = Rblock[8].left - locatex;
                slideblock.right = Rblock[8].right - locatex;
                slideblock.top = Rblock[8].top;
                slideblock.bottom = Rblock[8].bottom;
            }
            if (Bstage2 && locatex>=2870 && locatex <=2950)
            {
                if (yellow.jump == false)
                {
                    if (blockcheck(yellow, c2block) == 4)
                        yellow.checkstate = true;
                }
                if (purple.jump == false)
                {
                    if (blockcheck(purple, c2block) == 4)
                        purple.checkstate = true;
                }
            }
            if (KeyBuffer['W'])
            {
                if (purple.jump == false)
                {
                    if (purple.movedir == 1)
                    {
                        purple.movedir = 2;
                        purple.xPos = 0;
                    }
                    if (purple.movedir == 0)
                    {
                        purple.movedir = 3;
                        purple.xPos = 2100;
                    }
                    purple.savey = purple.rect.top;
                    purple.jump = true;
                }
            }
            if (KeyBuffer['A'])
            {
                if (purple.jump == false)
                {
                    purple.movedir = 1;
                    ++purple.xcount;
                    if (purple.xcount % 6 == 0)
                        purple.xPos += 300;
                    if (purple.xPos >= 1200)
                        purple.xPos = 300;
                }
                if (purple.rect.right < 100)
                {
                    locatex -= 2;
                    OffsetRect(&yellow.rect, 2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&purple.rect, 2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &purple.rect))
                            {
                                if (rect.left == purple.rect.left)
                                {
                                    OffsetRect(&purple.rect, 2, 0);
                                    purple.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    OffsetRect(&purple.rect, -2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&purple.rect, 2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &purple.rect))
                            {
                                if (rect.left == purple.rect.left)
                                {
                                    OffsetRect(&purple.rect, 2, 0);
                                    purple.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (KeyBuffer['S'])
            {
                if (purple.jump == false)
                {
                    if (purple.movedir == 1)//left
                    {
                        purple.movedir = 4;
                        purple.xPos = 0;
                    }
                    if (purple.movedir == 0)//right
                    {
                        purple.movedir = 5;
                        purple.xPos = 1500;
                    }
                }
                purple.down = true;
            }
            if (KeyBuffer['D'])
            {
                if (purple.jump == false)
                {
                    purple.movedir = 0;
                    ++purple.xcount;
                    if (purple.xcount % 6 == 0)
                        purple.xPos += 300;
                    if (purple.xPos >= 1200)
                        purple.xPos = 0;
                }
                if (purple.rect.right >= 500)
                {
                    locatex += 2;
                    OffsetRect(&yellow.rect, -2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&purple.rect, -2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &purple.rect))
                            {
                                if (rect.right == purple.rect.right)
                                {
                                    OffsetRect(&purple.rect, -2, 0);
                                    purple.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    OffsetRect(&purple.rect, 2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&purple.rect, -2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &purple.rect))
                            {
                                if (rect.right == purple.rect.right)
                                {
                                    OffsetRect(&purple.rect, -2, 0);
                                    purple.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (KeyBuffer[VK_UP])
            {
                if (yellow.jump == false)
                {
                    if (yellow.movedir == 1)
                    {
                        yellow.movedir = 2;
                        yellow.xPos = 0;
                    }
                    if (yellow.movedir == 0)
                    {
                        yellow.movedir = 3;
                        yellow.xPos = 1200;
                    }
                    yellow.savey = yellow.rect.top;
                    yellow.jump = true;
                }
            }
            if (KeyBuffer[VK_LEFT])
            {
                if (yellow.jump == false)
                {
                    yellow.movedir = 1;
                    ++yellow.xcount;
                    if (yellow.xcount % 6 == 0)
                        yellow.xPos += 200;
                    if (yellow.xPos >= 600)
                        yellow.xPos = 200;
                }
                if (yellow.rect.right < 100)
                {
                    locatex -= 2;
                    OffsetRect(&purple.rect, 2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&yellow.rect, 2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &yellow.rect))
                            {
                                if (rect.left == yellow.rect.left)
                                {
                                    OffsetRect(&yellow.rect, 2, 0);
                                    yellow.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    OffsetRect(&yellow.rect, -2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&yellow.rect, 2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &yellow.rect))
                            {
                                if (rect.left == yellow.rect.left)
                                {
                                    OffsetRect(&yellow.rect, 2, 0);
                                    yellow.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (KeyBuffer[VK_RIGHT])
            {
                if (yellow.jump == false)
                {
                    yellow.movedir = 0;
                    ++yellow.xcount;
                    if (yellow.xcount % 6 == 0)
                        yellow.xPos += 200;
                    if (yellow.xPos >= 600)
                        yellow.xPos = 200;
                }
                if (yellow.rect.right >= 500)
                {
                    locatex += 2;
                    OffsetRect(&purple.rect, -2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&yellow.rect, -2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &yellow.rect))
                            {
                                if (rect.right == yellow.rect.right)
                                {
                                    OffsetRect(&yellow.rect, -2, 0);
                                    yellow.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    OffsetRect(&yellow.rect, 2, 0);
                    if (ypcheck(yellow, purple) == 1)
                    {
                        OffsetRect(&yellow.rect, -2, 0);
                    }
                    if (locatex <= 2100 || locatex >= 3500)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &copyc1[i], &yellow.rect))
                            {
                                if (rect.right == yellow.rect.right)
                                {
                                    OffsetRect(&yellow.rect, -2, 0);
                                    yellow.xmove = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (KeyBuffer[VK_DOWN])
            {
                if (yellow.jump == false)
                {
                    if (yellow.movedir == 1)//left
                    {
                        yellow.movedir = 4;
                        yellow.xPos = 0;
                    }
                    if (yellow.movedir == 0)//right
                    {
                        yellow.movedir = 5;
                        yellow.xPos = 800;
                    }
                }
                yellow.down = true;
            }
            if (yellow.down)
            {
                if (yellow.movedir == 4)
                {
                    ++yellow.downcount;
                    if (yellow.downcount % 7 == 0)
                    {
                        if (yellow.xPos != 800)
                            yellow.xPos += 200;
                        if (yellow.xPos == 800)
                        {
                            if (!KeyBuffer[VK_DOWN])
                            {
                                yellow.xPos = 0;
                                yellow.downcount = 0;
                                yellow.movedir = 1;
                                yellow.down = false;
                            }
                        }
                    }
                }
                if (yellow.movedir == 5)
                {
                    ++yellow.downcount;
                    if (yellow.downcount % 7 == 0)
                    {
                        if (yellow.xPos != 0)
                            yellow.xPos -= 200;
                        if (yellow.xPos == 0)
                        {
                            if (!KeyBuffer[VK_DOWN])
                            {
                                yellow.xPos = 600;
                                yellow.downcount = 0;
                                yellow.movedir = 0;
                                yellow.down = false;
                            }
                        }
                    }
                }
            }
            if (yellow.jump)
            {
                if (yellow.movedir == 2)
                {
                    if (yellow.jumpcount % 7 == 0 && yellow.jumpcount <= 28)
                    {
                        yellow.xPos += 200;
                        yellow.rect.top -= 30;
                        yellow.rect.bottom -= 30;
                    }
                    if (yellow.jumpcount % 7 == 0 && yellow.jumpcount > 28)
                    {
                        yellow.xPos += 200;
                        yellow.rect.top += 30;
                        yellow.rect.bottom += 30;
                    }
                    if (yellow.xPos >= 1400)
                    {
                        yellow.jump = false;
                        yellow.plusjump = true;
                        yellow.jumpcount = 0;
                        yellow.movedir = 1;
                        yellow.xPos = 0;
                        yellow.rect.top = yellow.savey;
                        yellow.rect.bottom = yellow.rect.top + 110;
                    }
                    ++yellow.jumpcount;
                }
                else if (yellow.movedir == 3)
                {
                    ++yellow.jumpcount;
                    if (yellow.jumpcount % 7 == 0 && yellow.jumpcount <= 28)
                    {
                        yellow.xPos -= 200;
                        yellow.rect.top -= 35;
                        yellow.rect.bottom -= 35;
                    }
                    if (yellow.jumpcount % 7 == 0 && yellow.jumpcount > 28)
                    {
                        yellow.xPos -= 200;
                        yellow.rect.top += 30;
                        yellow.rect.bottom += 30;
                    }
                    if (yellow.xPos <= 0)
                    {
                        yellow.jumpcount = 0;
                        yellow.movedir = 0;
                        yellow.xPos = 600;
                        yellow.rect.top = yellow.savey;
                        yellow.rect.bottom = yellow.rect.top + 110;
                        yellow.jump = false;
                        yellow.plusjump = true;
                    }

                }
                if (ypcheck(yellow, purple) == 1)
                {
                    yellow.rect.bottom = purple.rect.top;
                    yellow.rect.top = yellow.rect.bottom - 110;
                    yellow.posstate = true;
                }
            }
            if (purple.down)
            {
                if (purple.movedir == 4)
                {
                    ++purple.downcount;
                    if (purple.downcount % 7 == 0)
                    {
                        if (purple.xPos != 1500)
                            purple.xPos += 300;
                        if (purple.xPos == 1500)
                        {
                            if (!KeyBuffer['S'])
                            {
                                purple.xPos = 0;
                                purple.downcount = 0;
                                purple.movedir = 1;
                                purple.down = false;
                            }
                        }
                    }
                }
                if (purple.movedir == 5)
                {
                    ++purple.downcount;
                    if (purple.downcount % 7 == 0)
                    {
                        if (purple.xPos != 0)
                            purple.xPos -= 300;
                        if (purple.xPos == 0)
                        {
                            if (!KeyBuffer['S'])
                            {
                                purple.xPos = 1200;
                                purple.downcount = 0;
                                purple.movedir = 0;
                                purple.down = false;
                            }
                        }
                    }
                }
            }
            if (purple.jump)
            {
                if (purple.movedir == 2)
                {
                    ++purple.jumpcount;
                    if (purple.jumpcount % 7 == 0 && purple.jumpcount <= 28)
                    {
                        purple.xPos += 300;
                        purple.rect.top -= 30;
                        purple.rect.bottom -= 30;
                    }
                    if (purple.jumpcount % 7 == 0 && purple.jumpcount > 28)
                    {
                        purple.xPos += 300;
                        purple.rect.top += 30;
                        purple.rect.bottom += 30;
                    }
                    if (purple.xPos >= 2400)
                    {
                        purple.jump = false;
                        purple.plusjump = true;
                        purple.jumpcount = 0;
                        purple.movedir = 1;
                        purple.xPos = 0;
                        purple.rect.top = purple.savey;
                        purple.rect.bottom = purple.rect.top + 130;
                    }
                }
                else if (purple.movedir == 3)
                {
                    ++purple.jumpcount;
                    if (purple.jumpcount % 7 == 0 && purple.jumpcount <= 28)
                    {
                        purple.xPos -= 300;
                        purple.rect.top -= 30;
                        purple.rect.bottom -= 30;
                    }
                    if (purple.jumpcount % 7 == 0 && purple.jumpcount > 28)
                    {
                        purple.xPos -= 300;
                        purple.rect.top += 30;
                        purple.rect.bottom += 30;
                    }
                    if (purple.xPos <= 0)
                    {
                        purple.jumpcount = 0;
                        purple.movedir = 0;
                        purple.xPos = 1200;
                        purple.rect.top = purple.savey;
                        purple.rect.bottom = purple.rect.top + 130;
                        purple.jump = false;
                        purple.plusjump = true;
                    }
                }
                if (ypcheck(yellow, purple) == 1)
                {
                    purple.rect.bottom = yellow.rect.top;
                    purple.rect.top = purple.rect.bottom - 110;
                    purple.posstate = true;
                }
            }
            if (yellow.plusjump)
            {
                ++yellow.plusjumpcount;
                if (yellow.plusjumpcount == 20)
                {
                    yellow.plusjump = false;
                    yellow.plusjumpcount = 0;
                }
            }
            if (purple.plusjump)
            {
                ++purple.plusjumpcount;
                if (purple.plusjumpcount == 20)
                {
                    purple.plusjump = false;
                    purple.plusjumpcount = 0;
                }
            }
            if (yellow.posstate)
            {
                if (yellow.rect.left > purple.rect.right || yellow.rect.right < purple.rect.left)
                {
                    yellow.rect.top = purple.rect.top + 10;
                    yellow.rect.bottom = yellow.rect.top + 110;
                    yellow.posstate = false;
                }
            }
            if (purple.posstate)
            {
                if (purple.rect.left > yellow.rect.right || purple.rect.right < yellow.rect.left)
                {
                    purple.rect.top = yellow.rect.top - 10;
                    purple.rect.bottom = purple.rect.top + 130;
                    purple.posstate = false;
                }
            }
            if (Bstage1)
            {
                for (int i = 0; i < 50; ++i)
                {
                    OffsetRect(&c1block[i], -locatex, 0);
                }
                OffsetRect(&roundfinishcheck[0], -locatex, 0);
                for (int i = 0; i < 2; ++i)
                {
                    OffsetRect(&round1[i].rect, -locatex, 0);
                }
                OffsetRect(&obstacle1[0], -locatex, 0);
                if (IntersectRect(&rect, &yellow.rect, &roundfinishcheck[0]))
                {
                    B1yellow = true;
                }
                if (IntersectRect(&rect, &purple.rect, &roundfinishcheck[0]))
                {
                    B1purple = true;
                }

                if (blockcheck(yellow, c1block) == 1)
                {
                    for (int i = 0; i < 50; ++i)
                    {
                        if (IntersectRect(&rect, &yellow.rect, &c1block[i]))
                        {
                            yellow.rect.bottom = c1block[i].top;
                            yellow.rect.top = yellow.rect.bottom - 110;
                            break;
                        }
                    }
                }
                if (blockcheck(purple, c1block) == 1)
                {
                    for (int i = 0; i < 50; ++i)
                    {
                        if (IntersectRect(&rect, &purple.rect, &c1block[i]))
                        {
                            purple.rect.bottom = c1block[i].top;
                            purple.rect.top = purple.rect.bottom - 130;
                            break;
                        }
                    }
                }
                if (monstercheck(yellow, round1) == 1 && yellow.state)
                {
                    yellow.state = false;
                    ++deathcount;
                }
                if (monstercheck(purple, round1) == 1 && purple.state)
                {
                    purple.state = false;
                    ++deathcount;
                }

             
                if (B1yellow && B1purple)
                {
                    B1roundfinish = true;   //round 끝남 체크
                }
                if (yellow.jump == false)
                {
                    if (blockcheck(yellow, c1block) == 4)
                    {
                        if (yellow.xmove == false)
                            OffsetRect(&yellow.rect, 0, 10);
                        else
                            yellow.xmove = false;
                    }
                }
                if (purple.jump == false)
                {
                    if (blockcheck(purple, c1block) == 4)
                    {
                        if (purple.xmove == false)
                            OffsetRect(&purple.rect, 0, 10);
                        else
                            purple.xmove = false;
                    }
                }
                if (IntersectRect(&rect, &obstacle1[0],&yellow.rect) && yellow.state)
                {
                    if (yellow.jump == true || yellow.plusjump == true)
                    {
                        yellow.state = false;
                        ++deathcount;
                    }
                }
                if (IntersectRect(&rect, &obstacle1[0], &purple.rect)&& purple.state)
                {
                    if (purple.jump == true || purple.plusjump == true)
                    {
                        purple.state = false;
                        ++deathcount;
                    }
                }
                if (Bfinal == false)
                {
                    if (c1block[23].top == yellow.rect.bottom&& c1block[23].left <= yellow.rect.right && c1block[23].left <= yellow.rect.left && c1block[23].right+40 >= yellow.rect.right)
                        Bsame1 = true;
                    else if (c1block[23].top == purple.rect.bottom && c1block[23].left <= purple.rect.right && c1block[23].left <= purple.rect.left && c1block[23].right+40 >= purple.rect.right)
                        Bsame1 = true;
                    else
                        Bsame1 = false;


                    if (c1block[24].top == yellow.rect.bottom && c1block[24].left <= yellow.rect.right && c1block[24].left <= yellow.rect.left && c1block[24].right+40 >= yellow.rect.right)
                        Bsame2 = true;
                    else if (c1block[24].top == purple.rect.bottom && c1block[24].left <= purple.rect.right && c1block[24].left <= purple.rect.left && c1block[24].right+40 >= purple.rect.right)
                        Bsame2 = true;
                    else
                        Bsame2 = false;
                }
                if (IntersectRect(&rect, &slideblock, &yellow.rect))
                {
                    if (yellow.down == false && yellow.state)
                    {
                        yellow.state = false;
                        ++deathcount;
                    }
                }
                if (IntersectRect(&rect, &slideblock, &purple.rect))
                {
                    if (purple.down == false && purple.state)
                    {
                        purple.state = false;
                        ++deathcount;
                    }
                }
                if (Bsame1 && Bsame2)
                {
                    ++smallcount;
                    OffsetRect(&c1block[23], 2, 0);
                    OffsetRect(&Rsmallblock[2], 2, 0);
                    OffsetRect(&Rsmallblock[3], 2, 0);
                    OffsetRect(&c1block[24], 2, 0);
                    OffsetRect(&yellow.rect, 2, 0);
                    OffsetRect(&purple.rect, 2, 0);
                    if (smallcount == 330)
                    {
                        Bfinal = true;
                        Bsame1 = false;
                        Bsame2 = false;
                    }
                }
                OffsetRect(&obstacle1[0], locatex, 0);
                for (int i = 0; i < 50; ++i)
                {
                    OffsetRect(&c1block[i], locatex, 0);
                }
                for (int i = 0; i < 2; ++i)
                {
                    OffsetRect(&round1[i].rect, locatex, 0);
                }
                OffsetRect(&roundfinishcheck[0], locatex, 0);
            }
            Bsame1 = false;
            Bsame2 = false;

            if (Bstage2)
            {
                {
                    for (int i = 0; i < 50; ++i)
                    {
                        OffsetRect(&c2block[i], -locatex, 0);
                    }
                    for (int i = 0; i < 20; ++i)
                    {
                        OffsetRect(&pcblock[i], -locatex, 0);
                    }
                    for (int i = 0; i < 20; ++i)
                    {
                        OffsetRect(&ycblock[i], -locatex, 0);
                    }
                    for (int i = 0; i < 20; ++i)
                    {
                        OffsetRect(&obstacle2[i], -locatex, 0);
                    }
                    for (int i = 0; i < 2; ++i)
                    {
                        OffsetRect(&round2[i].rect, -locatex, 0);
                    }
                    OffsetRect(&roundfinishcheck[1], -locatex, 0);
                    OffsetRect(&changeblock, -locatex, 0);
                }
                ++count6;
                if (count6 % 150 == 0)
                    B6 = !B6;
                if (count6 % 2 == 0)
                {
                    if (B6 == false)
                    {
                        
                        OffsetRect(&R2block[6], 0, 5);
                        OffsetRect(&c2block[4], 0, 5);
                    }
                    else
                    {
                        OffsetRect(&R2block[6], 0, -5);
                        OffsetRect(&c2block[4], 0, -5);
                    }
                }
                {
                    if (blockcheck(yellow, c2block) == 1)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &yellow.rect, &c2block[i]))
                            {
                                yellow.rect.bottom = c2block[i].top;
                                yellow.rect.top = yellow.rect.bottom - 110;
                                break;
                            }
                        }
                    }
                    if (blockcheck(purple, c2block) == 1)
                    {
                        for (int i = 0; i < 50; ++i)
                        {
                            if (IntersectRect(&rect, &purple.rect, &c2block[i]))
                            {
                                purple.rect.bottom = c2block[i].top;
                                purple.rect.top = purple.rect.bottom - 130;
                                break;
                            }
                        }
                    }
                    if (locatex < 3000 || (locatex > 3000 && BYorP == false))
                    {
                        if (pyblockcheck(yellow, ycblock) == 1)
                        {
                            for (int i = 0; i < 20; ++i)
                            {
                                if (IntersectRect(&rect, &yellow.rect, &ycblock[i]))
                                {
                                    yellow.rect.bottom = ycblock[i].top;
                                    yellow.rect.top = yellow.rect.bottom - 110;
                                    break;
                                }
                            }
                        }
                    }
                    if (locatex < 3000 || (locatex > 3000 && BYorP == true))
                    {
                        if (pyblockcheck(purple, pcblock) == 1)
                        {
                            for (int i = 0; i < 20; ++i)
                            {
                                if (IntersectRect(&rect, &purple.rect, &pcblock[i]))
                                {
                                    purple.rect.bottom = pcblock[i].top;
                                    purple.rect.top = purple.rect.bottom - 130;
                                    break;
                                }
                            }
                        }
                    }
                    if (IntersectRect(&rect, &yellow.rect, &roundfinishcheck[1]))
                    {
                        B2yellow = true;
                    }
                    if (IntersectRect(&rect, &purple.rect, &roundfinishcheck[1]))
                    {
                        B2purple = true;
                    }

                    if (B2yellow && B2purple)
                    {
                        B2roundfinish = true;   //round 끝남 체크
                        //SetTimer(hWnd, 3, 100, NULL);
                    }
                    //2round끝나고 추가? 또는 그만?
                    if (obstaclecheck(yellow, obstacle2) == 1 && yellow.state)
                    {
                        yellow.state = false;
                        ++deathcount;
                    }
                    if (obstaclecheck(purple, obstacle2) == 1 && purple.state)
                    {
                        purple.state = false;
                        ++deathcount;
                    }

                    if (monstercheck(yellow, round2) == 1 && yellow.state)
                    {
                        yellow.state = false;
                        ++deathcount;
                    }
                    if (monstercheck(purple, round2) == 1 && purple.state)
                    {
                        purple.state = false;
                        ++deathcount;
                    }
                    if ((yellow.rect.bottom == changeblock.top && yellow.rect.left>=changeblock.left && yellow.rect.right<= changeblock.right)|| IntersectRect(&rect, &yellow.rect, &changeblock))
                    {
                        yellow.rect.bottom = changeblock.top;
                        yellow.rect.top = yellow.rect.bottom - 110;
                        BYorP = true;
                    }
                    else
                        BYorP = false;

                    if (yellow.jump == false)
                    {
                        if (blockcheck(yellow, c2block) == 4)
                            yellow.checkstate = true;
                    }
                    if (purple.jump == false)
                    {
                        if (blockcheck(purple, c2block) == 4)
                            purple.checkstate = true;
                    }
                    if (yellow.jump == false)
                    {
                        if (pyblockcheck(yellow, ycblock) == 4)
                            yellow.pystate = true;
                    }
                    if (purple.jump == false)
                    {
                        if (pyblockcheck(purple, pcblock) == 4)
                            purple.pystate = true;
                    }
                    if (yellow.jump == false)
                    {
                        if (yellow.checkstate && yellow.pystate)
                            OffsetRect(&yellow.rect, 0, 10);
                    }
                    if (purple.jump == false)
                    {
                        if (purple.checkstate && purple.pystate)
                            OffsetRect(&purple.rect, 0, 10);
                    }
                    yellow.checkstate = false;  purple.checkstate = false;
                    yellow.pystate = false;     purple.pystate = false;
                }
                {
                    for (int i = 0; i < 50; ++i)
                    {
                        OffsetRect(&c2block[i], locatex, 0);
                    }
                    for (int i = 0; i < 20; ++i)
                    {
                        OffsetRect(&pcblock[i], locatex, 0);
                    }
                    for (int i = 0; i < 20; ++i)
                    {
                        OffsetRect(&ycblock[i], locatex, 0);
                    }
                    for (int i = 0; i < 20; ++i)
                    {
                        OffsetRect(&obstacle2[i], locatex, 0);
                    }
                    for (int i = 0; i < 2; ++i)
                    {
                        OffsetRect(&round2[i].rect, locatex, 0);
                    }
                    OffsetRect(&changeblock, locatex, 0);
                    OffsetRect(&roundfinishcheck[1], locatex, 0);
                }
                if (BYorPmovecheck == false && BYorP == true)
                {
                    locatex -= 800;
                    OffsetRect(&yellow.rect, 800, 0);
                    OffsetRect(&purple.rect, 800, 0);
                    BYorPmovecheck = true;
                }
            }
            if (Bstage1)
            {
                for (int i = 0; i < 2; ++i)
                {
                    ++round1[i].movingcount;
                    if (i == 0)
                    {
                        if (round1[0].movingcount % 180 == 0)
                            round1[0].dir = !round1[0].dir;
                    }
                    if (i == 1)
                    {
                        if (round1[1].movingcount % 140 == 0)
                            round1[1].dir = !round1[1].dir;
                    }
                    if (round1[i].dir)
                    {
                        OffsetRect(&round1[i].rect, 2, 0);
                        if (round1[i].movingcount % 10 == 0)
                        {
                            if (round1[i].xPos == 0)
                                round1[i].xPos += 400;
                            else
                                round1[i].xPos = 0;
                        }
                    }
                    else
                    {
                        OffsetRect(&round1[i].rect, -2, 0);
                        if (round1[i].movingcount % 10 == 0)
                        {
                            if (round1[i].xPos == 0)
                                round1[i].xPos += 400;
                            else
                                round1[i].xPos = 0;
                        }
                    }
                }
                for (int i = 0; i < 3; ++i)
                {
                    OffsetRect(&checkpoint[i], -locatex, 0);
                    if (IntersectRect(&rect, &checkpoint[i], &yellow.rect))
                        checkcount = i;
                    if (IntersectRect(&rect, &checkpoint[i], &purple.rect))
                        checkcount = i;
                    OffsetRect(&checkpoint[i], locatex, 0);
                }
            }
            if (Bstage2)
            {
                for (int i = 0; i < 2; ++i)
                {
                    ++round2[i].movingcount;

                    if (round2[i].movingcount % 270 == 0)
                        round2[i].dir = !round2[i].dir;
                    if (round2[i].dir)
                    {
                        OffsetRect(&round2[i].rect, 2, 0);
                        if (round2[i].movingcount % 10 == 0)
                        {
                            if (round2[i].xPos == 0)
                                round2[i].xPos = 800;
                            else
                                round2[i].xPos -= 400;
                        }
                    }
                    else
                    {
                        OffsetRect(&round2[i].rect, -2, 0);
                        if (round2[i].movingcount % 10 == 0)
                        {
                            if (round2[i].xPos < 800)
                                round2[i].xPos += 400;
                            else
                                round2[i].xPos = 0;
                        }
                    }
                }
                for (int i = 3; i < 6; ++i)
                {
                    OffsetRect(&checkpoint[i], -locatex, 0);
                    if (IntersectRect(&rect, &checkpoint[i], &yellow.rect))
                        checkcount = i;
                    if (IntersectRect(&rect, &checkpoint[i], &purple.rect))
                        checkcount = i;
                    OffsetRect(&checkpoint[i], locatex, 0);
                }
            }
            //떨어진 거 확인
            if (yellow.rect.top >= 800 && yellow.state)
            {
                yellow.state = false;
                ++deathcount;
            }
            if (purple.rect.top >= 800 && purple.state)
            {
                purple.state = false;
                ++deathcount;
            }
            if (yellow.state == false || purple.state == false)
                Bdie = true;
            break;
        case 2:
            if (min == 60)
            {
                ++hour;
                min = 0;
            }
            if (sec == 60)
            {
                ++min;
                sec = 0;
            }
            ++sec;
            break;
        case 3:
            if(B1roundfinish)
                ++finishcount;
            if (finishcount == 5 && B1roundfinish)
            {
                B1roundfinish = false;
                Bstage1 = false;
                Bstage2 = true;
                locatex = 0;
                yellow.xPos = 600;
                yellow.rect = { 225,380,290,490 };
                purple.xPos = 1200;
                purple.rect = { 80,370,140,480 };
                finishcount = 0;
                KillTimer(hWnd, 3);
            }
            break;
        }
        ReleaseDC(hWnd, hDC);
        InvalidateRgn(hWnd, NULL, FALSE);
        break;
    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);
        memDC = CreateCompatibleDC(hDC);
        hbit = CreateCompatibleBitmap(hDC, wr.right, wr.bottom);
        (HBITMAP)SelectObject(memDC, hbit);
        FillRect(memDC, &wr, (HBRUSH)GetStockObject(BLACK_BRUSH));

        background[0].Draw(memDC, wr.left, wr.top, wr.right, wr.bottom);

        if (Bstart)
            start[0].Draw(memDC, wr.right / 2 - 150, wr.top + 100, 300, 550);

        if (Bstage1)
        {
            stage1background[0].TransparentBlt(memDC, wr.left, wr.top, wr.right, wr.bottom, RGB(0, 0, 0));
            stage1background[1].TransparentBlt(memDC, 100, 100, 400, 100, RGB(0, 0, 0));
            stage1background[2].TransparentBlt(memDC, 800, 250, 300, 50, RGB(0, 0, 0));
            stage1block[0].TransparentBlt(memDC, Rblock[0].left - locatex, Rblock[0].top, Rblock[0].right - Rblock[0].left, Rblock[0].bottom - Rblock[0].top, RGB(0, 0, 0));
            stage1block[1].TransparentBlt(memDC, Rblock[1].left - locatex, Rblock[1].top, Rblock[1].right - Rblock[1].left, Rblock[1].bottom - Rblock[1].top, RGB(0, 0, 0));
            stage1block[2].TransparentBlt(memDC, Rblock[2].left - locatex, Rblock[2].top, Rblock[2].right - Rblock[2].left, Rblock[2].bottom - Rblock[2].top, RGB(0, 0, 0));
            stage1block[3].TransparentBlt(memDC, Rblock[3].left - locatex, Rblock[3].top, Rblock[3].right - Rblock[3].left, Rblock[3].bottom - Rblock[3].top, RGB(0, 0, 0));
            stage1block[4].TransparentBlt(memDC, Rblock[4].left - locatex, Rblock[4].top, Rblock[4].right - Rblock[4].left, Rblock[4].bottom - Rblock[4].top, RGB(0, 0, 0));
            stage1block[5].TransparentBlt(memDC, Rblock[5].left - locatex, Rblock[5].top, Rblock[5].right - Rblock[5].left, Rblock[5].bottom - Rblock[5].top, RGB(0, 0, 0));
            stage1block[6].TransparentBlt(memDC, Rsmallblock[0].left - locatex, Rsmallblock[0].top, Rsmallblock[0].right - Rsmallblock[0].left, Rsmallblock[0].bottom - Rsmallblock[0].top, RGB(0, 0, 0));
            stage1block[6].TransparentBlt(memDC, Rsmallblock[1].left - locatex, Rsmallblock[1].top, Rsmallblock[1].right - Rsmallblock[1].left, Rsmallblock[1].bottom - Rsmallblock[1].top, RGB(0, 0, 0));
            //stage1block[6].TransparentBlt(memDC, Rsmallblock[2].left - locatex, Rsmallblock[2].top, Rsmallblock[2].right - Rsmallblock[2].left, Rsmallblock[2].bottom - Rsmallblock[2].top, RGB(0, 0, 0));
            stage1block[7].TransparentBlt(memDC, Rblock[6].left - locatex, Rblock[6].top, Rblock[6].right - Rblock[6].left, Rblock[6].bottom - Rblock[6].top, RGB(0, 0, 0));
            stage1block[8].TransparentBlt(memDC, Rblock[7].left - locatex, Rblock[7].top, Rblock[7].right - Rblock[7].left, Rblock[7].bottom - Rblock[7].top, RGB(0, 0, 0));
            stage1block[9].TransparentBlt(memDC, Rsmallblock[2].left - locatex, Rsmallblock[2].top, Rsmallblock[2].right - Rsmallblock[2].left, Rsmallblock[2].bottom - Rsmallblock[2].top, RGB(0, 0, 0));//동시발판
            stage1block[9].TransparentBlt(memDC, Rsmallblock[3].left - locatex, Rsmallblock[3].top, Rsmallblock[3].right - Rsmallblock[3].left, Rsmallblock[3].bottom - Rsmallblock[3].top, RGB(0, 0, 0));//동시발판
            stage1block[10].TransparentBlt(memDC, Rblock[8].left - locatex, Rblock[8].top, Rblock[8].right - Rblock[8].left, Rblock[8].bottom - Rblock[8].top, RGB(0, 0, 0));
            stage1block[0].TransparentBlt(memDC, Rblock[9].left - locatex, Rblock[9].top, Rblock[9].right - Rblock[9].left, Rblock[9].bottom - Rblock[9].top, RGB(0, 0, 0));

            if (round1[0].dir)
                monstersprite[1].TransparentBlt(memDC, round1[0].rect.left - locatex, round1[0].rect.top, round1[0].rect.right - round1[0].rect.left, round1[0].rect.bottom - round1[0].rect.top, round1[0].xPos, 0, 400, 300, RGB(255, 0, 0));
            else
                monstersprite[0].TransparentBlt(memDC, round1[0].rect.left - locatex, round1[0].rect.top, round1[0].rect.right - round1[0].rect.left, round1[0].rect.bottom - round1[0].rect.top, round1[0].xPos, 0, 400, 300, RGB(255, 0, 0));
            if (round1[1].dir)
                monstersprite[1].TransparentBlt(memDC, round1[1].rect.left - locatex, round1[1].rect.top, round1[1].rect.right - round1[1].rect.left, round1[1].rect.bottom - round1[1].rect.top, round1[1].xPos, 0, 400, 300, RGB(255, 0, 0));
            else
                monstersprite[0].TransparentBlt(memDC, round1[1].rect.left - locatex, round1[1].rect.top, round1[1].rect.right - round1[1].rect.left, round1[1].rect.bottom - round1[1].rect.top, round1[1].xPos, 0, 400, 300, RGB(255, 0, 0));
            for (int i = 0; i < 3; ++i)
                start[6].Draw(memDC, checkpoint[i].left - locatex, checkpoint[i].top, checkpoint[i].right - checkpoint[i].left, checkpoint[i].bottom - checkpoint[i].top);
        
       /*     HBRUSH hbrush = CreateSolidBrush(RGB(255, 0, 0));
            SelectObject(memDC, hbrush);
            Rectangle(memDC, obstacle1[0].left - locatex, obstacle1[0].top, obstacle1[0].right - locatex, obstacle1[0].bottom);
            DeleteObject(hbrush);*/
        }

        if (Bstage2)
        {
            stage2background[0].TransparentBlt(memDC, wr.left, wr.top, wr.right, wr.bottom, RGB(0, 0, 0));
            stage2block[0].TransparentBlt(memDC, R2block[0].left - locatex, R2block[0].top, R2block[0].right - R2block[0].left, R2block[0].bottom - R2block[0].top, RGB(0, 0, 0));
            stage2block[0].TransparentBlt(memDC, R2block[1].left - locatex, R2block[1].top, R2block[1].right - R2block[1].left, R2block[1].bottom - R2block[1].top, RGB(0, 0, 0));
            stage2block[1].TransparentBlt(memDC, R2block[2].left - locatex, R2block[2].top, R2block[2].right - R2block[2].left, R2block[2].bottom - R2block[2].top, RGB(0, 0, 0));
            stage2block[1].TransparentBlt(memDC, R2block[3].left - locatex, R2block[3].top, R2block[3].right - R2block[3].left, R2block[3].bottom - R2block[3].top, RGB(0, 0, 0));
            stage2block[2].TransparentBlt(memDC, pblock[0].left - locatex, pblock[0].top, pblock[0].right - pblock[0].left, pblock[0].bottom - pblock[0].top, RGB(0, 0, 0));
            stage2block[2].TransparentBlt(memDC, pblock[1].left - locatex, pblock[1].top, pblock[1].right - pblock[1].left, pblock[1].bottom - pblock[1].top, RGB(0, 0, 0));
            stage2block[2].TransparentBlt(memDC, pblock[2].left - locatex, pblock[2].top, pblock[2].right - pblock[2].left, pblock[2].bottom - pblock[2].top, RGB(0, 0, 0));
            stage2block[2].TransparentBlt(memDC, pblock[3].left - locatex, pblock[3].top, pblock[3].right - pblock[3].left, pblock[3].bottom - pblock[3].top, RGB(0, 0, 0));
            stage2block[2].TransparentBlt(memDC, pblock[4].left - locatex, pblock[4].top, pblock[4].right - pblock[4].left, pblock[4].bottom - pblock[4].top, RGB(0, 0, 0));
            stage2block[3].TransparentBlt(memDC, yblock[0].left - locatex, yblock[0].top, yblock[0].right - yblock[0].left, yblock[0].bottom - yblock[0].top, RGB(0, 0, 0));
            stage2block[3].TransparentBlt(memDC, yblock[1].left - locatex, yblock[1].top, yblock[1].right - yblock[1].left, yblock[1].bottom - yblock[1].top, RGB(0, 0, 0));
            stage2block[3].TransparentBlt(memDC, yblock[2].left - locatex, yblock[2].top, yblock[2].right - yblock[2].left, yblock[2].bottom - yblock[2].top, RGB(0, 0, 0));
            stage2block[3].TransparentBlt(memDC, yblock[3].left - locatex, yblock[3].top, yblock[3].right - yblock[3].left, yblock[3].bottom - yblock[3].top, RGB(0, 0, 0));
            stage2block[3].TransparentBlt(memDC, yblock[4].left - locatex, yblock[4].top, yblock[4].right - yblock[4].left, yblock[4].bottom - yblock[4].top, RGB(0, 0, 0));
            stage2block[4].TransparentBlt(memDC, R2block[4].left - locatex, R2block[4].top, R2block[4].right - R2block[4].left, R2block[4].bottom - R2block[4].top, RGB(0, 0, 0));
            stage2block[4].TransparentBlt(memDC, R2block[5].left - locatex, R2block[5].top, R2block[5].right - R2block[5].left, R2block[5].bottom - R2block[5].top, RGB(0, 0, 0));
            stage2block[5].TransparentBlt(memDC, obstacle2[0].left - locatex, obstacle2[0].top, obstacle2[0].right - obstacle2[0].left, obstacle2[0].bottom - obstacle2[0].top, RGB(0, 0, 0));
            stage2block[5].TransparentBlt(memDC, obstacle2[1].left - locatex, obstacle2[1].top, obstacle2[1].right - obstacle2[1].left, obstacle2[1].bottom - obstacle2[1].top, RGB(0, 0, 0));
            stage2block[5].TransparentBlt(memDC, obstacle2[2].left - locatex, obstacle2[2].top, obstacle2[2].right - obstacle2[2].left, obstacle2[2].bottom - obstacle2[2].top, RGB(0, 0, 0));
            stage2block[6].TransparentBlt(memDC, R2block[6].left - locatex, R2block[6].top, R2block[6].right - R2block[6].left, R2block[6].bottom - R2block[6].top, RGB(0, 0, 0)); //위아래로 움직이기
            stage2block[4].TransparentBlt(memDC, R2block[7].left - locatex, R2block[7].top, R2block[7].right - R2block[7].left, R2block[7].bottom - R2block[7].top, RGB(0, 0, 0)); //위아래로 움직이기
            stage2block[4].TransparentBlt(memDC, R2block[8].left - locatex, R2block[8].top, R2block[8].right - R2block[8].left, R2block[8].bottom - R2block[8].top, RGB(0, 0, 0)); //위아래로 움직이기
            stage2block[7].TransparentBlt(memDC, R2block[9].left - locatex, R2block[9].top, R2block[9].right - R2block[9].left, R2block[9].bottom - R2block[9].top, RGB(0, 0, 0)); //위아래로 움직이기
            stage2block[7].TransparentBlt(memDC, R2block[10].left - locatex, R2block[10].top, R2block[10].right - R2block[10].left, R2block[10].bottom - R2block[10].top, RGB(0, 0, 0));
            if (BYorP == false)
            {
                stage2block[3].TransparentBlt(memDC, yblock[5].left - locatex, yblock[5].top, yblock[5].right - yblock[5].left, yblock[5].bottom - yblock[5].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[3].TransparentBlt(memDC, yblock[6].left - locatex, yblock[6].top, yblock[6].right - yblock[6].left, yblock[6].bottom - yblock[6].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[3].TransparentBlt(memDC, yblock[7].left - locatex, yblock[7].top, yblock[7].right - yblock[7].left, yblock[7].bottom - yblock[7].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[3].TransparentBlt(memDC, yblock[8].left - locatex, yblock[8].top, yblock[8].right - yblock[8].left, yblock[8].bottom - yblock[8].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[3].TransparentBlt(memDC, yblock[9].left - locatex, yblock[9].top, yblock[9].right - yblock[9].left, yblock[9].bottom - yblock[9].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[3].TransparentBlt(memDC, yblock[10].left - locatex, yblock[10].top, yblock[10].right - yblock[10].left, yblock[10].bottom - yblock[10].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[3].TransparentBlt(memDC, yblock[11].left - locatex, yblock[11].top, yblock[11].right - yblock[11].left, yblock[11].bottom - yblock[11].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
            }
            else if (BYorP)
            {
                stage2block[2].TransparentBlt(memDC, pblock[5].left - locatex, pblock[5].top, pblock[5].right - pblock[5].left, pblock[5].bottom - pblock[5].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[2].TransparentBlt(memDC, pblock[6].left - locatex, pblock[6].top, pblock[6].right - pblock[6].left, pblock[6].bottom - pblock[6].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[2].TransparentBlt(memDC, pblock[7].left - locatex, pblock[7].top, pblock[7].right - pblock[7].left, pblock[7].bottom - pblock[7].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[2].TransparentBlt(memDC, pblock[8].left - locatex, pblock[8].top, pblock[8].right - pblock[8].left, pblock[8].bottom - pblock[8].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[2].TransparentBlt(memDC, pblock[9].left - locatex, pblock[9].top, pblock[9].right - pblock[9].left, pblock[9].bottom - pblock[9].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[2].TransparentBlt(memDC, pblock[10].left - locatex, pblock[10].top, pblock[10].right - pblock[10].left, pblock[10].bottom - pblock[10].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
                stage2block[2].TransparentBlt(memDC, pblock[11].left - locatex, pblock[11].top, pblock[11].right - pblock[11].left, pblock[11].bottom - pblock[11].top, RGB(0, 0, 0)); //노랑이 도착하면 보라로 변경
            }
            stage2block[4].TransparentBlt(memDC, R2block[11].left - locatex, R2block[11].top, R2block[11].right - R2block[11].left, R2block[11].bottom - R2block[11].top, RGB(0, 0, 0));
            stage2block[4].TransparentBlt(memDC, R2block[12].left - locatex, R2block[12].top, R2block[12].right - R2block[12].left, R2block[12].bottom - R2block[12].top, RGB(0, 0, 0));

            if (round2[0].dir)
                monstersprite[3].TransparentBlt(memDC, round2[0].rect.left - locatex, round2[0].rect.top, round2[0].rect.right - round2[0].rect.left, round2[0].rect.bottom - round2[0].rect.top, round2[0].xPos, 0, 400, 224, RGB(255, 0, 0));
            else
                monstersprite[2].TransparentBlt(memDC, round2[0].rect.left - locatex, round2[0].rect.top, round2[0].rect.right - round2[0].rect.left, round2[0].rect.bottom - round2[0].rect.top, round2[0].xPos, 0, 400, 224, RGB(255, 0, 0));
            if (round2[1].dir)
                monstersprite[3].TransparentBlt(memDC, round2[1].rect.left - locatex, round2[1].rect.top, round2[1].rect.right - round2[1].rect.left, round2[1].rect.bottom - round2[1].rect.top, round2[1].xPos, 0, 400, 224, RGB(255, 0, 0));
            else
                monstersprite[2].TransparentBlt(memDC, round2[1].rect.left - locatex, round2[1].rect.top, round2[1].rect.right - round2[1].rect.left, round2[1].rect.bottom - round2[1].rect.top, round2[1].xPos, 0, 400, 224, RGB(255, 0, 0));
            for (int i = 3; i < 6; ++i)
                start[6].Draw(memDC, checkpoint[i].left - locatex, checkpoint[i].top, checkpoint[i].right - checkpoint[i].left, checkpoint[i].bottom - checkpoint[i].top);
            HBRUSH hbrush = CreateSolidBrush(RGB(255, 0, 0));
            SelectObject(memDC, hbrush);
            Rectangle(memDC, changeblock.left - locatex, changeblock.top, changeblock.right - locatex, changeblock.bottom);
            DeleteObject(hbrush);
        }

        if (Bkeytimer)
        {
            start[2].TransparentBlt(memDC, Rstart[2].left, Rstart[2].top, Rstart[2].right - Rstart[2].left, Rstart[2].bottom - Rstart[2].top, RGB(0, 0, 0)); //보라가 위에 노랑이 오른쪽에 서있게 만들기
            start[3].TransparentBlt(memDC, Rstart[3].left, Rstart[3].top, Rstart[3].right - Rstart[3].left, Rstart[3].bottom - Rstart[3].top, RGB(0, 0, 0)); //보라가 위에 노랑이 오른쪽에 서있게 만들기
            start[4].TransparentBlt(memDC, Rstart[4].left, Rstart[4].top, Rstart[4].right - Rstart[4].left, Rstart[4].bottom - Rstart[4].top, RGB(0, 0, 0)); //보라가 위에 노랑이 오른쪽에 서있게 만들기
        }

        if (Bstage1 || Bstage2)
        {
            if (yellow.state)
            {
                if (yellow.movedir == 0)
                    yellowsprite[1].TransparentBlt(memDC, yellow.rect.left, yellow.rect.top, 150, 150, yellow.xPos, 0, 200, 200, RGB(255, 0, 0));
                else if (yellow.movedir == 1)
                    yellowsprite[0].TransparentBlt(memDC, yellow.rect.left, yellow.rect.top, 150, 150, yellow.xPos, 0, 200, 200, RGB(255, 0, 0));
                else if (yellow.movedir == 2)
                    yellowsprite[2].TransparentBlt(memDC, yellow.rect.left, yellow.rect.top, 150, 150, yellow.xPos, 0, 200, 200, RGB(255, 0, 0));
                else if (yellow.movedir == 3)
                    yellowsprite[3].TransparentBlt(memDC, yellow.rect.left, yellow.rect.top, 150, 150, yellow.xPos, 0, 200, 200, RGB(255, 0, 0));
                else if (yellow.movedir == 4)
                    yellowsprite[4].TransparentBlt(memDC, yellow.rect.left, yellow.rect.top, 150, 150, yellow.xPos, 0, 200, 200, RGB(255, 0, 0));
                else if (yellow.movedir == 5)
                    yellowsprite[5].TransparentBlt(memDC, yellow.rect.left, yellow.rect.top, 150, 150, yellow.xPos, 0, 200, 200, RGB(255, 0, 0));
            }

            if (purple.state)
            {
                if (purple.movedir == 0)
                    purplesprite[1].TransparentBlt(memDC, purple.rect.left, purple.rect.top, 150, 150, purple.xPos, 0, 300, 200, RGB(255, 0, 0));
                else if (purple.movedir == 1)
                    purplesprite[0].TransparentBlt(memDC, purple.rect.left, purple.rect.top, 150, 150, purple.xPos, 0, 300, 200, RGB(255, 0, 0));
                else if (purple.movedir == 2)
                    purplesprite[2].TransparentBlt(memDC, purple.rect.left, purple.rect.top, 150, 150, purple.xPos, 0, 300, 200, RGB(255, 0, 0));
                else if (purple.movedir == 3)
                    purplesprite[3].TransparentBlt(memDC, purple.rect.left, purple.rect.top, 150, 150, purple.xPos, 0, 300, 200, RGB(255, 0, 0));
                else if (purple.movedir == 4)
                    purplesprite[4].TransparentBlt(memDC, purple.rect.left, purple.rect.top, 150, 150, purple.xPos, 0, 300, 200, RGB(255, 0, 0));
                else if (purple.movedir == 5)
                    purplesprite[5].TransparentBlt(memDC, purple.rect.left, purple.rect.top, 150, 150, purple.xPos, 0, 300, 200, RGB(255, 0, 0));
            }
        }

        if (Bdie)
        {
            start[5].Draw(memDC, 300, 300, 600, 170);
            HFONT font = CreateFont(40, 0, 0, 0, 200, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, L"궁서");
            wsprintf(Tdeathcount, L"%d", deathcount);
            wsprintf(Ttime, L"%d : %d : %d", hour, min, sec);
            SelectObject(memDC, font);
            TextOut(memDC, 600, 310, Tdeathcount, lstrlen(Tdeathcount));
            TextOut(memDC, 550, 360, Ttime, lstrlen(Ttime));
            DeleteObject(font);
        }

        if (B1roundfinish)
            finish[0].Draw(memDC, 230, 120, 800, 600);
        if (B2roundfinish)
            finish[1].Draw(memDC, 230, 120, 800, 600);
        BitBlt(hDC, 0, 0, wr.right, wr.bottom, memDC, 0, 0, SRCCOPY);
        DeleteObject(hbit);
        DeleteDC(memDC);
        EndPaint(hWnd, &ps);

        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam); // 위의 세 메시지 외의 나머지 메시지는 OS로
}

int ypcheck(character yellow, character purple)
{
    RECT rect{};
    if (IntersectRect(&rect, &yellow.rect, &purple.rect))
    {
        return 1;
    }
}

int blockcheck(character cha1, RECT cblock[50])
{
    RECT rect{ 0,0,0,0 };
    for (int i = 0; i < 50; ++i)
    {
        if (IntersectRect(&rect, &cha1.rect, &cblock[i]))
        {

            if ((rect.right - rect.left) >= 45)
            {
                rect = { 0,0,0,0 };
                return 1;
            }
        }
    }
    for (int i = 0; i < 50; ++i)
    {
        if (cha1.rect.bottom == cblock[i].top && cha1.rect.right > cblock[i].left && cha1.rect.left < cblock[i].right)
            return 3;
    }
    if (cha1.posstate)
        return 5;
    return 4;
}

int pyblockcheck(character cha1, RECT cblock[20])
{
    RECT rect{ 0,0,0,0 };
    for (int i = 0; i < 20; ++i)
    {
        if (IntersectRect(&rect, &cha1.rect, &cblock[i]))
        {
            if ((rect.right - rect.left) >= (rect.bottom - rect.top))
            {
                if ((rect.right - rect.left) >= 30)
                {
                    rect = { 0,0,0,0 };
                    return 1;
                }
            }
        }
    }
    for (int i = 0; i < 20; ++i)
    {
        if (cha1.rect.bottom == cblock[i].top && cha1.rect.right > cblock[i].left && cha1.rect.left < cblock[i].right)
            return 3;
    }
    if (cha1.posstate)
        return 5;
    return 4;
}

bool obstaclecheck(character cha1, RECT obstacle[20])
{
    RECT rect{ 0,0,0,0 };

    for (int i = 0; i < 20; ++i)
    {
        if (cha1.jump == false)
        {
            if (IntersectRect(&rect, &cha1.rect, &obstacle[i]) && cha1.plusjump == false)
            {
                if (rect.left == obstacle[i].left && rect.right == obstacle[i].right && rect.top == obstacle[i].top && rect.bottom == obstacle[i].bottom)
                    return true;
            }
        }
        else
            return false;
    }
}

bool monstercheck(character cha1, monster m[2])
{
    RECT rect{ 0,0,0,0 };

    RECT copym[2]{};
    for (int i = 0; i < 2; ++i)
    {
        copym[i].left = m[i].rect.left-15;
        copym[i].right = m[i].rect.right-15;
        copym[i].top = m[i].rect.top;
        copym[i].bottom = m[i].rect.bottom;
    }
    for (int i = 0; i < 2; ++i)
    {
        if (cha1.jump == false)
        {
            if (IntersectRect(&rect, &cha1.rect, &copym[i]) && cha1.plusjump == false)
            {
                return true;
            }
        }
        else
            return false;
    }
}