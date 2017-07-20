#ifndef __AXMSCURSES_H__
#define __AXMSCURSES_H__


#define MAXCOL 10
#define MAXWIN 32*16*16

void draw_main(int windownum,int lineid,int colnum,const char* info,int ymove=0);
void draw_main_all(int windownum,int col,int ymove);
void draw_setupwin(int lineid,const char* namestr,const char* valuestr);
void draw_setupstatic(int lineid,const char* info);
void draw_setupdata(int lineid,const char* info);

int init_main_window(int windownum,int colnum,int linelen,int startx,int starty);
int init_title_window (int windownum,int linelen,int startx,int starty,const char* info);
int init_bottom_window(int windownum,int linelen,int startx,int starty,const char* info);
int init_setup_window (int windownum,int linelen,int startx,int starty);

void init_window(int windownum,int mainlen,int setuplen,const char* title,const char* bottom);

int mainwork(int windownnum,int colnum,int mainlen,int setuplen,const char* title,const char* bottom);




void draw_main_trunk     (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_id        (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_modch     (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_type      (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_linestate (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_trunkstate(int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_called    (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_calling   (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_dtmf      (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);
void draw_main_vocinfo   (int windownum,int lineid,int ymove,TRUNK_STRUCT* pOneTrunk);


#endif
