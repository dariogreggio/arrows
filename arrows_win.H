//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#define STRICT

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#ifndef WIN32
#include <shell.h>
#endif

#include "afxres.h"
#include "resource.h"

#define HORIZ_CHARS 40
#define MIN_RASTER 0
#define MAX_RASTER 25*8
#define HORIZ_SIZE HORIZ_CHARS*8
#define VERT_SIZE 25*8
#define HORIZ_OFFSCREEN 0
#define VERT_OFFSCREEN 0
//#define REAL_SIZE    
#define FORECOLOR 13 //verde chiaro

typedef unsigned short int SWORD;

extern HWND ghWnd,hStatusWnd;
extern HINSTANCE g_hinst;
extern HANDLE hAccelTable;

ATOM MyRegisterClass(CONST WNDCLASS*);
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
int decodeKBD(int, long, BOOL);
int UpdateScreen(HDC,SWORD,SWORD);
int Arrows(int);
BOOL CenterWindow(HWND, HWND);
int WritePrivateProfileInt(char *,char *,int,char *);
