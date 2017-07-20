/***********************************************************************************
*                   XMSCurses Library 
* filename : axmscurses.cpp
* date : 2007.09.05
* Author : Jerry.Wang
* Version : 2.0
* Function List:
* init_window : init the window framework
* draw_main_all : draw all lines in the screen
*
*
***********************************************************************************/

#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


#include "DJAcsDataDef.h"
#include "DJAcsAPIDef.h"

#include "XMS_Sub.h"
#include "XMS_Event.h"
#include "XMS_String.h"
#include "axmscurses.h"

#define C_BLACK 100
#define C_GREEN 200
#define C_RED   300
#define C_CYAN  400
#define C_WHITE 500
#define C_MAGENTA 600
#define C_BLUE  700
#define C_YELLOW 800


int linehigh=3;
WINDOW* g_title;
WINDOW* g_bottom;
WINDOW* g_winhead;
WINDOW* g_win[MAXWIN][MAXCOL];

WINDOW* g_setuphead;
WINDOW* g_setupstatic[MAXWIN];
WINDOW* g_setupdata[MAXWIN];
char   g_buf[MAXWIN][MAXCOL][20];

extern int trknum;

extern TYPE_XMS_DSP_DEVICE_RES_DEMO	AllDeviceRes[MAX_DSP_MODULE_NUMBER_OF_XMS];
extern TYPE_CHANNEL_MAP_TABLE	MapTable_Trunk[MAX_TRUNK_NUM_IN_THIS_DEMO];
extern int g_getdev;
extern int g_ymove;

//for test only
void init_buffer(int trknum,int colnum,int linelen)
{
	memset(&g_buf[0],0,sizeof(g_buf));

	for(int i=0;i<trknum;i++)
	{
		for( int j = 0 ; j< colnum ; j++)
		{
			switch(j)
			{
			case 0:
			sprintf(g_buf[i][j],"%-4d",i);break;
			case 1:
			sprintf(g_buf[i][j],"5-9-%-3d",i);break;
			case 2:
			sprintf(g_buf[i][j],"S_FREE");break;
			case 3:
			sprintf(g_buf[i][j],"TRK_FREE");break;
			case 4:
			sprintf(g_buf[i][j],"12345");break;
			case 5:
			sprintf(g_buf[i][j],"67890");break;
			case 6:
			sprintf(g_buf[i][j],"*#ABCD");break;
			case 7:
			sprintf(g_buf[i][j],"(2,%-3d)",i);break;
			//sprintf(g_buf[i][j],"%-3d  5-9-%-3d   S_FREE   TRK_FREE   12345   67890         (2,%-3d)",i,i,i);
			}
		}
	}
}



//draw a specified col in line with info...
//windownum: the max line in the current window
//lineid   : the trkid in the XMS system
//col      : the column id in the main window of XMS system
//info     : the displayed content
//ymove    : the scrolled number ... 
void draw_main(int windownum,int lineid,int col,const char* info,int ymove)
{
	if( lineid<0 || lineid > MAXWIN)
		return;
	if(! ( (lineid-ymove) >= 0   &&
               (lineid-ymove)  <= (windownum )
	      )
          )
		return;

	char buf[100];
	memset(buf,0,sizeof(buf));
	//sprintf(buf,"No.%d Trunk Information:%s",lineid,info);
	sprintf(buf,"%s",info);
	mvwaddstr(g_win[lineid-ymove][col],1,1,buf);
	refresh();
	wrefresh(g_win[lineid-ymove][col]);
}

void draw_main_trunk(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	draw_main_id        (windownum,lineid,ymove,pOneTrunk);
	draw_main_modch     (windownum,lineid,ymove,pOneTrunk);
	draw_main_type      (windownum,lineid,ymove,pOneTrunk);
	draw_main_linestate (windownum,lineid,ymove,pOneTrunk);
	draw_main_trunkstate(windownum,lineid,ymove,pOneTrunk);
	draw_main_called    (windownum,lineid,ymove,pOneTrunk);
	draw_main_calling   (windownum,lineid,ymove,pOneTrunk);
	draw_main_dtmf      (windownum,lineid,ymove,pOneTrunk);
	draw_main_vocinfo   (windownum,lineid,ymove,pOneTrunk);
}

