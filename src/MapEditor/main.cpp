#include <windows.h>
#include <stdio.h>
#include "res.h"
#include "resource.h"

#define CLDWND1		1001
#define SRLLBAR1	1010

#define WM_ChangeStage	WM_USER+1

class Object_c {
public:
	int No;
	char *NextStage;
	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFO bmpInfo;
	BYTE *bPixelBits;
	POINT Pt;
	POINT BPt;
	SIZE Sz;
	Object_c *prev, *next;

	Object_c() {
		No = BPt.x = BPt.y = Pt.x = Pt.y = Sz.cx = Sz.cy = 0;
		prev = next = NULL;
	}
};

typedef struct _Set {
	char MidiFileName[150];
	int Timer;
	int ScreenX;
	int ScreenMax;
} WindowStates;

LRESULT CALLBACK ChildWndProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp );
LRESULT CALLBACK WndProc(HWND hWnd , UINT msg , WPARAM wp , LPARAM lp);
BOOL	CALLBACK DlgStage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL	CALLBACK CreateStage(HWND hWnd, UINT message, WPARAM wp, LPARAM lp);
BOOL	CALLBACK Version(HWND hWnd, UINT message, WPARAM wp, LPARAM lp);
BOOL	CALLBACK NextStageName(HWND hWnd, UINT message, WPARAM wp, LPARAM lp);

int DestroyAllObject_c();
int Loading(HWND hWnd);
DWORD GetInObj(FILE *fpin, TCHAR *order);
int ReadBmp(HWND hWnd, PSTR pstr, Object_c *Obj);
int OnlyOneCheck(HWND hWnd,int MID);
int ShowObject_c(HDC hdc, Object_c *Obtmp);
int SaveStage(FILE *fp);
int DestroyPtObject_c(POINT Pt);
int MESG(HWND hWnd, int i)
{
	char s[10];
	sprintf(s, "%d",i);
	MessageBox(hWnd, s, "ナンバーメッセージ", MB_OK);
	return 1;
}

int NowID;
char StageName[150];
WindowStates *WindowSt;
SCROLLINFO scr;

Object_c *Otmp;			// Object_の一時的
Object_c MyObj;			// マウス用の表示クラス（？）みたいなやつ
Object_c Human;			// 主人公用
Object_c *Enemies;		// 敵
Object_c StartEnemy;	// 敵の雛形
Object_c *Items;		// アイテム
Object_c StartItem;		// アイテムの雛形
Object_c *Effect;		// 描画効果
Object_c StartEffect;	// 効果の雛形
Object_c *Block;		// ブロック
Object_c StartBlock;	// ブロックの雛形
Object_c *Goal;			// ゴール
Object_c StartGoal;		// ゴールの雛形
Object_c StartEraser;	// 消しゴム


