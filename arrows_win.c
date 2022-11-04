#include <windows.h>
#include <ctype.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include "arrows_win.h"

#ifndef Z80
#define SCR_W HORIZ_CHARS
#define SCR_H 25
#define UP_CHR '^'
#define DN_CHR 'V'
#define LF_CHR '<'
#define RG_CHR '>'
#define BOX_H (SCR_H / 8)
#define BOX_W (SCR_W / 8)
#define BOX_TIME 4000
#define SEC_TIME 1000		///CLOCKS_PER_SEC
#define MOVE_TIME 250
#else
#define SCR_W 40
#define SCR_H 4
#define BOX_H 1
#define BOX_W 5
#define UP_CHR '^'
#define DN_CHR 'V'
#define LF_CHR '<'
#define RG_CHR '>'
#define BOX_TIME 80
#define MOVE_TIME 10
#define SEC_TIME 25
#endif


extern BYTE VideoRAM[HORIZ_CHARS*25];
extern BYTE ColorRAM[HORIZ_CHARS*25];
extern BYTE coloreCaratteri;
COLORREF Colori[16]={
	RGB(0,0,0),						 // nero
	RGB(0xff,0xff,0xff),	 // bianco
	RGB(0x80,0x00,0x00),	 // rosso
	RGB(0x00,0x80,0x80),	 // azzurro
	RGB(0x80,0x00,0x80),	 // porpora
	RGB(0x00,0x80,0x00),	 // verde
	RGB(0x00,0x00,0x80),	 // blu
	RGB(0x80,0x80,0x00),	 // giallo
	
	RGB(0xff,0x80,0x40),	 // arancio
	RGB(0x80,0x40,0x40),	 // marrone
	RGB(0xff,0x80,0x80),	 // rosso chiaro
	RGB(0x40,0x40,0x40),	 // grigio scuro
	RGB(0x80,0x80,0x80),	 // grigio medio
	RGB(0x80,0xff,0x80),	 // verde chiaro
	RGB(0x80,0x80,0xff),	 // blu chiaro
	RGB(0xc0,0xc0,0xc0)		 // grigio chiaro
	};
SWORD VICRaster=MIN_RASTER;
BYTE Keyboard[8];
BYTE gameMode,bPaused;
enum GAME_STATE {
	GAME_SPLASH,
	GAME_INIT,
	GAME_START_PLAY,
	GAME_PLAYING,
	GAME_ENDING,
	GAME_END
	};


char xPos,yPos;
char Dir,mChr;
struct LPOS {
	BYTE X,Y;
	char c;
	} lPos[20];
char lLen;
char Bx,By,Bp,Bok,B2erase;
char LCDX,LCDY;
char cursorMode;

#ifndef Z80
#define home() cursor(1,1)

void cursOn() {

	cursorMode=1;
	}

void cursOff() {

	cursorMode=0;
	}

int BlankSc(int tlrow,int tlcol,int brrow,int brcol) {
	int x,y;
	
	if (tlrow < 1) tlrow = 1;
	if (tlrow > SCR_H) tlrow = SCR_H;
	if (tlcol < 1) tlcol = 1;
	if (tlcol > SCR_W) tlcol = SCR_W;
	if (brrow < 1) brrow = 1;
	if (brrow > SCR_H) brrow = SCR_H;
	if (brcol < 1) brcol = 1;
	if (brcol > SCR_W) brcol = SCR_W;

	for(y=tlcol; y<brcol; y++) {
		for(x=tlrow; x<brrow; x++) {
			VideoRAM[(y-1)*SCR_H+x-1]=' ';
			ColorRAM[(y-1)*SCR_H+x-1]=coloreCaratteri;
			}
		}

	}

void cls() {

	memset(VideoRAM,' ',HORIZ_CHARS*25);
	memset(ColorRAM,coloreCaratteri /*verde chiaro*/,HORIZ_CHARS*25);
	LCDX=LCDY=1;
	}