void draw_main_id(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%4d",pOneTrunk->iSeqID);
	draw_main(windownum,lineid,0,buf,ymove);
}

void draw_main_modch(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%2d,%3d",pOneTrunk->deviceID.m_s8ModuleID,pOneTrunk->deviceID.m_s16ChannelID);
	draw_main(windownum,lineid,1,buf,ymove);
}

void draw_main_type(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%10s",GetString_DeviceSub(pOneTrunk->deviceID.m_s16DeviceSub));
	draw_main(windownum,lineid,2,buf,ymove);
}

void draw_main_linestate(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	char buf2[100];
	memset(buf,0,sizeof(buf));
	memset(buf2,0,sizeof(buf2));
	GetString_LineState(buf,pOneTrunk->iLineState);
	sprintf(buf2,"%14s",buf);
	//wattron(g_win[lineid][3],C_RED | A_STANDOUT | A_BOLD|A_BLINK | A_UNDERLINE |A_REVERSE );
	draw_main(windownum,lineid,3,buf2,ymove);
	//wattroff(g_win[lineid][3],C_RED);
}

void draw_main_trunkstate(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%26s",GetString_TrunkState(pOneTrunk->State));
	draw_main(windownum,lineid,4,buf,ymove);
}

void draw_main_called(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%8s",pOneTrunk->CalleeCode);
	draw_main(windownum,lineid,5,buf,ymove);
}

void draw_main_calling(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%8s",pOneTrunk->CallerCode);
	draw_main(windownum,lineid,6,buf,ymove);
}

void draw_main_dtmf(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%8s",pOneTrunk->DtmfBuf);
	draw_main(windownum,lineid,7,buf,ymove);
}

void draw_main_vocinfo(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk)
{
	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"(%d,%3d)",pOneTrunk->VocDevID.m_s8ModuleID,pOneTrunk->VocDevID.m_s16ChannelID);
	draw_main(windownum,lineid,8,buf,ymove);
}

//draw whole main window with the scroll position...
/*void draw_main_all(int windownum,int colnum, int ymove)
{
	for(int i=0;i<windownum;i++)
	{
		for(int j=0; j<colnum;j++)
		{
			//draw_main(windownum,i+ymove,j,g_buf[i+ymove][j],ymove);
			draw_main_trunk(windownum,i+ymove,ymove,pOneTrunk);
		}
	}
}*/
void draw_main_all(int windownum,int colnum, int ymove)
{
	DeviceID_t dev;
	TRUNK_STRUCT* pOneTrunk;
	for(int i=0;i<windownum;i++)
	{
		for(int j=0; j<colnum;j++)
		{
			//draw_main(windownum,i+ymove,j,g_buf[i+ymove][j],ymove);
			
			dev.m_s8ModuleID   = MapTable_Trunk[i+ymove].m_s8ModuleID;
			dev.m_s16ChannelID = MapTable_Trunk[i+ymove].m_s16ChannelID;
			
			//if( dev.m_s8ModuleID == 0 )
			//	return;
				
			pOneTrunk = &M_OneTrunk(dev);
			draw_main_trunk(windownum,i+ymove,ymove,pOneTrunk);
		}
	}
}
//parameter ...
//lineid   : the line id
//namestr  : the variable's name
//valuestr : the variable's value
void draw_setupwin(int lineid,const char* namestr,const char* valuestr)
{
	draw_setupstatic(lineid,namestr);
	draw_setupdata(lineid,valuestr);
}


