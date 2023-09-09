// Diary.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Diary.h"
#include <CommCtrl.h>
#include <stdio.h>
#include <commdlg.h>
#include <windowsx.h>

#define MAX_LOADSTRING 100
#define ID_BUTTON_SAVE 1000
#define ID_BUTTON_DELETE 1001
#define ID_BUTTON_BMP 1002
#define ID_STATIC_BMP 1003
#define ID_EDIT_NOTE 1004
#define ID_CALENDAR 1005
#define ID_STATIC_DATE 1006
#define MAX_FILEPATH 500
#define MAX_BUFFER 3000




// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND hwndSAVE;                                  //저장버튼 핸들
HWND hwndDELETE;                                  //삭제버튼 핸들
HWND hwndBMP;                                  //BMP선택 버튼 핸들
HWND hwndPICTURE;                              //BMP 표시 컨트롤
HWND hwndEDIT;                                  //EDIT 컨트롤
HWND hwndCALENDAR;                              //달력
HWND hwndDATE;                                  //날짜 표시
TCHAR szDate[MAX_LOADSTRING];
TCHAR szCurDir[MAX_FILEPATH];                   //현재 디렉토리
HBITMAP hBitmap;                                //bmp파일 비트맵 핸들
TCHAR szCurBmpFile[MAX_FILEPATH];               //표시된 bmp 파일 명


// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void SaveItem();
void DeleteItem();
void SelectBmp(HWND);
void ReadItem(); 
void SetMonthState(SYSTEMTIME);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_DIARY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIARY));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIARY));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DIARY);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SYSTEMTIME time;
    NMHDR nmh;
    int wmId;
    HDC hDC, hMemDC;
    HBITMAP hOldBitmap;
    static HBITMAP hBkgBitmap;
    int bx, by;
    BITMAP bit;
    LPDRAWITEMSTRUCT pDS;
    TCHAR buffer[MAX_BUFFER];

    switch (message)
    {
    case WM_CREATE:

        // child control 만들기
        hwndSAVE = CreateWindow("BUTTON", "저장", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_DISABLED,  //저장버튼
                   690, 420, 90, 30,hWnd, (HMENU)ID_BUTTON_SAVE, hInst, NULL); 

        hwndDELETE = CreateWindow("BUTTON", "삭제", WS_CHILD | WS_VISIBLE | WS_BORDER| WS_DISABLED, //삭제 버튼
                     783, 420, 90, 30, hWnd, (HMENU)ID_BUTTON_DELETE, hInst, NULL);

        hwndBMP = CreateWindow("BUTTON", "사진 선택", WS_CHILD | WS_VISIBLE | WS_BORDER,  //사진 선택 버튼
            690, 130, 90, 30, hWnd, (HMENU)ID_BUTTON_BMP, hInst, NULL);

        hwndPICTURE = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_OWNERDRAW, //사진 표시 (정적 컨트롤)
            580, 170, 300, 200, hWnd, (HMENU)ID_STATIC_BMP, hInst, NULL);

        hwndEDIT = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER,   //내용 입력 (에디트 컨트롤) 
            250, 170, 300, 200, hWnd, (HMENU)ID_EDIT_NOTE, hInst, NULL);

        hwndDATE = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | WS_BORDER,  //날짜 표시 
            250, 150, 300, 20, hWnd, (HMENU)ID_STATIC_DATE, hInst, NULL);

        hwndCALENDAR = CreateWindowEx(0, MONTHCAL_CLASS, "", WS_CHILD | WS_VISIBLE | WS_BORDER | MCS_DAYSTATE,  //캘린더 컨트롤
            10, 140, 200, 250, hWnd, (HMENU)ID_CALENDAR, hInst, NULL);
        HFONT hFont = CreateFont(20, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
            VARIABLE_PITCH | FF_ROMAN, "Arial");
        SendMessage(hwndCALENDAR, WM_SETFONT, hFont, TRUE); //캘린더 컨트롤 폰트 수정

        GetCurrentDirectory(sizeof(szCurDir), szCurDir);  

        hBkgBitmap = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BACK));
        break;

    case WM_NOTIFY:
        nmh = *(LPNMHDR)lParam; //lParam 에 들어있는 주소를 NMHDR* 로 형변환
        switch (nmh.idFrom) {
        case ID_CALENDAR:
            if (nmh.code == MCN_SELCHANGE) { //MCN_SELCHANGE : 선택한 날짜가 변경되는 경우 전송되는 메시지

                //달력에서 클릭한 날짜 표시
                MonthCal_GetCurSel(hwndCALENDAR, (LPSYSTEMTIME) &time);
                wsprintf(szDate, "%4d-%02d-%2d", time.wYear, time.wMonth, time.wDay); //szDate 에 년-월-일 형태로 담는다
                SetWindowText(hwndDATE, szDate); //컨트롤에 날짜 표시
                ReadItem();  //내용,사진 읽어오기
                SetMonthState(time);  //날짜 볼드처리 
            }
            break;

        }
        return FALSE;
        break;


    case WM_COMMAND:
        {
            wmId = LOWORD(wParam);
            
            switch (wmId)
            {
            case ID_EDIT_NOTE:
                GetWindowText(hwndEDIT, buffer, sizeof(buffer)); //에디트 컨트롤 내용 가져옴
                EnableWindow(hwndSAVE, lstrlen(buffer) > 0);  //내용이 있을 경우 저장 버튼 활성화 
                break;

            case ID_BUTTON_SAVE: //저장버튼
                SaveItem();
                break;

            case ID_BUTTON_DELETE: //삭제 버튼
                DeleteItem();
                break;

            case ID_BUTTON_BMP:  //사진 선택 버튼
                SelectBmp(hWnd);
                break;

            case IDM_EXIT: //종료
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;  
    case WM_DRAWITEM:
        wmId = LOWORD(wParam);
        pDS = (LPDRAWITEMSTRUCT)lParam;

        if (wmId == ID_STATIC_BMP) {
            hDC = pDS->hDC;
            if (hBitmap != NULL) {
                hDC = pDS->hDC;
                hMemDC = CreateCompatibleDC(hDC); //hDC와 호환되는 DC생성
                hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);  //메모리 DC에 선택 -> 메모리DC 표면에 비트맵 그려짐

                GetObject(hBitmap, sizeof(BITMAP), &bit);  //비트맵의 GDI정보 조사
                bx = bit.bmWidth;
                by = bit.bmHeight;

                BitBlt(hDC, 0, 0, bx, by, hMemDC, 0, 0, SRCCOPY); //메모리 DC표면에 그려진 비트맵을 화면 DC로 복사

                SelectObject(hMemDC, hOldBitmap); 

                DeleteDC(hMemDC); //메모리DC 해제 
                UpdateWindow(hwndPICTURE);  //영역 갱신

            }
            else {
                RECT rect;
                GetClientRect(hwndPICTURE, &rect);  //컨트롤 좌표값 구함
                FillRect(hDC, &rect, (HBRUSH)(COLOR_WINDOW + 1)); 

            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            hMemDC = CreateCompatibleDC(hdc);
            SelectObject(hMemDC, hBkgBitmap);
            BitBlt(hdc, 0, 0, 1347, 803, hMemDC, 0, 0, SRCCOPY);
            DeleteObject(hMemDC);

            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


//저장
void SaveItem() {
    HANDLE hFile;
    int size;
    TCHAR buffer[MAX_BUFFER];
    TCHAR fullfilename[MAX_FILEPATH];


    //edit내용 저장
    wsprintf(fullfilename, "%s\\%s.txt", szCurDir, szDate);

    hFile = CreateFile(fullfilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    GetWindowText(hwndEDIT, buffer, sizeof(buffer));
    size = lstrlen(buffer);

    WriteFile(hFile, buffer, size, (LPDWORD)&size, NULL);
    CloseHandle(hFile);

    //bmp파일 저장
    wsprintf(fullfilename, "%s\\%s.bmp", szCurDir, szDate);
    CopyFile(szCurBmpFile, fullfilename, FALSE); 

    EnableWindow(hwndDELETE,TRUE);
}

//삭제
void DeleteItem() {
    TCHAR fullfilename[MAX_FILEPATH];

    SetWindowText(hwndEDIT, "");  //에디트컨트롤 문자열을 비움

    wsprintf(fullfilename, "%s\\%s.txt", szCurDir, szDate); //txt삭제
    DeleteFile(fullfilename);

    wsprintf(fullfilename, "%s\\%s.bmp", szCurDir, szDate); //bmp삭제
    DeleteFile(fullfilename);
    InvalidateRect(hwndPICTURE, NULL, TRUE);

    EnableWindow(hwndDELETE, FALSE); //내용이 없어졌으므로 삭제버튼 비활성화
}

//사진 선택
void SelectBmp(HWND hwnd) {
    OPENFILENAME OFN;
    TCHAR lpstrFile[MAX_FILEPATH] = "";
    static TCHAR filter[] = "BMP 파일\0* .bmp";

    memset(&OFN, 0, sizeof(OPENFILENAME));
    OFN.lStructSize = sizeof(OPENFILENAME); //파일 크기
    OFN.hwndOwner = hwnd;   //파일열기 대화상자를 실행하는 핸들값
    OFN.lpstrFilter = filter;   //필터 설정 (bmp파일만 선택) 
    OFN.lpstrFile = lpstrFile;  //파일 경로 
    OFN.nMaxFile = 100;  //lpstrFile 이 가리키는 문자열의 크기
    OFN.lpstrInitialDir = ".";  //파일 찾기를 시작할 디렉토리 지정

    if (GetOpenFileName(&OFN) != 0) {
        lstrcpy((LPSTR) szCurBmpFile,(LPCSTR) OFN.lpstrFile); //파일 경로 szCurBmpFile 에 복사
        hBitmap = LoadImage(NULL, szCurBmpFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); //사진 불러오기
        InvalidateRect(hwndPICTURE, NULL, TRUE);
    }

}

void ReadItem() {
    HANDLE hFile;
    int size;
    TCHAR buffer[MAX_BUFFER];
    TCHAR fullfilename[MAX_FILEPATH];
    DWORD charNum;
    int fileSize;

    //EDIT 내용 읽기 
    SetWindowText(hwndEDIT, "");

    wsprintf(fullfilename, "%s\\%s.txt", szCurDir, szDate); 

    DeleteObject(hBitmap);  //사용한 gdi 오브젝트 제거 
    hBitmap = NULL;
    InvalidateRect(hwndPICTURE, NULL, TRUE); //윈도우 갱신

    hFile = CreateFile(fullfilename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    fileSize = GetFileSize(hFile, &size);
        
    size = lstrlen(buffer); 

    memset(buffer, 0, sizeof(buffer));
    ReadFile(hFile, buffer, fileSize, &charNum, NULL);
    CloseHandle(hFile);

    SetWindowText(hwndEDIT, buffer);

    //bmp파일 읽기
    wsprintf(fullfilename, "%s\\%s.bmp", szCurDir, szDate);
    hBitmap = LoadImage(NULL, fullfilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); //비트맵 로드
    InvalidateRect(hwndPICTURE, NULL, TRUE);

    EnableWindow(hwndDELETE, TRUE);   //삭제 버튼 활성화
}


void SetMonthState(SYSTEMTIME time) {
    TCHAR fullfilename[MAX_FILEPATH];
    MONTHDAYSTATE arrMonthState[3];
    SYSTEMTIME arrTimeRange[3];
    int nIndex;

    arrMonthState[0] = 0;
    arrMonthState[1] = 0;
    arrMonthState[2] = 0;
    
    MonthCal_GetMonthRange(hwndCALENDAR, GMR_DAYSTATE, arrTimeRange);
    if (arrTimeRange[0].wMonth == time.wMonth) 
        nIndex = 0;    //부분적으로 표시되는 전 월이 없는 경우 (두 개의월이 표시됨) 
    else
        nIndex = 1;  //부분적으로 표시되는 전 월이 있는 경우 (세 개의 월이 표시됨)

    


    for (int i = 0; i <= 31; i++) {
        wsprintf(fullfilename, "%4d-%02d-%2d.txt", time.wYear, time.wMonth, i+1);
        if (GetFileAttributes(fullfilename) != INVALID_FILE_ATTRIBUTES)   //저장된 파일이 존재 하는 경우 
            arrMonthState[nIndex] |= 0x01 << i;    // 해당 일에 대해 비트 1로 설정
    }
    MonthCal_SetDayState(hwndCALENDAR, nIndex + 2, arrMonthState);      //표시된 월의 날짜별 상태 설정 (굵은 표시) 
}