void cursor(char col, char row) {

	LCDY=row;
	LCDX=col;

	if(LCDY < 1) LCDY = 1;
	if(LCDY > SCR_H) LCDY = SCR_H;
	if(LCDX < 1) LCDX = 1;
	if(LCDX > SCR_W) LCDX = SCR_W;

  }

#endif  

void center(char *);
void LCDputch(char);
void LCDputs(char *);
int LCDkbhit(void);
int LCDgetch(void);
void PtBox(char, char, char, unsigned char);
void PlotBruco(void);


int Arrows() {
	static int VICCounter=0;
	static int timer1divider;
	static int c;
	MSG msg;
	BOOL bMsgAvail;
	HDC hDC;
  char i,Exit,ch,mLen,Tim;
  int MoveTime;
  int Punti;
  clock_t t,t1,t2;
  char x,y;
	char mybuf[32];


	memset(VideoRAM,' ',HORIZ_CHARS*25);
	memset(ColorRAM,coloreCaratteri /*verde chiaro*/,HORIZ_CHARS*25);

/*	VideoRAM[0]='!'; VideoRAM[1]='A'; 
	VideoRAM[80]='B'; VideoRAM[81]='5'; VideoRAM[101]='6';VideoRAM[111]='7';VideoRAM[121]='8';
	VideoRAM[320]='!'; VideoRAM[321]='A'; 
	VideoRAM[480]='B'; VideoRAM[481]='5';
	VideoRAM[960]='H'; VideoRAM[961]='I';
	VideoRAM[80*24]='Z';
	for(c=0; c<25; c++) {
		VideoRAM[80*c]=(c % 10) + '0';
		} */

	gameMode=GAME_SPLASH;
	bPaused=0;

	while(TRUE) {

		c++;
		if(!(c & 0x3fff)) {
			bMsgAvail=PeekMessage(&msg,NULL,0,0,PM_REMOVE /*| PM_NOYIELD*/);

			if(bMsgAvail) {
				if(msg.message == WM_QUIT)
		  		break;
				if(!TranslateAccelerator(msg.hwnd,hAccelTable,&msg)) {
					TranslateMessage(&msg); 	 /* Translates virtual key codes			 */
					DispatchMessage(&msg);		 /* Dispatches message to window			 */
					}
				}
  		}

/*		if(DoReset) {
			DoReset=0;
			}
			*/

		VICCounter++;
		if(!(VICCounter & 0x7fff)) {		// con 3ffff fa circa 1/sec... 28/2/20

			VICRaster+=8;					 	 // raster pos count, 200 al sec...
			if(VICRaster >= MAX_RASTER)		 // 
				VICRaster=MIN_RASTER;
			hDC=GetDC(ghWnd);
			UpdateScreen(hDC,VICRaster,VICRaster+8);
			ReleaseDC(ghWnd,hDC);
			}



		switch(gameMode) {
			case GAME_SPLASH:
				B2erase=0;
				cursOff();
			//rifo:
				cls();
//				center("start");
				for(i=0; i<SCR_H; i++)
					LCDputch('\n');
				cursor(0,10);
				center("ARROWS");
				cursor(0,12);
				center("(C) G.Dar 1994-2020");
				t=clock()+(SEC_TIME*5);
				gameMode++;
				break;

			case GAME_INIT:
/*				{				char myBuf[128];
				wsprintf(myBuf,"t, clock %u: %u",t,clock());
				SetWindowText(hStatusWnd,myBuf);
					VideoRAM[0]=clock() & 0x7f;
				}*/

				if(t > clock()) {
					if(LCDkbhit()) {
						if(LCDgetch() == '\x1b') {
							cursOn();
							exit(4);
							}
						else  
							gameMode++;
						}  
					}
				else
					gameMode=GAME_SPLASH;
				break;

			case GAME_START_PLAY:

//start:
				Tim=90;
				Punti=0;
				Exit=0;
				lLen=4;
				srand(/*GetTickCount()*/ clock() /* mi sa che vale 0 a ogni boot */ /* 0 per test! */);

start2:
				MoveTime=MOVE_TIME;
				Bx=By=0;
				Bok=0;
				B2erase=0;
				cls();
//							LCDY=2;
//							center("INIZIO!");
				cursor(SCR_W/2-5,1);
				sprintf(mybuf,"%2d - %3u00",Tim,Punti);
				LCDputs(mybuf);


				mLen=0;
				xPos=SCR_W/2;
				yPos=SCR_H/2;
				Dir=6;
				mChr=RG_CHR;
				t=t2=clock();
				t1=clock()+BOX_TIME;
				t2=clock()+SEC_TIME;
				gameMode=GAME_PLAYING;
				break;

			case GAME_PLAYING:
				if(Exit || !Tim) 
					gameMode=GAME_ENDING;
				if(bPaused)
					break;

				if(LCDkbhit()) {
					ch=LCDgetch();
					switch(ch) {
						case '\x1b':
							Exit=1;
							break;
						case '2':
							Dir=2;
							mChr=DN_CHR;
							break;
						case '4':
							Dir=4;
							mChr=LF_CHR;
							break;
						case '6':
							Dir=6;
							mChr=RG_CHR;
							break;
						case '8':
							Dir=8;
							mChr=UP_CHR;
							break;
							
						}
					}
				if(clock() >= t) {
					lPos[mLen].X=xPos;  
					lPos[mLen].Y=yPos;  
					cursor(xPos,yPos);
					LCDputch(mChr);
					lPos[mLen].c=mChr;  
					t=clock()+MoveTime;
					switch(Dir) {
						case 2:
							if(yPos<(SCR_H))
								yPos++;
							break;
						case 4:
							if(xPos>0)
								xPos--;
							break;
						case 6:
							if(xPos<(SCR_W))
								xPos++;
							break;
						case 8:
							if(yPos>2)
								yPos--;
							break;
						}
					for(i=0; i<lLen; i++) {
						if(lPos[i].X==xPos && lPos[i].Y==yPos) {
error:			  
							MessageBeep(MB_ICONERROR);
							if(lLen < 20)
								lLen+=2;
							goto start2;  
							}
						}
					if(xPos>SCR_W || yPos>SCR_H || xPos<1 || yPos<2) {   // ERRORE negli OR su OpenC/Z80
						goto error;
						}

					if(B2erase) {
						PtBox(0,Bx,By,0);
						Bx=By=0;
						B2erase=0;
						}

					if(mLen<lLen)
						mLen++;
					else {  
						cursor(lPos[0].X,lPos[0].Y);
						LCDputch(' ');
		/*				for(i=1; i<=lLen; i++) {
							lPosX[i-1]=lPosX[i];
							lPosY[i-1]=lPosY[i];
							}*/
						memmove(&lPos[0],&lPos[1],lLen*sizeof(struct LPOS));  
						}  

					if(Bok) {
						if(xPos>=Bx && yPos>=By && xPos<(Bx+BOX_W) && yPos<(By+BOX_H)) {
							MessageBeep(MB_ICONASTERISK);
							MessageBeep(MB_ICONASTERISK);
							PtBox(-1,Bx,By,0);

							PlotBruco();

							B2erase=1;
							Bok=0;
							Punti+=Bp;
							}
						}
					
					}  
				if(clock() >= t1) {
					i=(rand() & 3) +1;
					t1=clock() + (i*(BOX_TIME/3));
					if(Bok || B2erase) {
						PtBox(0,Bx,By,0);

							PlotBruco();

					}
					x=1+(rand() % (SCR_W-5));
					y=2+(rand() % (SCR_H-3-1));
					PtBox(1,x,y,rand());
					Bx=x;
					By=y;
					Bok=1;
					}
				if(clock() >= t2) {
#ifndef Z80		
					MoveTime-=2;
#else
					if(!(Tim & 7))
						MoveTime--;
#endif		  
					t2=clock()+SEC_TIME;
					Tim--;
					cursor(SCR_W/2-5,1);
					sprintf(mybuf,"%2d - %3u00",Tim,Punti);
					LCDputs(mybuf);
					}


				break;

			case GAME_ENDING:
				cls();
				MessageBeep(MB_ICONWARNING);
				cursor(SCR_W/2-5,1);
				sprintf(mybuf,"%2d - %3u00",Tim,Punti);
				LCDputs(mybuf);
				cursor(0,SCR_H-2);
				center("GAME OVER");
				t=clock()+(SEC_TIME*3);
				gameMode++;
				break;

			case GAME_END:
				if(t < clock()) 
					gameMode=GAME_SPLASH;
				break;

			}		// gameMode

		}		// while(1)


	}



  