//draw the specified setup static information
void draw_setupstatic(int lineid,const char* info)
{
	if( lineid<0 || lineid > MAXWIN)
		return;

	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%012s",info);
	mvwaddstr(g_setupstatic[lineid],1,1,buf);
	refresh();
	wrefresh(g_setupstatic[lineid]);
}

//draw the specified setup data window
void draw_setupdata(int lineid,const char* info)
{
	if( lineid<0 || lineid > MAXWIN)
		return;

	char buf[100];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%012s",info);
	//redrawwin(g_setupdata[lineid]);
	mvwaddstr(g_setupdata[lineid],1,1,buf);
	wattron(g_setupdata[lineid],A_UNDERLINE);
	wattron(g_setupdata[lineid],A_BOLD);
	refresh();
	wrefresh(g_setupdata[lineid]);
}

//initilize the main window 
int init_main_window(int windownum,int colnum,int linelen,int startx,int starty)
{
	int i = 0;
	if( windownum > MAXWIN)
	{
		printf("windownum[%d] too large\n",windownum);
		return -1;
	}
	if( colnum > MAXCOL)
	{
		printf("colnum[%d] too large\n",colnum);
		return -2;
	}

	i = 1;
	g_winhead = newwin(linehigh,linelen,starty+(linehigh-1)*i,startx);
	keypad(g_winhead,TRUE);
	box(g_winhead,'|','-');
	refresh();
        mvwaddstr(g_winhead,1,1,"     ID      Mod,Ch       Type       LineState              State                  Called         Calling       DTMF   VocInfo");
	wrefresh(g_winhead);

	for( i=2; i< (windownum+2); i++ )
	{
		for( int j=0; j< colnum;j++)
		{
			switch(j)
			{
				case 0:
				g_win[i-2][j] = newwin(linehigh,10,starty+(linehigh-1)*i,startx);
				break;
				case 1:
				g_win[i-2][j] = newwin(linehigh,10,starty+(linehigh-1)*i,startx+10);
				break;
				case 2:
				g_win[i-2][j] = newwin(linehigh,15,starty+(linehigh-1)*i,startx+20);
				break;
				case 3:
				g_win[i-2][j] = newwin(linehigh,15,starty+(linehigh-1)*i,startx+35);
				break;
				case 4:
				g_win[i-2][j] = newwin(linehigh,30,starty+(linehigh-1)*i,startx+50);
				break;
				case 5:
				g_win[i-2][j] = newwin(linehigh,15,starty+(linehigh-1)*i,startx+80);
				break;
				case 6:
				g_win[i-2][j] = newwin(linehigh,15,starty+(linehigh-1)*i,startx+95);
				break;
				case 7:
				g_win[i-2][j] = newwin(linehigh,10,starty+(linehigh-1)*i,startx+110);
				break;
				case 8:
				g_win[i-2][j] = newwin(linehigh,10,starty+(linehigh-1)*i,startx+120);
				break;
				
			}
			//g_win[i-2][j] = newwin(linehigh,linelen/colnum+1,starty+(linehigh-1)*i,startx+(linelen/colnum)*j);
			keypad(g_win[i-2][j],TRUE);
			box(g_win[i-2][j],'|','-');
			refresh();
			wrefresh(g_win[i-2][j]);
			mvwaddstr(g_win[i-2][j],1,1,"");
			wrefresh(g_win[i-2][j]);
		}
		
		
	}
	return 0;
}

//initialize the title window with Title infor
int init_title_window(int windownum,int linelen,int startx,int starty,const char* info)
{

	int i = 0;
	char buf[100];
	if( windownum > MAXWIN)
	{
		printf("setup windownum[%d] too large\n",windownum);
		return -1;
	}

	g_title=  newwin(linehigh,linelen,starty+(linehigh-1)*i,startx);
	keypad(g_title,TRUE);
	box(g_title,'|','-');
        mvwaddstr(g_title,1,1,info);
	wattron(g_title,A_BOLD);
	//wattron(g_title,A_REVERSE);
	refresh();
	wrefresh(g_title);

}

