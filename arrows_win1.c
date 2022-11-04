#define APPNAME "Arrows"

// Windows Header Files:
#include <windows.h>

// Local Header Files
#include "arrows_win.h"
#include "resource.h"

// Makes it easier to determine appropriate code paths:
#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

// Global Variables:
HINSTANCE g_hinst;
HANDLE hAccelTable;
char szAppName[] = APPNAME; // The name of this application
char INIFile[] = APPNAME".ini";
char szTitle[] = APPNAME; // The title bar text
int AppXSize=832,AppYSize=668,AppYSizeR,YXRatio=1;
BOOL fExit,debug,doppiaDim;
extern BYTE gameMode,bPaused;
HWND ghWnd,hStatusWnd;
HBRUSH hBrush;
HPEN hPen1;
HFONT hFont,hFont2;
UINT hTimer;
BYTE VideoRAM[HORIZ_CHARS*25];
extern BYTE font8x8[];
BYTE ColorRAM[HORIZ_CHARS*25],VideoHIRAM[HORIZ_CHARS*8/2 * 25*8];		// 1 byte ogni 2 pixel horz
extern COLORREF Colori[16];
extern BYTE Keyboard[1];
HFILE spoolFile;
DWORD ColorQUAD[16];
BYTE coloreCaratteri=FORECOLOR;
BITMAPINFO bmI= {
	{	40,
		HORIZ_CHARS*8,200,
		1,4,BI_RGB,0,
		0,0,16,0
		},
	ColorQUAD
	};


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;

	if(!hPrevInstance) {
		if(!InitApplication(hInstance)) {
			return (FALSE);
		  }
	  }

	if (!InitInstance(hInstance, nCmdShow)) {
		return (FALSE);
  	}

	if(*lpCmdLine) {
		PostMessage(ghWnd,WM_USER+1,0,(LPARAM)lpCmdLine);
		}

	hAccelTable = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
	Arrows(0);

  return (msg.wParam);
	}

int _fastcall PlotChar(int pos,int ch,BYTE c) {
	BYTE *p,*p1;
	register int i,j,k;
	char myBuf[128];
	BYTE backColor=0;

	p=&font8x8;
//	ch -= ' ';
	p += ch << 3;
	p1=&VideoHIRAM[0]+pos;
	for(i=0; i<8; i++) {
		k=*p;
		*p1=k & 0x80 ? (c & 0xf) << 4 : backColor;
		*p1|=k & 0x40 ? (c & 0xf) : backColor;
		p1++;
		*p1=k & 0x20 ? (c & 0xf) << 4 : backColor;
		*p1|=k & 0x10 ? (c & 0xf) : backColor;
		p1++;
		*p1=k & 0x8 ? (c & 0xf) << 4 : backColor;
		*p1|=k & 0x4 ? (c & 0xf) : backColor;
		p1++;
		*p1=k & 0x2 ? (c & 0xf) << 4 : backColor;
		*p1|=k & 0x1 ? (c & 0xf) : backColor;

		p++;
		p1+=HORIZ_CHARS*8/2-3;
		}
//	wsprintf(myBuf,"Colore char: %02x, sfondo: %02x",c & 0xff,VICReg[0x21]);
//				SetWindowText(hStatusWnd,myBuf);
	}

int UpdateScreen(HDC hDC,SWORD rowIni,SWORD rowFin) {
	register int i,j;
	int k,y1,y2,x1,x2;
	register BYTE *p1;
	BYTE *psc,*psc2;

  y1=rowIni/8;
  y2=rowFin/8;
  x1=0;
  x2=HORIZ_CHARS;
	for(i=y1; i<y2; i++) {
		k = HORIZ_OFFSCREEN+ i*HORIZ_CHARS*8 *8/2;
    k += x1*2;

//		k+=HORIZ_CHARS*8 *8/2;

    p1=&VideoRAM[0]+i*HORIZ_CHARS;
    psc=&ColorRAM[0]+i*HORIZ_CHARS;
    for(j=x1; j<x2; j++) {
      PlotChar(k,*p1++,*psc++);
      k+=4;
			}
		}

	i=StretchDIBits(hDC,0,doppiaDim ? ((8+rowIni)*AppYSizeR)/104 : ((8+rowIni)*AppYSizeR)/208,doppiaDim ? AppXSize*2 : AppXSize,
		doppiaDim ? -((rowFin-rowIni)*AppYSizeR)/104 : -((rowFin-rowIni)*AppYSizeR)/208,
		0,rowIni,HORIZ_CHARS*8,rowFin-rowIni,VideoHIRAM,&bmI,DIB_RGB_COLORS,SRCCOPY);
	}