int WINAPI WinMain(HINSTANCE hInstance , HINSTANCE hPrevInstance ,
			PSTR lpCmdLine , int nCmdShow) {
	HWND hWnd;
	MSG msg;
	WNDCLASS winc;
	int gm;

	winc.style 			= CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc	= WndProc;
	winc.cbClsExtra		= winc.cbWndExtra	= 0;
	winc.hInstance		= hInstance;
	winc.hIcon			= LoadIcon(NULL , IDI_APPLICATION);
	winc.hCursor		= LoadCursor(NULL , IDC_ARROW);
	winc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName	= "MYMENU";
	winc.lpszClassName	= TEXT("MainWindow");

	if (!RegisterClass(&winc)) return 1;

	hWnd = CreateWindow(
			TEXT("MainWindow") , TEXT("マリクス マップエディター") ,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE ,
			CW_USEDEFAULT , CW_USEDEFAULT ,
			800 , 600 ,
			NULL , NULL , hInstance , NULL
	);

	if (hWnd == NULL) return 1;

	while ( 1 ) {
		gm = GetMessage(&msg , NULL , 0 , 0 );
		if( gm<=0 ) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd , UINT msg , WPARAM wp , LPARAM lp)
{
	static HWND chwnd;
	static HMENU hmenu;
	MENUITEMINFO menuInfo;
	POINT Pt;

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		int id;
		id = MessageBox(hWnd,"終了しますか？", "終了メッセージ", MB_YESNO);
		if( id==IDYES )
		{
			SendMessage(hWnd, WM_DESTROY, 0, 0);
		}
		return 0;
	case WM_CREATE:
		WindowSt = (WindowStates*)malloc(sizeof(WindowStates));
		memset(WindowSt->MidiFileName,'\0',140);
		strcpy(WindowSt->MidiFileName,"mu_nantou.mid");
		WindowSt->ScreenMax = 640;
		WindowSt->ScreenX = 0;
		WindowSt->Timer = 30;
		DialogBox(
			(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
			MAKEINTRESOURCE(IDD_DIALOG2),
			hWnd, (DLGPROC)CreateStage);
		DialogBox(
			(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
			MAKEINTRESOURCE(IDD_DIALOG1),
			hWnd, (DLGPROC)DlgStage);

		Loading(hWnd);
		memcpy(&MyObj, &Human, sizeof(Object_c));

		WNDCLASS cwc;
		cwc.style			= CS_HREDRAW | CS_VREDRAW;
		cwc.lpfnWndProc		= ChildWndProc;
		cwc.cbClsExtra		= cwc.cbWndExtra	= 0;
		cwc.hInstance		= ((LPCREATESTRUCT)(lp))->hInstance;
		cwc.hIcon			= NULL;
		cwc.hCursor			= LoadCursor(NULL , IDC_ARROW);
		cwc.hbrBackground	= (HBRUSH)CreateSolidBrush(RGB(0,0xff,0x80));
		cwc.lpszMenuName	= NULL;
		cwc.lpszClassName	= TEXT("CHILD");

		RegisterClass(&cwc);
		
		chwnd = CreateWindow(
			TEXT("CHILD"),
			TEXT("メイクウィンドウ"),
			WS_OVERLAPPED | WS_CHILD | WS_BORDER | WS_VISIBLE | WS_HSCROLL,
			110,30,
			640,498,
			hWnd,
			(HMENU)CLDWND1,
			((LPCREATESTRUCT)(lp))->hInstance,
			NULL
		);

		CreateWindow(
			TEXT("BUTTON"),
			TEXT("主人公"),
			WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			10, 30,
			100, 30,
			hWnd,
			(HMENU)IDM_HUMAN,
			((LPCREATESTRUCT)(lp))->hInstance,
			NULL
		);
		CreateWindow(
			TEXT("BUTTON"),
			TEXT("敵"),
			WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
			10, 60,
			100, 30,
			hWnd,
			(HMENU)IDM_ENEMY,
			((LPCREATESTRUCT)(lp))->hInstance,
			NULL
		);
		CreateWindow(
			TEXT("BUTTON"),
			TEXT("アイテム"),
			WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
			10, 90,
			100, 30,
			hWnd,
			(HMENU)IDM_ITEM,
			((LPCREATESTRUCT)(lp))->hInstance,
			NULL
		);
		CreateWindow(
			TEXT("BUTTON"),
			TEXT("ゴール"),
			WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
			10, 120,
			100, 30,
			hWnd,
			(HMENU)IDM_GOAL,
			((LPCREATESTRUCT)(lp))->hInstance,
			NULL
		);
		CreateWindow(
			TEXT("BUTTON"),
			TEXT("ブロック"),
			WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
			10, 150,
			100, 30,
			hWnd,
			(HMENU)IDM_BLOCK,
			((LPCREATESTRUCT)(lp))->hInstance,
			NULL
		);
		CreateWindow(
			TEXT("BUTTON"),
			TEXT("消しゴム"),
			WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
			10, 180,
			100, 30,
			hWnd,
			(HMENU)IDM_ERASER,
			((LPCREATESTRUCT)(lp))->hInstance,
			NULL
		);

		menuInfo.cbSize = sizeof(MENUITEMINFO);
		OnlyOneCheck(hWnd,IDM_HUMAN);
		return 0;
	case WM_COMMAND:
		TCHAR s[100];

		*s = '\0';

		switch( LOWORD(wp) )
		{
		case IDM_NEW:
			DestroyAllObject_c();
			InvalidateRect(chwnd, NULL, TRUE);
			DialogBox(
				(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
				MAKEINTRESOURCE(IDD_DIALOG2),
				hWnd, (DLGPROC)CreateStage);
			break;
		case IDM_SAVE:
			FILE *fp;
			int k;
			k=0;
			CreateDirectory("stage",NULL);
			if( !(fp=fopen(StageName,"w")) )
			{
				MessageBox(hWnd, "ステージファイルを作成できません", StageName, MB_OK);
				return 1;
			}
			SaveStage(fp);
			fclose(fp);
			break;
		case IDM_ABOUT:
			DialogBox(
				(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
				MAKEINTRESOURCE(IDD_DIALOG3),
				hWnd, (DLGPROC)Version);
			break;
		case IDM_END:
			lstrcpy(s, "終了しまっす");
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case IDM_STAGE:
			lstrcpy(s, "ステージの設定すんだな");
			DialogBox(
				(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
				MAKEINTRESOURCE(IDD_DIALOG1),
				hWnd, (DLGPROC)DlgStage);
			break;
		case IDM_HUMAN:
			memcpy(&MyObj, &Human, sizeof(Object_c));
			OnlyOneCheck(hWnd, IDM_HUMAN);
			break;
		case IDM_ENEMY:
			Pt = MyObj.Pt;
			memcpy(&MyObj, StartEnemy.next, sizeof(Object_c));
			MyObj.Pt = Pt;
			OnlyOneCheck(hWnd, IDM_ENEMY);
			break;
		case IDM_ITEM:
			Pt = MyObj.Pt;
			memcpy(&MyObj, StartItem.next, sizeof(Object_c));
			MyObj.Pt = Pt;
			OnlyOneCheck(hWnd, IDM_ITEM);
			break;
		case IDM_GOAL:
			Pt = MyObj.Pt;
			memcpy(&MyObj, StartGoal.next, sizeof(Object_c));
			MyObj.Pt = Pt;
			OnlyOneCheck(hWnd, IDM_GOAL);
			break;
		case IDM_BLOCK:
			Pt = MyObj.Pt;
			memcpy(&MyObj, StartBlock.next, sizeof(Object_c));
			MyObj.Pt = Pt;
			OnlyOneCheck(hWnd, IDM_BLOCK);
			break;
		case IDM_ERASER:
			Pt = MyObj.Pt;
			memcpy(&MyObj, StartEraser.next, sizeof(Object_c));
			MyObj.Pt = Pt;
			OnlyOneCheck(hWnd, IDM_ERASER);
		}
		InvalidateRect(hWnd,NULL,TRUE);

		return 0;
	case WM_ChangeStage:
		SendMessage(chwnd, msg, wp, lp);
		return 0;
	}
	return DefWindowProc(hWnd , msg , wp , lp);
}

LRESULT CALLBACK ChildWndProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
	static HWND scroll;
	static int dx;//増分
	static int range;//最大スクロール範囲
	HDC hdc;
	PAINTSTRUCT ps;
	static RECT rc;
	RECT rcs;
	static POINT MP;
	POINT Pt;

	switch( msg )
	{
	case WM_CREATE:
		scr.cbSize = sizeof( SCROLLINFO );
		scr.fMask = SIF_PAGE | SIF_RANGE;
		scr.nMin = 0;
		scr.nMax = WindowSt->ScreenMax;
		scr.nPage = 640;

		SetScrollInfo(hWnd, SB_HORZ, &scr,TRUE);

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_PAINT:
		LOGPEN lopnPen;
		lopnPen.lopnStyle = PS_SOLID;
		lopnPen.lopnWidth.x = 3;
		lopnPen.lopnColor = RGB(0,0,0xff);

		hdc = BeginPaint(hWnd , &ps);
		SelectObject(hdc , CreatePenIndirect(&lopnPen));
		SelectObject(hdc, GetStockObject(NULL_BRUSH));

		ShowObject_c(hdc, &Human);
		ShowObject_c(hdc, Enemies);
		ShowObject_c(hdc, Items);
		ShowObject_c(hdc, Block);
		ShowObject_c(hdc, Goal);

		SetDIBitsToDevice(
			hdc , MP.x , MP.y ,
			MyObj.Sz.cx, MyObj.Sz.cy,
			MyObj.BPt.x , MyObj.bmpInfo.bmiHeader.biHeight-(MyObj.BPt.y+MyObj.Sz.cy),
			MyObj.BPt.x, MyObj.bmpInfo.bmiHeader.biHeight,
			MyObj.bPixelBits , &MyObj.bmpInfo , DIB_RGB_COLORS
		);

		DeleteObject(SelectObject(hdc , GetStockObject(BLACK_PEN)));

		EndPaint(hWnd , &ps);
		return 0;
	case WM_MOUSEMOVE:
		InvalidateRect(hWnd, &rc, TRUE);
		MP.x = LOWORD(lp);
		MP.y = HIWORD(lp);
		MP.x = MP.x - (MP.x+scr.nPos)%8;
		MP.y = MP.y - (MP.y+scr.nPos)%8;
		MyObj.Pt = MP;
		SetRect(&rc,MyObj.Pt.x,MyObj.Pt.y,MyObj.Pt.x+MyObj.Sz.cx,MyObj.Pt.y+MyObj.Sz.cy);
		InvalidateRect(hWnd, &rc, TRUE);
		return 0;
	case WM_RBUTTONUP:
		switch( NowID )
		{
		case IDM_ENEMY:
			Pt = MyObj.Pt;
			if( MyObj.next==NULL )
			{
				memcpy(&MyObj,StartEnemy.next,sizeof(Object_c));
			}
			else
			{
				memcpy(&MyObj,MyObj.next,sizeof(Object_c));
			}
			MyObj.Pt = Pt;
			break;
		case IDM_ITEM:
			Pt = MyObj.Pt;
			if( MyObj.next==NULL )
			{
				memcpy(&MyObj,StartItem.next,sizeof(Object_c));
			}
			else
			{
				memcpy(&MyObj,MyObj.next,sizeof(Object_c));
			}
			MyObj.Pt = Pt;
			break;
		case IDM_GOAL:
			Pt = MyObj.Pt;
			if( MyObj.next==NULL )
			{
				memcpy(&MyObj,StartGoal.next,sizeof(Object_c));
			}
			else
			{
				memcpy(&MyObj,MyObj.next,sizeof(Object_c));
			}
			MyObj.Pt = Pt;
			break;
		case IDM_BLOCK:
			Pt = MyObj.Pt;
			if( MyObj.next==NULL )
			{
				memcpy(&MyObj,StartBlock.next,sizeof(Object_c));
			}
			else
			{
				memcpy(&MyObj,MyObj.next,sizeof(Object_c));
			}
			MyObj.Pt = Pt;
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_LBUTTONUP:
		Otmp = &MyObj;
		SetRect(&rcs,0,0,0,0);
		switch( NowID )
		{
		case IDM_HUMAN:
			Otmp = &Human;
			SetRect(&rcs, Otmp->Pt.x-2, Otmp->Pt.y-2,
				Otmp->Pt.x+Otmp->bmpInfo.bmiHeader.biWidth+2, Otmp->Pt.y+Otmp->bmpInfo.bmiHeader.biHeight+2);
			InvalidateRect(hWnd, &rcs, TRUE);
			Human.Pt.x = MyObj.Pt.x + scr.nPos;
			Human.Pt.y = MyObj.Pt.y;
			SetRect(&rcs, Otmp->Pt.x, Otmp->Pt.y,
				Otmp->Pt.x+Otmp->bmpInfo.bmiHeader.biWidth, Otmp->Pt.y+Otmp->bmpInfo.bmiHeader.biHeight);
			break;
		case IDM_ENEMY:
			Otmp = new Object_c;
			memcpy(Otmp,&MyObj,sizeof(Object_c));
			if( Enemies==NULL )
			{
				Otmp->next = NULL;
				Otmp->prev = NULL;
				Enemies = Otmp;
			}
			else
			{
				Otmp->next = Enemies;
				Otmp->prev = NULL;
				Enemies->prev = Otmp;
				Enemies = Otmp;
			}
			Otmp->Pt.x = MyObj.Pt.x + scr.nPos;
			Otmp->Pt.y = MyObj.Pt.y;
			SetRect(&rcs, Otmp->Pt.x, Otmp->Pt.y,
				Otmp->Pt.x+Otmp->bmpInfo.bmiHeader.biWidth, Otmp->Pt.y+Otmp->bmpInfo.bmiHeader.biHeight);

			break;
		case IDM_ITEM:
			Otmp = new Object_c;
			memcpy(Otmp,&MyObj,sizeof(Object_c));
			if( Items==NULL )
			{
				Otmp->next = NULL;
				Otmp->prev = NULL;
				Items = Otmp;
			}
			else
			{
				Otmp->next = Items;
				Otmp->prev = NULL;
				Items->prev = Otmp;
				Items = Otmp;
			}
			Otmp->Pt.x = MyObj.Pt.x + scr.nPos;
			Otmp->Pt.y = MyObj.Pt.y;
			SetRect(&rcs, Otmp->Pt.x, Otmp->Pt.y,
				Otmp->Pt.x+Otmp->bmpInfo.bmiHeader.biWidth, Otmp->Pt.y+Otmp->bmpInfo.bmiHeader.biHeight);

			break;
		case IDM_GOAL:
			Otmp = new Object_c;
			memcpy(Otmp,&MyObj,sizeof(Object_c));
			if( Goal==NULL )
			{
				Otmp->next = NULL;
				Otmp->prev = NULL;
				Goal = Otmp;
			}
			else
			{
				Otmp->next = Goal;
				Otmp->prev = NULL;
				Goal->prev = Otmp;
				Goal = Otmp;
			}
			Otmp->Pt.x = MyObj.Pt.x + scr.nPos;
			Otmp->Pt.y = MyObj.Pt.y;
			SetRect(&rcs, Otmp->Pt.x, Otmp->Pt.y,
				Otmp->Pt.x+Otmp->bmpInfo.bmiHeader.biWidth, Otmp->Pt.y+Otmp->bmpInfo.bmiHeader.biHeight);

			DialogBox(
				(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
				MAKEINTRESOURCE(IDD_DIALOG4),
				hWnd, (DLGPROC)NextStageName);
			break;
		case IDM_BLOCK:
			Otmp = new Object_c;
			memcpy(Otmp,&MyObj,sizeof(Object_c));
			if( Block==NULL )
			{
				Otmp->next = NULL;
				Otmp->prev = NULL;
				Block = Otmp;
			}
			else
			{
				Otmp->next = Block;
				Otmp->prev = NULL;
				Block->prev = Otmp;
				Block = Otmp;
			}
			Otmp->Pt.x = MyObj.Pt.x + scr.nPos;
			Otmp->Pt.y = MyObj.Pt.y;
			SetRect(&rcs, Otmp->Pt.x, Otmp->Pt.y,
				Otmp->Pt.x+Otmp->bmpInfo.bmiHeader.biWidth, Otmp->Pt.y+Otmp->bmpInfo.bmiHeader.biHeight);

			break;
		case IDM_ERASER:
			MyObj.Pt.x += scr.nPos;
			DestroyPtObject_c(MyObj.Pt);
			SetRect(&rcs,0,0,640,480);
			break;
		}

		SetRect(&rcs,rcs.left-5,rcs.top-5,rcs.right+5,rcs.bottom+5);

		InvalidateRect(hWnd, &rcs, TRUE);
		return 0;
	case WM_HSCROLL:
        switch (LOWORD(wp)) {
            case SB_LINELEFT:
                dx = -1;
                break;
            case SB_LINERIGHT:
                dx = 1;
                break;
            case SB_THUMBPOSITION:
                dx = HIWORD(wp)-scr.nPos;
                break;
            case SB_PAGERIGHT:
                dx = 240;
                break;
            case SB_PAGELEFT:
                dx = -240;
                break;
            default:
                dx = 0;
                break;
        }
        dx = max(-scr.nPos, min(dx, scr.nMax - (scr.nPos+(signed)scr.nPage)));

        if (dx != 0) {
			scr.nPos += dx;
            ScrollWindow(hWnd, -dx, 0, NULL, NULL);
            SetScrollPos(hWnd, SB_HORZ, scr.nPos, TRUE);
            UpdateWindow(hWnd);
        }
		InvalidateRect(hWnd, NULL, TRUE);
        break;
			
	case WM_ChangeStage:

		scr.fMask = SIF_PAGE | SIF_RANGE;
		scr.nMin = 0;
		scr.nMax = WindowSt->ScreenMax;
		scr.nPage = 640;

		SetScrollInfo(hWnd, SB_HORZ, &scr,TRUE);

		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	return DefWindowProc( hWnd, msg, wp, lp );
}

BOOL CALLBACK DlgStage(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	char s[80];
	switch( message )
	{
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_EDIT1, WindowSt->MidiFileName);
		sprintf(s, "%d", WindowSt->ScreenX);
		SetDlgItemText(hWnd, IDC_EDIT2, s);
		sprintf(s, "%d", WindowSt->ScreenMax);
		SetDlgItemText(hWnd, IDC_EDIT3, s);
		sprintf(s, "%d", WindowSt->Timer);
		SetDlgItemText(hWnd, IDC_EDIT4, s);
		return TRUE;
	case WM_CLOSE:
		SendMessage(hWnd,WM_COMMAND,IDOK,0);
		return TRUE;
	case WM_COMMAND:
		switch( wp )
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_EDIT1, s, 79);
			strcpy(WindowSt->MidiFileName,s);
			GetDlgItemText(hWnd, IDC_EDIT2, s, 79);
			WindowSt->ScreenX = atoi(s);
			GetDlgItemText(hWnd, IDC_EDIT3, s, 79);
			WindowSt->ScreenMax = atoi(s);
			GetDlgItemText(hWnd, IDC_EDIT4, s, 79);
			WindowSt->Timer = atoi(s);
			if( WindowSt->ScreenMax < 640 )
				WindowSt->ScreenMax = 640;
			if( WindowSt->ScreenX > WindowSt->ScreenMax )
				WindowSt->ScreenX = 640;
			SendMessage(GetParent(hWnd),WM_ChangeStage, 0 , 0);
			EndDialog(hWnd, 0);
			break;
		}
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK CreateStage(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	static char s[150];
	switch( message )
	{
	case WM_CLOSE:
		SendMessage(hWnd, WM_COMMAND, IDOK, 0);
		return TRUE;
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch( wp )
		{
			case IDOK:
				GetDlgItemText(hWnd, IDC_EDIT1, s, 149);
				if( !strlen(s) )
				{
					MessageBox(hWnd, "ステージファイル名を入力してください", "警告めっせぇじ", MB_OK);
					return TRUE;
				}
				sprintf(StageName, "stage\\\\%s.stg",s);
				EndDialog(hWnd, 0);
				break;
		}
		return TRUE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK Version(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	switch( message )
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	case WM_COMMAND:
		switch( wp )
		{
		case IDOK:
			EndDialog(hWnd, 0);
			break;
		}
		return TRUE;
	default:
		return FALSE;
	}
}


BOOL CALLBACK NextStageName(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	char s[150];

	switch( message )
	{
	case WM_COMMAND:
		switch( wp )
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_EDIT1, s, 149);
			if( !strlen(s) )
			{
				MessageBox(hWnd, "ファイル名を入力してください", "警告めっせぇじ", MB_OK);
				return TRUE;
			}
			Otmp->NextStage = (char*)malloc(strlen(s)+strlen("stage\\\\.stg")+1);
			if( !strcmp(s, "NULL") )
			{
				sprintf(Otmp->NextStage, "NULL");
			}
			else
			{
				sprintf(Otmp->NextStage, "stage\\\\%s.stg", s);
			}
			EndDialog(hWnd, 0);
			break;
		}
		return TRUE;
	default:
		return FALSE;
	}
}


////////////////////////////////////////
// ロード画面

int Loading(HWND hWnd)
{
	TCHAR FileName[140];
	TCHAR BmpName[140];
	FILE *fp;

	// 汎用変数
	int j, k, i, n;

	Human.next = NULL;
	StartEnemy.next = NULL;
	StartItem.next = NULL;
	StartBlock.next = NULL;
	StartGoal.next = NULL;

	Enemies = NULL;
	Items = NULL;
	Block = NULL;
	Goal = NULL;

	// 自分用
	strcpy(FileName,"config\\charam.cfg");
	if (!(fp = fopen(FileName, "r")))
	{
		MessageBox(hWnd, "charam.cfg(自分用の設定ファイル)が見つかりません", "エラーめっせーじ", MB_OK);
		return 0;
	}
	while (1)
	{
		TCHAR order[50];

		GetInObj(fp, order);
		if( !lstrcmp(order, "exit-setting") || feof(fp) )
		{
			MessageBox(hWnd, "セッティング終了", "しゅーりょ", MB_OK);
			break;
		}
		else if (!lstrcmp(order, "bmp"))
		{

			GetInObj(fp, order);

			GetInObj(fp, order);
			Human.BPt.x = atoi(order);

			GetInObj(fp, order);
			Human.BPt.y = atoi(order);

			GetInObj(fp, order);
			Human.Sz.cx = atoi(order);

			GetInObj(fp, order);
			Human.Sz.cy = atoi(order);

			GetInObj(fp, order);
			lstrcpy(BmpName, order);

			GetInObj(fp, order);

			if( ReadBmp(hWnd, BmpName, &Human) )
			{
				MessageBox(hWnd, "プログラムは継続できません", "警告めっせーぇじ", MB_OK);
				PostQuitMessage(0);
				return 0;
			}
			break;
		}
	}
	fclose( fp );


	// 敵用
	sprintf(FileName, "config\\charae.cfg");
	if (!(fp = fopen(FileName, "r")))
	{
		MessageBox(hWnd,"charae.cfg(敵用の設定ファイル)が見つかりません。", "警告めっせぇ～じ", MB_OK);;
		return 0;
	}
	while (1)
	{
		char order[50];

		GetInObj(fp, order);

		if (!strcmp(order, "exit-setting") || feof(fp))
		{
			break;
		}
		else if (!strcmp(order, "enemy"))
		{
			GetInObj(fp, order);
			j = atoi(order);

			for ( k=0; k<j; k++)
			{
				GetInObj(fp, order);
				if (!strcmp(order, "film"))
				{
					int ReadFlag = 0;
					if( StartEnemy.next==NULL )
					{
						Otmp = (StartEnemy.next = new Object_c);
					}
					else
					{
						Otmp = (Otmp->next = new Object_c);
					}
					GetInObj(fp, order);
					n = atoi(order);

					// 打破時のポイント
					GetInObj(fp, order);

					// パターン
					GetInObj(fp, order);

					// ヒットポイント
					GetInObj(fp, order);

					for (; n > 0; n--)
					{
						GetInObj(fp, order);
						if (!strcmp(order, "bmp"))
						{
							int num;

							if( !ReadFlag )
							{
								Otmp->No = k+1;

								GetInObj(fp, order);
								num = atoi(order) - 1;

								GetInObj(fp, order);
								Otmp->BPt.x = atoi(order);

								GetInObj(fp, order);
								Otmp->BPt.y = atoi(order);

								GetInObj(fp, order);
								Otmp->Sz.cx = atoi(order);

								GetInObj(fp, order);
								Otmp->Sz.cy = atoi(order);

								GetInObj(fp, order);
								strcpy(BmpName,order);

								GetInObj(fp, order);

								if( ReadBmp(hWnd, BmpName, Otmp) )
								{
									MessageBox(hWnd, "プログラムは継続できません", "警告めっせーぇじ", MB_OK);
									PostQuitMessage(0);
									return 0;
								}
								for (i=0; i<num; i++)
								{
									GetInObj(fp, order);//bx

									GetInObj(fp, order);//by

									GetInObj(fp, order);//cx

									GetInObj(fp, order);//cy

									GetInObj(fp, order);//fname

									GetInObj(fp, order);//ct
								}
								ReadFlag = 1;
							}
							else
							{
								GetInObj(fp, order);
								num = atoi(order);

								for (i=0; i<num; i++)
								{
									GetInObj(fp, order);

									GetInObj(fp, order);

									GetInObj(fp, order);

									GetInObj(fp, order);

									GetInObj(fp, order);

									GetInObj(fp, order);
								}
							}
						}
					}
				}
			}
		}
		break;
	}
	Otmp->next = StartEnemy.next;
	fclose(fp);

	// アイテム
	k = 0;
	sprintf(FileName, "config\\item.cfg");
	if (!(fp=fopen(FileName,"r")))
	{
		MessageBox(hWnd,"item.cfg(アイテム設定ファイル)が読み込めません", "警告めっせぇじ", MB_OK);
		return 1;
	}
	while(1)
	{
		char order[50];

		GetInObj(fp, order);
		if (!strcmp(order, "exit-setting") || feof(fp))
		{
			break;
		}
		if (!strcmp(order, "bmp"))
		{
			k++;
			if( StartItem.next==NULL)
			{
				Otmp = (StartItem.next = new Object_c);
			}
			else
			{
				Otmp = (Otmp->next = new Object_c);
			}

			Otmp->No = k;

			GetInObj(fp, order);
			Otmp->BPt.x = atoi(order);

			GetInObj(fp, order);
			Otmp->BPt.y = atoi(order);

			GetInObj(fp, order);
			Otmp->Sz.cx = atoi(order);

			GetInObj(fp, order);
			Otmp->Sz.cy = atoi(order);

			GetInObj(fp, order);
			strcpy(BmpName, order);

			GetInObj(fp, order);

			if( ReadBmp(hWnd, BmpName, Otmp) )
			{
				MessageBox(hWnd, "プログラムは継続できません", "警告めっせーぇじ", MB_OK);
				PostQuitMessage(0);
				return 0;
			}
		}
	}
	Otmp->next = StartItem.next;
	fclose(fp);

	// ゴール
	Otmp = (StartGoal.next = new Object_c);
	Otmp->Sz.cx = Otmp->Sz.cy = 32;
	Otmp->No = 1;
	ReadBmp(hWnd, "bmp\\treasurebox.bmp", Otmp);

	// ブロック
	Otmp = (StartBlock.next = new Object_c);
	ReadBmp(hWnd, "bmp\\block.bmp", Otmp);
	Otmp->Sz.cx = Otmp->Sz.cy = 32;
	Otmp->No = 2;
	Otmp = (Otmp->next = new Object_c);
	ReadBmp(hWnd, "bmp\\faceblock.bmp", Otmp);
	Otmp->Sz.cx = Otmp->Sz.cy = 32;
	Otmp->No = 1;

	// 消しゴム
	Otmp = (StartEraser.next = new Object_c);
	ReadBmp(hWnd, "bmp\\eraser.bmp", Otmp);
	Otmp->Sz.cx = Otmp->Sz.cy = 32;
	Otmp->No = 1;

	return 1;
}

int SaveStage(FILE *fp)
{
	int k;
	// ステージ曲
	if( *WindowSt->MidiFileName!='\0' )
		fprintf(fp, "music\nmusic\\\\%s\n",WindowSt->MidiFileName);

	// Windouステータス
	fprintf(fp, "screenx\n%d\n",WindowSt->ScreenX);
	fprintf(fp, "screenmax\n%d\n",WindowSt->ScreenMax);
	fprintf(fp, "timer\n%d\n",WindowSt->Timer);

	// 主人公の初期位置
	fprintf(fp, "human\n%d,%d\n",Human.Pt.x,Human.Pt.y);

	// アイテム
	for( k=0, Otmp=Items; Otmp!=NULL; k++,Otmp=Otmp->next )
		;
	fprintf(fp, "item,%d\n",k);
	for( Otmp=Items; Otmp!=NULL; Otmp=Otmp->next )
	{
		fprintf(fp, "%d,%d,%d\n",Otmp->No,Otmp->Pt.x,Otmp->Pt.y);
	}

	// ゴール
	for( k=0, Otmp=Goal; Otmp!=NULL; k++, Otmp=Otmp->next )
		;
	fprintf(fp, "goal,%d\n",k);
	for( Otmp=Goal; Otmp!=NULL; Otmp=Otmp->next )
	{
		fprintf(fp, "%d,%d,%s\n",Otmp->Pt.x,Otmp->Pt.y,Otmp->NextStage);
	}

	// 敵
	for( k=0, Otmp=Enemies; Otmp!=NULL; k++, Otmp=Otmp->next )
		;
	fprintf(fp, "enemy,%d\n",k);
	for( Otmp=Enemies; Otmp!=NULL; Otmp=Otmp->next )
	{
		fprintf(fp, "%d,%d,%d\n",Otmp->No,Otmp->Pt.x,Otmp->Pt.y);
	}

	// ブロック
	for( k=0, Otmp=Block; Otmp!=NULL; k++,Otmp=Otmp->next )
		;
	fprintf(fp, "block,%d\n",k);
	for( Otmp=Block; Otmp!=NULL; Otmp=Otmp->next )
	{
		fprintf(fp, "%d,%d,%d\n",Otmp->No,Otmp->Pt.x,Otmp->Pt.y);
	}

	// 設定終了
	fprintf(fp, "exit-setting\n\n\n");

	return 1;
}

int ShowObject_c(HDC hdc, Object_c *Obtmp)
{
	int k = 0;

	while( Obtmp!=NULL)
	{
		SetDIBitsToDevice(
			hdc , Obtmp->Pt.x-scr.nPos , Obtmp->Pt.y ,
			Obtmp->Sz.cx, Obtmp->Sz.cy,
			Obtmp->BPt.x , Obtmp->bmpInfo.bmiHeader.biHeight-(Obtmp->BPt.y+Obtmp->Sz.cy),
			Obtmp->BPt.x, Obtmp->bmpInfo.bmiHeader.biHeight,
			Obtmp->bPixelBits , &Obtmp->bmpInfo , DIB_RGB_COLORS
		);

		Rectangle(hdc,
			Obtmp->Pt.x-scr.nPos,Obtmp->Pt.y,
			Obtmp->Pt.x-scr.nPos+Obtmp->Sz.cx,
			Obtmp->Pt.y+Obtmp->Sz.cy);

		Obtmp = Obtmp->next;
		k++;
	}
	return k;
}

DWORD GetInObj(FILE *fpin, TCHAR *order)
{
	int k;
	char ch;

	for (k = 0; ; k++)
	{
		ch = fgetc(fpin);
		if (ch == EOF) 
		{
			order[k] = '\0';
			break;
		}
		else if (ch == ',' || ch == '\n')
		{
			order[k] = '\0';
			break;
		}
		else if (ch == ' ')
		{
			k = -1;
			continue;
		}
		else if (ch == ';')
		{
			while (fgetc(fpin)!='\n')
				;
			order[k] = '\0';
			break;
		}
		order[k] = ch;
	}
	return k;
}

int ReadBmp(HWND hWnd, PSTR pstr, Object_c *Obj)
{
	HANDLE hFile;
	DWORD dwBytes;

	hFile = CreateFile(pstr , GENERIC_READ , 0 , NULL ,
		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if (hFile == INVALID_HANDLE_VALUE) return 1;

	ReadFile(hFile , &Obj->bmpFileHeader , sizeof (BITMAPFILEHEADER) , &dwBytes , NULL);
	if (Obj->bmpFileHeader.bfType != 0x4D42) {
		MessageBox(NULL , TEXT("This is not a bitmap file") , NULL , MB_OK);
		return 1;
	}

	ReadFile(hFile , &Obj->bmpInfo , sizeof (BITMAPINFOHEADER) , &dwBytes , NULL);
	Obj->bPixelBits = (BYTE *) malloc (Obj->bmpFileHeader.bfSize - Obj->bmpFileHeader.bfOffBits);
	ReadFile(hFile ,Obj->bPixelBits ,
		Obj->bmpFileHeader.bfSize - Obj->bmpFileHeader.bfOffBits , &dwBytes , NULL);
	CloseHandle(hFile);

	return 0;
}

int OnlyOneCheck(HWND hWnd,int MID)
{
	HMENU hmenu;
	MENUITEMINFO menuInfo;

	hmenu = GetMenu(hWnd);
	menuInfo.cbSize = sizeof( MENUITEMINFO );
	menuInfo.fMask = MIIM_STATE;
	NowID = MID;

	GetMenuItemInfo(hmenu,IDM_HUMAN,FALSE, &menuInfo);
	menuInfo.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,IDM_HUMAN,FALSE, &menuInfo);

	GetMenuItemInfo(hmenu,IDM_ENEMY,FALSE, &menuInfo);
	menuInfo.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,IDM_ENEMY,FALSE, &menuInfo);

	GetMenuItemInfo(hmenu,IDM_ITEM,FALSE, &menuInfo);
	menuInfo.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,IDM_ITEM,FALSE, &menuInfo);

	GetMenuItemInfo(hmenu,IDM_GOAL,FALSE, &menuInfo);
	menuInfo.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,IDM_GOAL,FALSE, &menuInfo);

	GetMenuItemInfo(hmenu,IDM_BLOCK,FALSE, &menuInfo);
	menuInfo.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,IDM_BLOCK,FALSE, &menuInfo);
	
	GetMenuItemInfo(hmenu,IDM_ERASER,FALSE, &menuInfo);
	menuInfo.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hmenu,IDM_ERASER,FALSE, &menuInfo);

	GetMenuItemInfo(hmenu,MID,FALSE, &menuInfo);
	menuInfo.fState = MFS_CHECKED;
	SetMenuItemInfo(hmenu,MID,FALSE, &menuInfo);

	return 0;
}