//intialize the bottom window with the Usage information...
int init_bottom_window(int windownum,int linelen,int startx,int starty,const char* info)
{

        int i = 0;
        char buf[100];
        if( windownum > MAXWIN)
        {
                printf("setup windownum[%d] too large\n",windownum);
                return -1;
        }

	i = 15+2;
        g_bottom=  newwin(linehigh,linelen,starty+(linehigh-1)*i,startx);
	keypad(g_bottom,TRUE);
        box(g_bottom,'|','-');
        refresh();
        mvwaddstr(g_bottom,1,1,info);
        wrefresh(g_bottom);

}

//intialize the specified setup data window
int init_setup_window(int windownum,int linelen,int startx,int starty)
{
	int i = 0;
	char buf[100];
	if( windownum > MAXWIN)
	{
		printf("setup windownum[%d] too large\n",windownum);
		return -1;
	}

	i= 1;

	g_setuphead=  newwin(linehigh,linelen,starty+(linehigh-1)*i,startx);
	keypad(g_setuphead,TRUE);
	box(g_setuphead,'|','-');
	refresh();
        mvwaddstr(g_setuphead,1,1," Setup Window ");
	wrefresh(g_setuphead);

	for( i=2; i< ( windownum + 2 );  )
	{
		g_setupstatic[i-2] = newwin(linehigh,linelen/2+1,starty+(linehigh-1)*i,startx);
		keypad(g_setupstatic[i-2],TRUE);
		box(g_setupstatic[i-2],'|','-');
		refresh();
		wrefresh(g_setupstatic[i-2]);
		memset(buf,0,sizeof(buf));
		//sprintf(buf,"Static[%d]",i-2);
		sprintf(buf,"");
                mvwaddstr(g_setupstatic[i-2],1,1,buf);
		wrefresh(g_setupstatic[i-2]);

		g_setupdata[i-2] = newwin(linehigh,linelen/2,starty+(linehigh-1)*i,startx+linelen/2);
		keypad(g_setupdata[i-2],TRUE);
		box(g_setupdata[i-2],'|','-');
		refresh();
		wrefresh(g_setupdata[i-2]);
		memset(buf,0,sizeof(buf));
		//sprintf(buf,"Data[%d]",i-2);
		sprintf(buf,"");
                mvwaddstr(g_setupdata[i-2],1,1,buf);
		wrefresh(g_setupdata[i-2]);

		i++;
	}
}

//initialize the ALL Window: title,main,setup(static,data),bottom 
void init_window(int windownum,int colnum,int mainlen,int setuplen,const char* title,const char* bottom)
{
	init_title_window(3,mainlen + setuplen-1,1,1,title);
	init_main_window(windownum,colnum,mainlen,1,1);
	
	if( setuplen >0 )
		init_setup_window(windownum,setuplen,mainlen,1);
	init_bottom_window(3,mainlen+setuplen-1,1,1,bottom);
}


//int windownum=15;
//int colnum = 8;
//int mainlen = 100;
//int setuplen = 30;

int mainwork(int windownum,int colnum,int mainlen,int setuplen,const char* title,const char* bottom)
{
	int i,ch,x,y;
	char buf[100];
	//int ymove = 0; //the benchmark of Main Window Scrolled;
	//int trknum = 30;
	//int windownum=15;
	//int colnum = 8;
	//int mainlen = 100;
	//int setuplen = 30;
	
	initscr();
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr,FALSE);
	
	/*if ( ! has_colors( ) )
	{
		printf(" NON-color Mode \n");
		endwin();
		exit(0);
	}
	
	if( start_color() == ERR )
	{
		printf(" start_color failed \n");
		endwin();
		exit(0);
	}
	init_pair(C_BLACK,COLOR_BLACK,COLOR_BLACK);
	init_pair(C_GREEN,COLOR_GREEN,COLOR_BLACK);
	init_pair(C_RED,COLOR_RED,COLOR_GREEN);
	init_pair(C_CYAN,COLOR_CYAN,COLOR_BLACK);
	init_pair(C_WHITE,COLOR_WHITE,COLOR_BLACK);
	init_pair(C_MAGENTA,COLOR_MAGENTA,COLOR_BLACK);
	init_pair(C_BLUE,COLOR_BLUE,COLOR_BLACK);
	init_pair(C_YELLOW,COLOR_YELLOW,COLOR_BLACK);*/

	