VOID CALLBACK myTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime) {
	int i;
	HDC hDC;

	}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId,wmEvent;
	PAINTSTRUCT ps;
	HDC hDC,hCompDC;
 	POINT pnt;
	HMENU hMenu;
	HRSRC hrsrc;
	HFILE myFile;
 	BOOL bGotHelp;
	int i,j,k,i1,j1,k1;
	long l;
	char myBuf[128];
	char *s,*s1;
	LOGBRUSH br;
	RECT rc;
	SIZE mySize;
	HFONT hOldFont;
	HPEN myPen,hOldPen;
	HBRUSH myBrush,hOldBrush;
	static int TimerCnt;
	HANDLE hBasic;

	switch (message) { 
		case WM_COMMAND:
			wmId    = LOWORD(wParam); // Remember, these are...
			wmEvent = HIWORD(wParam); // ...different for Win32!

			switch (wmId) {
				case ID_APP_ABOUT:
					DialogBox(g_hinst,MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)About);
					break;

				case ID_APP_EXIT:
					PostMessage(hWnd,WM_CLOSE,0,0l);
					break;

				case ID_GIOCO_NUOVAPARTITA:
					gameMode=2;
					break;

				case ID_FILE_OPEN:
					break;

				case ID_FILE_SAVE_AS:
					break;

				case ID_OPZIONI_DEBUG:
					debug=!debug;
					break;

				case ID_OPZIONI_DIMENSIONEDOPPIA:
					doppiaDim=!doppiaDim;
					break;

				case ID_OPZIONI_COLORE_VERDE:
					coloreCaratteri=13;
					break;

				case ID_OPZIONI_COLORE_AMBRA:
					coloreCaratteri=10;
					break;

				case ID_OPZIONI_COLORE_BIANCO:
					coloreCaratteri=1;
					break;

				case ID_OPZIONI_PAUSED:
					bPaused=!bPaused;
					{
						char mybuf[64];
						strcpy(mybuf,szTitle);
						if(bPaused)
							strcat(mybuf," (in pausa)");		// in release non va??
						SetWindowText(hWnd,mybuf);
					}
					break;

				case ID_EDIT_PASTE:
					break;

        case ID_HELP: // Only called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_FINDER,(DWORD)0);
          if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_INDEX: // Not called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_CONTENTS,(DWORD)0);
		      if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_FINDER: // Not called in Windows 95
          if(!WinHelp(hWnd, APPNAME".HLP", HELP_PARTIALKEY,
				 		(DWORD)(LPSTR)"")) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_USING: // Not called in Windows 95
					if(!WinHelp(hWnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName, MB_OK|MB_ICONHAND);
					  }
					break;

				default:
					return (DefWindowProc(hWnd, message, wParam, lParam));
			}
			break;

		case WM_NCRBUTTONUP: // RightClick on windows non-client area...
			if (IS_WIN95 && SendMessage(hWnd, WM_NCHITTEST, 0, lParam) == HTSYSMENU) {
				// The user has clicked the right button on the applications
				// 'System Menu'. Here is where you would alter the default
				// system menu to reflect your application. Notice how the
				// explorer deals with this. For this app, we aren't doing
				// anything
				return (DefWindowProc(hWnd, message, wParam, lParam));
			  }
			else {
				// Nothing we are interested in, allow default handling...
				return (DefWindowProc(hWnd, message, wParam, lParam));
			  }
      break;

      case WM_RBUTTONDOWN: // RightClick in windows client area...
        pnt.x = LOWORD(lParam);
        pnt.y = HIWORD(lParam);
        ClientToScreen(hWnd, (LPPOINT)&pnt);
        hMenu = GetSubMenu(GetMenu(hWnd),2);
        if(hMenu) {
          TrackPopupMenu(hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
          }
        break;

		case WM_PAINT:
			hDC=BeginPaint(hWnd,&ps);
			myPen=CreatePen(PS_SOLID,16,Colori[0]);
			br.lbStyle=BS_SOLID;
			br.lbColor=Colori[0];
			br.lbHatch=0;
			myBrush=CreateBrushIndirect(&br);
			SelectObject(hDC,myPen);
			SelectObject(hDC,myBrush);
//			SelectObject(hDC,hFont);
//			Rectangle(hDC,0,0,200,200);
			Rectangle(hDC,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom);
/*			for(i=0; i<25; i++) {
				for(j=0; j<40; j++) {
					SetTextColor(hDC,RGB(0xff,0xff,0xff));
					if(myBuf[0]=ram_seg[0x400+j+i*25])
					  TextOut(hDC,j*10+5,i*10+5,myBuf,1);
				  }
			  }
				*/
			UpdateScreen(hDC,0,200);
			DeleteObject(myPen);
			DeleteObject(myBrush);
			EndPaint(hWnd,&ps);
			break;        

		case WM_TIMER:
			TimerCnt++;
			break;

		case WM_SIZE:
			GetClientRect(hWnd,&rc);
			AppXSize=rc.right-rc.left;
			AppYSizeR=rc.bottom-rc.top;
			MoveWindow(hStatusWnd,0,rc.bottom-16,rc.right,16,1);
			break;        

		case WM_KEYDOWN:
			decodeKBD(wParam,lParam,1);
			break;        

		case WM_KEYUP:
			decodeKBD(wParam,lParam,0);
			break;        

		case WM_CREATE:
//			bInFront=GetPrivateProfileInt(APPNAME,"SempreInPrimoPiano",0,INIFile);

			for(i=0; i<16; i++) {
				bmI.bmiColors[i].rgbRed=GetRValue(Colori[i]);
				bmI.bmiColors[i].rgbGreen=GetGValue(Colori[i]);
				bmI.bmiColors[i].rgbBlue=GetBValue(Colori[i]);
				}
			hFont=CreateFont(12,6,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN, (LPSTR)"Courier New");
			hFont2=CreateFont(14,7,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"Arial");
			GetClientRect(hWnd,&rc);
			hStatusWnd = CreateWindow("static","",
				WS_BORDER | SS_LEFT | WS_CHILD,
				0,rc.bottom-16,AppXSize-GetSystemMetrics(SM_CXVSCROLL)-2*GetSystemMetrics(SM_CXSIZEFRAME),16,
				hWnd,1001,g_hinst,NULL);
			ShowWindow(hStatusWnd, SW_SHOW);
			GetClientRect(hWnd,&rc);
			AppYSizeR=rc.bottom-rc.top;
			SendMessage(hStatusWnd,WM_SETFONT,(WPARAM)hFont2,0);
			hPen1=CreatePen(PS_SOLID,1,RGB(255,255,255));
			br.lbStyle=BS_SOLID;
			br.lbColor=0x000000;
			br.lbHatch=0;
			hBrush=CreateBrushIndirect(&br);


//			spoolFile=_lcreat("spoolfile.txt",0);
//			hTimer=SetTimer(NULL,0,1000/32,myTimerProc);  // basato su Raster
			// non usato... fa schifo!
			break;

		case WM_QUERYENDSESSION:
			return 1l;
			break;

		case WM_CLOSE:
esciprg:          
		  DestroyWindow(hWnd);
			return 0l;
			break;

		case WM_DESTROY:
//			WritePrivateProfileInt(APPNAME,"SempreInPrimoPiano",bInFront,INIFile);
			// Tell WinHelp we don't need it any more...
			KillTimer(hWnd,hTimer);
//			_lclose(spoolFile);
	    WinHelp(hWnd,APPNAME".HLP",HELP_QUIT,(DWORD)0);
			DeleteObject(hBrush);
			DeleteObject(hPen1);
			DeleteObject(hFont);
			DeleteObject(hFont2);
			PostQuitMessage(0);
			break;

   	case WM_INITMENU:
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_DEBUG,MF_BYCOMMAND | (debug ? MF_CHECKED : MF_UNCHECKED));
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_DIMENSIONEDOPPIA,MF_BYCOMMAND | (doppiaDim ? MF_CHECKED : MF_UNCHECKED));
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_COLORE_BIANCO,MF_BYCOMMAND | (coloreCaratteri==1 ? MF_CHECKED : MF_UNCHECKED));
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_COLORE_VERDE,MF_BYCOMMAND | (coloreCaratteri==13 ? MF_CHECKED : MF_UNCHECKED));
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_COLORE_AMBRA,MF_BYCOMMAND | (coloreCaratteri==10 ? MF_CHECKED : MF_UNCHECKED));
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_PAUSED,MF_BYCOMMAND | (bPaused ? MF_CHECKED : MF_UNCHECKED));
			break;

		default:
			return (DefWindowProc(hWnd, message, wParam, lParam));
		}
	return (0);
	}