int DestroyAllObject_c()
{
	Object_c *Otmp1;
	Object_c *Otmp2;
	Human.Pt.x = 0;
	Human.Pt.y = 0;

	// 敵の消去
	Otmp1=Enemies;
	while( Otmp1!=NULL )
	{
		Otmp2 = Otmp1->next;
		free(Otmp1);
		Otmp1 = Otmp2;
	}
	Enemies = NULL;

	// アイテムの消去
	Otmp1=Items;
	while( Otmp1!=NULL )
	{
		Otmp2 = Otmp1->next;
		free(Otmp1);
		Otmp1 = Otmp2;
	}
	Items = NULL;

	// ゴールの消去
	Otmp1=Goal;
	while( Otmp1!=NULL )
	{
		Otmp2 = Otmp1->next;
		free(Otmp1);
		Otmp1 = Otmp2;
	}
	Goal = NULL;

	// ブロックの消去
	Otmp1=Block;
	while( Otmp1!=NULL )
	{
		Otmp2 = Otmp1->next;
		free(Otmp1);
		Otmp1 = Otmp2;
	}
	Block = NULL;

	return 0;
}

int DestroyPtObject_c(POINT Pt)
{
	RECT dstRc;
	Object_c *Otmp;
	Object_c *PrevOtmp;

	// 敵の消去
	Otmp = Enemies;
	while( Otmp!=NULL )
	{
		SetRect(&dstRc, Otmp->Pt.x, Otmp->Pt.y, Otmp->Pt.x+Otmp->Sz.cx, Otmp->Pt.y+Otmp->Sz.cy);
		if( PtInRect(&dstRc,Pt) )
		{
			if( Otmp->prev == NULL )
			{
				Enemies = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = NULL;
			}
			else
			{
				Otmp->prev->next = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = Otmp->prev;
			}
			PrevOtmp = Otmp;
			Otmp = Otmp->next;
			free(PrevOtmp);
			continue;
		}
		Otmp = Otmp->next;
	}


	// アイテムの消去
	Otmp=Items;
	while( Otmp!=NULL )
	{
		SetRect(&dstRc, Otmp->Pt.x, Otmp->Pt.y, Otmp->Pt.x+Otmp->Sz.cx, Otmp->Pt.y+Otmp->Sz.cy);
		if( PtInRect(&dstRc,Pt) )
		{
			if( Otmp->prev == NULL )
			{
				Items = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = NULL;
			}
			else
			{
				Otmp->prev->next = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = Otmp->prev;
			}
			PrevOtmp = Otmp;
			Otmp = Otmp->next;
			free(PrevOtmp);
			continue;
		}
		Otmp = Otmp->next;
	}


	// ゴールの消去
	Otmp=Goal;
	while( Otmp!=NULL )
	{
		SetRect(&dstRc, Otmp->Pt.x, Otmp->Pt.y, Otmp->Pt.x+Otmp->Sz.cx, Otmp->Pt.y+Otmp->Sz.cy);
		if( PtInRect(&dstRc,Pt) )
		{
			if( Otmp->prev == NULL )
			{
				Goal = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = NULL;
			}
			else
			{
				Otmp->prev->next = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = Otmp->prev;
			}
			PrevOtmp = Otmp;
			Otmp = Otmp->next;
			free(PrevOtmp);
			continue;
		}
		Otmp = Otmp->next;
	}


	// ブロックの消去
	Otmp=Block;
	while( Otmp!=NULL )
	{
		SetRect(&dstRc, Otmp->Pt.x, Otmp->Pt.y, Otmp->Pt.x+Otmp->Sz.cx, Otmp->Pt.y+Otmp->Sz.cy);
		if( PtInRect(&dstRc,Pt) )
		{
			if( Otmp->prev == NULL )
			{
				Block = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = NULL;
			}
			else
			{
				Otmp->prev->next = Otmp->next;
				if( Otmp->next != NULL )
					Otmp->next->prev = Otmp->prev;
			}
			PrevOtmp = Otmp;
			Otmp = Otmp->next;
			free(PrevOtmp);
			continue;
		}
		Otmp = Otmp->next;
	}

	return 0;
}