/*
	init_title_window(3,129,1,1,"                       ***** Donjin XMS Demo Sample *****   By Jerry Sep,6,2007  ");
	init_main_window(15,90,1,1);
	init_setup_window(15,40,90,1);
	init_bottom_window(3,129,1,1,"['q' to Quit] [UP,DOWN to Scroll Window][0,1,...N to Focus on the SetupWindow]  ");
*/

	//init_window(windownum,colnum,mainlen,setuplen,"                       ***** Donjin XMS Demo Sample *****   By Jerry Sep,6,2007  ","['q' to Quit] [UP,DOWN to Scroll Window] ");

	init_window(windownum,colnum,mainlen,setuplen,title,bottom);

	//NONBlock -1: Use Select

	//for test only
	//init_buffer(trknum,colnum,100);

	struct timeval t;
	fd_set f;
	
	t.tv_sec=0;
	t.tv_usec = 1000000;

	//NONBlock -2 :Use fcntl(0,F_SETFL,O_NONBLOCK);

	draw_setupstatic(0,"IP");
	draw_setupstatic(1,"Port");
	draw_setupdata(0,"58");
	draw_setupdata(1,"9000");
	
	int scrolled = windownum;
	
	int drawflag = 0;
	ch = 0 ;
	for( ;;)
	{
		if( g_getdev == 0 )
		{
			usleep(1000);
			continue;
		}
		refresh();

		if ( drawflag == 1)
		{
			draw_main_all(windownum,colnum,g_ymove);
			drawflag = 0 ;
		}
		//FD_ZERO(&f);
		//FD_SET(0,&f);

		//if ( !select(1,&f,NULL,NULL,&t))
		//	continue; 
		if ( ( ch = getch() ) == ERR )
		{
			ch = 0 ;
			usleep(1000);
		}

		drawflag = 1;	
		switch(ch)
		{
			case 'q':endwin();exit(0);
			case 65:
			case KEY_UP:  
				if( g_ymove > 0 )
				{
					if ( scrolled < windownum)
					{
						g_ymove -= scrolled;
						scrolled = windownum;
						
					}
					else if (scrolled == windownum )
					{
						g_ymove -= scrolled;
					}
				 	
				}
				break;
			case 66:
			case KEY_DOWN: 
				if( scrolled < windownum) 
					break;
					
				if( g_ymove < ( trknum - windownum) && (trknum-windownum - g_ymove )>windownum )
				{
					scrolled = windownum; 
					g_ymove+=scrolled;
					
				}
				else if( g_ymove < ( trknum - windownum) && (trknum-windownum - g_ymove )<windownum )
				{
					scrolled =(trknum-windownum - g_ymove);
					g_ymove  +=scrolled;
					
				}
				break;
			default:
				drawflag = 0;
				break;
		}	

		/*draw_setupstatic(windownum-1,"Scro");
		memset(buf,0,sizeof(buf));
		//sprintf(buf,"%d-%d %d-%03d",KEY_UP,KEY_DOWN,ch,ymove);
		sprintf(buf,"%04d",g_ymove);
		draw_setupdata(windownum-1,buf);*/

		//draw_main_all(windownum,colnum,g_ymove);
		
	}
}

/*int main(int argc,char** argv)
{
	mainwork(15,8,100,30,"DONJIN DJXMS DEMO BY JERRY SEP,05,2007","[q] to exit ,Up,Down to Scroll Window.");
}*/
