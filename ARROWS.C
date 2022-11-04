#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#ifndef Z80
#include <dos.h>
#endif

#ifndef Z80
#define INT_VID 0x10
#define VID_SCUP 0x06
#define VID_SCDN 0x07
#define VID_SETP 0x05
#define VID_SETC 0x02
#define V_DISP 0x07
#define SCR_W 80
#define SCR_H 25
#define UP_CHR '^'
#define DN_CHR 'V'
#define LF_CHR '<'
#define RG_CHR '>'
#define BOX_H (SCR_H / 8)
#define BOX_W (SCR_W / 8)
#define BOX_TIME 4000
#define SEC_TIME 1000
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


char xPos,yPos;
char Dir,mChr;
char lPosX[20],lPosY[20];
char lLen;
char Bx,By,Bp,Bok;

#ifndef Z80
#define home() cursor(1,1)

void cursOn() {
	union REGS ir;

	ir.h.ah=1;
	ir.h.ch=2;
	ir.h.cl=7;
	int86(INT_VID,&ir,&ir);
	}

void cursOff() {
	union REGS ir;

	ir.h.ah=1;
	ir.h.ch=8;
	ir.h.cl=0;
	int86(INT_VID,&ir,&ir);
	}

int BlankSc(int tlrow,int tlcol,int brrow,int brcol) {
	union REGS ir;
	
	if (tlrow < 1) tlrow = 1;
	if (tlrow > SCR_H) tlrow==SCR_H;
	if (tlcol < 1) tlcol = 1;
	if (tlcol > SCR_W) tlcol = SCR_W;
	if (brrow < 1) brrow = 1;
	if (brrow > SCR_H) brrow = SCR_H;
	if (brcol < 1) brcol = 1;
	if (brcol > SCR_W) brcol = SCR_W;

	ir.h.ah = VID_SCUP;
	ir.h.al = 0;
	ir.h.ch = tlrow -1;
	ir.h.cl = tlcol -1;
	ir.h.dh = brrow -1;
	ir.h.dl = brcol -1;
	ir.h.bh = V_DISP;

	int86(INT_VID, &ir, &ir);

	}

cls()
{
	BlankSc(1,1,SCR_H,SCR_W);
	home();
	}

cursor(char col, char row) {
	union REGS ir;

	if (row < 1) row = 1;
	if (row > SCR_H) row = SCR_H;
	if (col < 1) col = 1;
	if (col > SCR_W) col=SCR_W;

	ir.h.ah = VID_SETC;
	ir.h.bh = 0;
	ir.h.dh = row -1;
	ir.h.dl = col -1;

	int86(INT_VID, &ir, &ir);

  }

#endif  
  
PtBox(char m, char x, char y, unsigned char v) {
  char i,j,k;
  
  if(x >= SCR_W-BOX_W)
    x=SCR_W-BOX_W-1;
  if(y >= SCR_H-BOX_H)
    y=SCR_H-BOX_H-1;
  if(m) {
	  k=BOX_H >> 1;
//	  if(k == 0)
//	    k=1;  
	  for(i=0; i<BOX_H; i++) {
	    for(j=0; j<BOX_W; j++) {
	      cursor(j+x,i+y);
	      putch('*');
	      }
	    }  
	  cursor(x+(BOX_W/2-1),y+k);
	  v %= 10;
	  if(v) {
	    printf("%1d00",v);
	    Bp=v;
	    }
	  else  { 
	    printf("???");
	    Bp=(rand() % 19)-9;
	    if(!Bp)
	      Bp++;
	    }
	  }
	else {
	  for(i=0; i<BOX_H; i++) {
	    for(j=0; j<BOX_W; j++) {
	      cursor(j+x,i+y);
	      putch(' ');
	      }
	    }  
	  }    
  }
  
center(char *s) {
  register unsigned char i;
  
  i=SCR_W-strlen(s);
  i >>= 1;
  while(i--)
    putch(' ');
  puts(s);
  }
  
main() {
  char i,Exit,ch,mLen,Tim;
  int MoveTime;
  int Punti;
  clock_t t,t1,t2;
  char x,y;

  cursOff();
rifo:
  for(i=0; i<SCR_H; i++)
    putch('\n');
  center("ARROWS");
  center("(C) G.Dar 1994");
  t=clock()+(SEC_TIME*5);
  while(t >= clock()) {
	  if(kbhit()) {
	    if(getch() == '\x1b') {
				cursOn();
	      exit(4);
	      }
	    else  
	      goto start;
	    }  
    }
  goto rifo;  

start:
  Tim=90;
  Punti=0;
  Exit=0;
  lLen=4;
  srand(0);
start2:
  MoveTime=MOVE_TIME;
  Bx=By=0;
  cls();
  mLen=0;
  xPos=SCR_W/2;
  yPos=SCR_H/2;
  Dir=6;
  mChr=RG_CHR;
  t=t2=clock();
  t1=clock()+BOX_TIME;
  t2=clock()+SEC_TIME;
	while(!Exit && Tim) {
		if(kbhit()) {
		  ch=getch();
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
			lPosX[mLen]=xPos;  
			lPosY[mLen]=yPos;  
			cursor(xPos,yPos);
			putch(mChr);
		  t=clock()+MoveTime;
		  switch(Dir) {
		    case 2:
		      if(yPos<(SCR_H-1))
		        yPos++;
		      break;
		    case 4:
		      if(xPos>0)
		        xPos--;
		      break;
		    case 6:
		      if(xPos<(SCR_W-1))
		        xPos++;
		      break;
		    case 8:
		      if(yPos>0)
		        yPos--;
		      break;
		    }
			for(i=0; i<lLen; i++) {
			  if(lPosX[i]==xPos && lPosY[i]==yPos) {
error:			  
			    putch(7);
			    if(lLen < 20)
			      lLen+=2;
			    goto start2;  
			    }
			  }
		  if(xPos>=SCR_W || yPos>=SCR_H || xPos<0 || yPos<0) {   // ERRORE negli OR
		    goto error;
		    }
			if(Bok) {
			  if(xPos>=Bx && yPos>=By && xPos<(Bx+BOX_W) && yPos<(By+BOX_H)) {
			    putch(7);
			    putch(7);
				  PtBox(0,Bx,By,0);
				  Bok=Bx=By=0;
				  Punti+=Bp;
				  }
			  }
			
			if(mLen<lLen)
			  mLen++;
			else {  
				cursor(lPosX[0],lPosY[0]);
				putch(' ');
/*				for(i=1; i<=lLen; i++) {
				  lPosX[i-1]=lPosX[i];
				  lPosY[i-1]=lPosY[i];
				  }*/
				memmove(lPosX,lPosX+1,lLen);  
				memmove(lPosY,lPosY+1,lLen);  
				}  
		  }  
		if(clock() >= t1) {
		  i=(rand() & 3) +1;
		  t1=clock() + (i*(BOX_TIME/3));
		  if(Bok)
			  PtBox(0,Bx,By,0);
		  x=rand() % SCR_W;
		  y=rand() % SCR_H;
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
		  cursor(SCR_W/2-5,0);
		  printf("%2d - %3d00",Tim,Punti);
		  }
	  }
  cls();
  cursor(SCR_W/2-5,0);
  printf("%2d - %3d00",Tim,Punti);
  cursor(0,SCR_H-1);
  center("GAME OVER");
  t=clock()+(SEC_TIME*3);
  while(t>=clock());
	goto rifo;  
  }
  