ATOM MyRegisterClass(CONST WNDCLASS *lpwc) {
	HANDLE  hMod;
	FARPROC proc;
	WNDCLASSEX wcex;

	hMod=GetModuleHandle("USER32");
	if(hMod != NULL) {

#if defined (UNICODE)
		proc = GetProcAddress (hMod, "RegisterClassExW");
#else
		proc = GetProcAddress (hMod, "RegisterClassExA");
#endif

		if(proc != NULL) {
			wcex.style         = lpwc->style;
			wcex.lpfnWndProc   = lpwc->lpfnWndProc;
			wcex.cbClsExtra    = lpwc->cbClsExtra;
			wcex.cbWndExtra    = lpwc->cbWndExtra;
			wcex.hInstance     = lpwc->hInstance;
			wcex.hIcon         = lpwc->hIcon;
			wcex.hCursor       = lpwc->hCursor;
			wcex.hbrBackground = lpwc->hbrBackground;
    	wcex.lpszMenuName  = lpwc->lpszMenuName;
			wcex.lpszClassName = lpwc->lpszClassName;

			// Added elements for Windows 95:
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.hIconSm = LoadIcon(wcex.hInstance, "SMALL");
			
			return (*proc)(&wcex);//return RegisterClassEx(&wcex);
			}
		}
	return (RegisterClass(lpwc));
	}