void PtBox(char m, char x, char y, unsigned char v) {
  char i,j;
	char mybuf[32];
  
  if(x >= SCR_W-BOX_W)		// il controllo messo qua serve a poco... perché poi non funzia il check coordinate sopra! (v.)
    x=SCR_W-BOX_W-1;
  if(y >= SCR_H-BOX_H)
    y=SCR_H-BOX_H-1;

  switch(m) {
		default:
//			k=BOX_H >> 1;
	//	  if(k == 0)
	//	    k=1;  
			for(i=0; i<BOX_H; i++) {
				for(j=0; j<BOX_W; j++) {
					cursor(j+x,i+y);
					LCDputch('*');
					}
				}  
			cursor(x+(BOX_W/2-1),y+(BOX_H >> 1));
			v %= 10;
			if(v) {
				sprintf(mybuf,"%1u00",v);
				LCDputs(mybuf);
				Bp=v;
				}
			else  { 
				sprintf(mybuf,"???");
				LCDputs(mybuf);
				Bp=(rand() % 19)-9;
				if(!Bp)
					Bp++;
				}
			break;

		case 0:
			for(i=0; i<BOX_H; i++) {
				for(j=0; j<BOX_W; j++) {
					cursor(j+x,i+y);
					LCDputch(' ');
					}
				}  
			break;

		case -1:
			for(i=0; i<BOX_H; i++) {
				for(j=0; j<BOX_W; j++) {
					cursor(j+x,i+y);
					LCDputch('=');
					}
				}  
			// se era ???, scrivere punti ora! (o farlo cmq?)
			cursor(x+(BOX_W/2-1),y+(BOX_H >> 1));
			sprintf(mybuf,"%1u00",Bp);
			LCDputs(mybuf);
			break;
	  }    
  }
  
void PlotBruco() {
	int i;

	for(i=0; i<lLen; i++) {
		cursor(lPos[i].X,lPos[i].Y);
		LCDputch(lPos[i].c);
		}
	}

void LCDputch(char ch) {

	switch(ch) {
		default:
			VideoRAM[(LCDY-1)*SCR_W+(LCDX-1)]=ch;
			LCDX++;
			if(LCDX>SCR_W) {
				LCDX=1;
				LCDY++;
				}
			break;
		case '\n':
			if(LCDY<SCR_H) {
				LCDY++;
				LCDX=1;
				}
			break;
		case '\r':
			LCDX=1;
			break;
		case 7:
			MessageBeep(MB_ICONASTERISK);
			break;
		}

	}

void LCDputs(char *s) {

	while(*s) 
		LCDputch(*s++);

	}

void center(char *s) {
  register unsigned char i;

  LCDX=1;
  i=SCR_W-strlen(s);
  i >>= 1;
  while(i--)
    LCDputch(' ');
  LCDputs(s);
  }
  
int LCDgetch() {

	return Keyboard[0];
	}

int LCDkbhit() {

	return Keyboard[0] != 0;
	}