BOOL InitApplication(HINSTANCE hInstance) {
  WNDCLASS  wc;
  HWND      hwnd;

  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC)WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APP32));
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));

        // Since Windows95 has a slightly different recommended
        // format for the 'Help' menu, lets put this in the alternate menu like this:
  if(IS_WIN95) {
		wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    } else {
	  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    }
  wc.lpszClassName = szAppName;

  if(IS_WIN95) {
	  if(!MyRegisterClass(&wc))
			return 0;
    }
	else {
	  if(!RegisterClass(&wc))
	  	return 0;
    }


  }

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	
	g_hinst=hInstance;

	ghWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, AppXSize,AppYSize,
		NULL, NULL, hInstance, NULL);

	if(!ghWnd) {
		return (FALSE);
	  }

	ShowWindow(ghWnd, nCmdShow);
	UpdateWindow(ghWnd);

	return (TRUE);
  }

//
//  FUNCTION: About(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "About" dialog box
// 		This version allows greater flexibility over the contents of the 'About' box,
// 		by pulling out values from the 'Version' resource.
//
//  MESSAGES:
//
//	WM_INITDIALOG - initialize dialog box
//	WM_COMMAND    - Input received
//
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static  HFONT hfontDlg;		// Font for dialog text
	static	HFONT hFinePrint;	// Font for 'fine print' in dialog
	DWORD   dwVerInfoSize;		// Size of version information block
	LPSTR   lpVersion;			// String pointer to 'version' text
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	UINT    uVersionLen;
	WORD    wRootLen;
	BOOL    bRetCode;
	int     i;
	char    szFullPath[256];
	char    szResult[256];
	char    szGetName[256];
	DWORD	dwVersion;
	char	szVersion[40];
	DWORD	dwResult;

	switch (message) {
    case WM_INITDIALOG:
//			ShowWindow(hDlg, SW_HIDE);
			hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
			hFinePrint = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
//			CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
			GetModuleFileName(g_hinst, szFullPath, sizeof(szFullPath));

			// Now lets dive in and pull out the version information:
			dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
			if(dwVerInfoSize) {
				LPSTR   lpstrVffInfo;
				HANDLE  hMem;
				hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
				lpstrVffInfo  = GlobalLock(hMem);
				GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
				// The below 'hex' value looks a little confusing, but
				// essentially what it is, is the hexidecimal representation
				// of a couple different values that represent the language
				// and character set that we are wanting string values for.
				// 040904E4 is a very common one, because it means:
				//   US English, Windows MultiLingual characterset
				// Or to pull it all apart:
				// 04------        = SUBLANG_ENGLISH_USA
				// --09----        = LANG_ENGLISH
				// ----04E4 = 1252 = Codepage for Windows:Multilingual
				lstrcpy(szGetName, "\\StringFileInfo\\040904E4\\");	 
				wRootLen = lstrlen(szGetName); // Save this position
			
				// Set the title of the dialog:
				lstrcat (szGetName, "ProductName");
				bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
					(LPSTR)szGetName,
					(LPVOID)&lpVersion,
					(UINT *)&uVersionLen);
//				lstrcpy(szResult, "About ");
//				lstrcat(szResult, lpVersion);
//				SetWindowText (hDlg, szResult);

				// Walk through the dialog items that we want to replace:
				for (i = DLG_VERFIRST; i <= DLG_VERLAST; i++) {
					GetDlgItemText(hDlg, i, szResult, sizeof(szResult));
					szGetName[wRootLen] = (char)0;
					lstrcat (szGetName, szResult);
					uVersionLen   = 0;
					lpVersion     = NULL;
					bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
						(LPSTR)szGetName,
						(LPVOID)&lpVersion,
						(UINT *)&uVersionLen);

					if(bRetCode && uVersionLen && lpVersion) {
					// Replace dialog item text with version info
						lstrcpy(szResult, lpVersion);
						SetDlgItemText(hDlg, i, szResult);
					  }
					else {
						dwResult = GetLastError();
						wsprintf (szResult, "Error %lu", dwResult);
						SetDlgItemText (hDlg, i, szResult);
					  }
					SendMessage (GetDlgItem (hDlg, i), WM_SETFONT, 
						(UINT)((i==DLG_VERLAST)?hFinePrint:hfontDlg),TRUE);
				  } // for


				GlobalUnlock(hMem);
				GlobalFree(hMem);

			}
		else {
				// No version information available.
			} // if (dwVerInfoSize)

    SendMessage(GetDlgItem (hDlg, IDC_LABEL), WM_SETFONT,
			(WPARAM)hfontDlg,(LPARAM)TRUE);

			// We are  using GetVersion rather then GetVersionEx
			// because earlier versions of Windows NT and Win32s
			// didn't include GetVersionEx:
			dwVersion = GetVersion();

			if (dwVersion < 0x80000000) {
				// Windows NT
				wsprintf (szVersion, "Microsoft Windows NT %u.%u (Build: %u)",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
          (DWORD)(HIWORD(dwVersion)) );
				}
			else
				if (LOBYTE(LOWORD(dwVersion))<4) {
					// Win32s
				wsprintf (szVersion, "Microsoft Win32s %u.%u (Build: %u)",
  				(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
					(DWORD)(HIWORD(dwVersion) & ~0x8000) );
				}
			else {
					// Windows 95
				wsprintf(szVersion,"Microsoft Windows 95 %u.%u",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))) );
				}

			SetWindowText(GetDlgItem(hDlg, IDC_OSVERSION), szVersion);
//			SetWindowPos(hDlg,NULL,GetSystemMetrics(SM_CXSCREEN)/2,GetSystemMetrics(SM_CYSCREEN)/2,0,0,SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
//			ShowWindow(hDlg, SW_SHOW);
			return (TRUE);

		case WM_COMMAND:
			if(wParam==IDOK || wParam==IDCANCEL) {
  		  EndDialog(hDlg,0);
			  return (TRUE);
			  }
			else if(wParam==3) {
				MessageBox(hDlg,"Se trovate utile questo programma, mandate un contributo!!\nVia Rivalta 39 - 10141 Torino (Italia)\n[Dario Greggio]","ADPM Synthesis sas",MB_OK);
			  return (TRUE);
			  }
			break;
		}

	return FALSE;
	}


int	decodeKBD(WPARAM wParam,LPARAM lParam,int mode) {

	if(mode) {
		switch(wParam) {
			case VK_NUMPAD0:
			case '0':
				Keyboard[0]='0';
				break;
			case VK_NUMPAD1:
			case '1':
				Keyboard[0]='1';
				break;
			case VK_NUMPAD2:
			case '2':
				Keyboard[0]='2';
				break;
			case VK_NUMPAD3:
			case '3':
				Keyboard[0]='3';
				break;
			case VK_NUMPAD4:
			case '4':
				Keyboard[0]='4';
				break;
			case VK_NUMPAD5:
			case '5':
				Keyboard[0]='5';
				break;
			case VK_NUMPAD6:
			case '6':
				Keyboard[0]='6';
				break;
			case VK_NUMPAD7:
			case '7':
				Keyboard[0]='7';
				break;
			case VK_NUMPAD8:
			case '8':
				Keyboard[0]='8';
				break;
			case VK_NUMPAD9:
			case '9':
				Keyboard[0]='9';
				break;
			case VK_HOME:
				break;
			case VK_DOWN:
				Keyboard[0]='2';
				break;
			case VK_RIGHT:
				Keyboard[0]='6';
				break;
			case VK_UP:
				Keyboard[0]='8';
				break;
			case VK_LEFT:
				Keyboard[0]='4';
				break;
			case VK_ESCAPE:
				Keyboard[0]=0x1b;
				break;
			case VK_SPACE:
				Keyboard[0]=' ';
				break;
			case VK_RETURN:
				Keyboard[0]=13;
				break;
			}
		}
	else {
		Keyboard[0]=0;
		}
	}


int WritePrivateProfileInt(char *s, char *s1, int n, char *f) {
  int i;
  char myBuf[16];
  
  wsprintf(myBuf,"%d",n);
  WritePrivateProfileString(s,s1,myBuf,f);
  }

int ShowMe() {
	int i;
	char buffer[16];

	buffer[0]='A'^ 0x17;
	buffer[1]='D'^ 0x17;
	buffer[2]='P'^ 0x17;
	buffer[3]='M'^ 0x17;
	buffer[4]='-'^ 0x17;
	buffer[5]='G'^ 0x17;
	buffer[6]='.'^ 0x17;
	buffer[7]='D'^ 0x17;
	buffer[8]='a'^ 0x17;
	buffer[9]='r'^ 0x17;
	buffer[10]=' '^ 0x17;
	buffer[11]='2'^ 0x17;
	buffer[12]='0' ^ 0x17;
	buffer[13]=0;
	for(i=0; i<13; i++) buffer[i]^=0x17;
	MessageBox(GetDesktopWindow(),buffer,APPNAME,MB_OK | MB_ICONEXCLAMATION);
	}


