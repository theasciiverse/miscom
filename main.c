/* miscom - defend cities from missile attack.
 * Copyright (C) 1995-6 Graham Richards.
 * Some changes by Russell Marks.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <curses.h>
#include "global.h"
#include "settings.h"
#include "draw.h"
#include "fire.h"
#include "level.h"
#include "sound.h"
#include "unix.h"
#include "wwn.h"


struct Missile missiles[MAXMISSILES];
struct Fireball fireballs[MAXMISSILES];
struct { int active;
	int x;
	int y;
	int speed;
	int sp; } plane;

int CtyPos[10]={1,9,17,25,35,43,51,59,68,0};

int Score;
int Ammo[3];
int keyptr;
int Cities[20];
int Delay;
int Screen[80][40];
int num_missiles;
int Level;


void killcity(int city)
	{
	int x;
	
	Cities[city]=0;
	x=CtyPos[city];
	if (city==0)
		{
		Ammo[0]=0;
		dispammo();
		}
	if (city==4)
		{	
		Ammo[1]=0;
		dispammo();
		}

	if (city==8)
		{
		Ammo[2]=0;
		dispammo();
		}

	}


void explode(int city)
	{
	if (Cities[city]==1)
		{
		queuesam(EFFECT_CHANNEL,CITYBANG_SAMPLE);
		killcity(city);
		}
	}


void missileframe() 
	{
	int i;
	int drawn;
	struct direction ret;

	for (i=0;i<MAXMISSILES;i++)
		{
		if (missiles[i].active==1)
			{
			drawn=0;
			missiles[i].spc--;
			if (missiles[i].spc<1)
				{
				missiles[i].spc=missiles[i].speed;


				ret=wwn(	missiles[i].fromx,
						missiles[i].fromy,
						missiles[i].x,
						missiles[i].y,
						missiles[i].tx,
						missiles[i].ty);

				if (ret.finished==1)
					{
					drawline(missiles[i].fromx,
						missiles[i].fromy,
						missiles[i].tx,
						missiles[i].ty,
						missiles[i].x,
						missiles[i].y,0);

					addfire(missiles[i].x,
						missiles[i].y,
						missiles[i].mis_type);
					
	
					if (missiles[i].mis_type==ICBM)
						{
						explode(missiles[i].city);
						}
					missiles[i].active=0;
					}
				else
					{

				if ((chkfire(missiles[i].x,
					missiles[i].y+RADIUS_ROOT)==NO_NUKE) ||(missiles[i].mis_subtype!=SMART) )
					{
					missiles[i].x+=ret.dx;
					missiles[i].y+=ret.dy;

					}
					if (chkcoords(missiles[i].x,missiles[i].y)==-1)
						{
						/* missiles gone AWOL (is this a can't happen?) */
						}

					if ((chkfire(missiles[i].x,missiles[i].y)==NUKE)&&(missiles[i].mis_type!=ABM) )
						{
						/* explode it */
						drawn=1;

						drawline(missiles[i].fromx,
						 missiles[i].fromy,
						 missiles[i].tx,
						 missiles[i].ty,
						 missiles[i].x,
						 missiles[i].y,0);
					/* cancel it */
						missiles[i].active=0;

						addfire(missiles[i].x,
							missiles[i].y,
							missiles[i].mis_type);
					
						}	

					if (missiles[i].mis_type==ICBM)
					  use_colour(COLOR_MAGENTA,A_BOLD);
					else
					  use_colour(COLOR_GREEN,A_BOLD);
					setpixel(missiles[i].x,missiles[i].y,1);
					use_colour(COLOR_WHITE,A_NORMAL);
					}

				}
			}
		}
	/* do the plane stuff */
	if (plane.active==1)
		{
		plane.sp--;
		if (plane.sp<1)
			{
			plane.sp=plane.speed;
			plane.x++;
			if (plane.x==MAX_X)
				{
				 plane.active=0;
				num_missiles--;
				}
			if (chkfire(plane.x,plane.y*2)==NUKE)
				{
				addfire(plane.x,plane.y*2,ICBM);
				plane.active=0;
				}
			if (random()%25==0)
				{
				addplaneicbm(plane.x,plane.y*2);
				}
			}		
		}
	}

int playlevel(int nmissiles)
	{
	int NotSeven=42;
	int countdown=L_DELAY;
	unsigned char gotkey;
	int curx,cury;
	int ocx,ocy;
	int beaten=0;
	int i;
	
	curx=35;
	cury=20;

	dispammo();
	drawptr(curx,cury);
	
	for (i=0;i<MAXMISSILES;i++)
		missiles[i].active=0;
	
	queuesam(EFFECT_CHANNEL,LEVELST_SAMPLE);
	
	while (NotSeven!=7)
		{
		missileframe();
		
		countdown--;
		if ((countdown==0)&&(nmissiles>0))
			{
			/* add a new missiles */
			addnewicbm();
			nmissiles--;
			countdown=(L_DELAY/2)+(random()%L_DELAY);
			}
		udfire();

		gotkey=getnbkey();
	
		if (gotkey!=0)
			{
			ocx=curx; ocy=cury;
			switch (gotkey)
				{
				case CKEY_UP:		cury-=5; break;
				case CKEY_DOWN:		cury+=5; break;
				case CKEY_LEFT:		curx-=6; break;
				case CKEY_RIGHT:	curx+=6; break;
				case CKEY_FIRE:		addnewabm(curx,cury); break;
				}
			if (curx<0) curx=0;
			if (curx>(MAX_X-1)) curx=MAX_X-1;
			if (cury<0) cury=0;
			if (cury>(MAX_Y-2)) cury=MAX_Y-2;
	
			if (gotkey==CKEY_PAUSE) key_pause();
			if ( (ocy!=cury) || (ocx!=curx) )
				{
				undrawptr(ocx,ocy);
				}
			drawptr(curx,cury);	
			}
		if ((random()%30)==0) drawptr(curx,cury);
		
		/* do stuff with plane */
		if (plane.active==1)
			{
			move(plane.y,plane.x);
			}
		else
			{
			move(CURHOME_Y,CURHOME_X);	
			}
	
		refresh();
		usleep(20000);
		
		if (Level>2)
			{
			if ((random()%split_prob[Level>8?8:Level])==0)
				spliticbm();
			}

		if (Level>4)
			{
			if ((random()%smart_prob[Level>8?8:Level])==0)
				makesmart();
			}

		if ((random()%150)==0)
			{
			if (numalive()==0) 
				{
				beaten=1;
				nmissiles=0;		
				}
			}

		/* add plane everysooften */
		if ((plane.active==0)&&(random()%1000==0))
			{
			plane.active=1;
			plane.x=1;
			plane.speed=Delay/3;
			plane.sp=4;
			plane.y=random()%10+(Level>5?5:Level);
			num_missiles++;
			}

			if ((nmissiles==0)&&(num_missiles==0)) NotSeven=7;
		}
	
	if (beaten==1) return(LEVEL_FAILED);
	return(LEVEL_COMPLETE);
	}


void resetammo()
	{
	Ammo[0]=AMMO;
	Ammo[1]=AMMO;
	Ammo[2]=AMMO;
	Cities[0]=1;
	Cities[4]=1;
	Cities[8]=1;
	}		
	

	
	
void setammo()
	{
	Ammo[0]=AMMO;
	Ammo[1]=AMMO;
	Ammo[2]=AMMO;
	
	Cities[0]=1;
	Cities[4]=1;
	Cities[8]=1;
	}


void initstuff()
	{
	int i;
	
	setammo();
	
	for (i=0;i<10;i++)
		{
		Cities[i]=1;
		}	
	
	for (i=0;i<MAXMISSILES;i++)
		{
		missiles[i].active=0;
		fireballs[i].active=0;
		}

	plane.active=0;
	plane.x=0;
	plane.y=0;
	plane.speed=10;
	plane.sp=10;	
	}


void playgame()
	{
	int ret;
	
	Level=Score=0;
	num_missiles=0;
	Delay=20;
	
	clear();
	initstuff();
	landscape();
	
	ret=LEVEL_COMPLETE;

	while (ret==LEVEL_COMPLETE)
		{
		ret=playlevel(MIS_PER_LEVEL);
		if(numalive()==0) break;	/* can happen late, so... */
		if (ret==LEVEL_COMPLETE)
			{
			queuesam(EFFECT_CHANNEL,LEVELEND_SAMPLE);
			lev_complete();
			}
		resetammo();
		dispcities();
		dispammo();
		Level++;
		Delay-=2;
		if(Delay<6) Delay=6;
		}
	}


void gameover()
	{
	char buf[128];
	
	queuesam(EFFECT_CHANNEL,GAMEOVER_SAMPLE);
	use_colour(COLOR_CYAN,A_BOLD);
	centre(" --- GAME OVER --- ",9);
	use_colour(COLOR_YELLOW,A_NORMAL);
	sprintf(buf," - SCORE: %06d - ",Score);
	centre(buf,11);
	use_colour(COLOR_GREEN,A_NORMAL);
	centre(" -- press a key -- ",13);
	use_colour(COLOR_WHITE,A_NORMAL);
	refresh();
	sleep(1);
	while(getnbkey()) ;
	while(!getnbkey()) usleep(100000);
	}


int titlescr()
	{
	int key;
	
	use_colour(COLOR_WHITE,A_NORMAL);
	clear();
	centre("Miscom",5);
	centre("Press Enter to play or Esc to quit",18);
	move(23,35);
	refresh();
	
	while(getnbkey()) ;
	while((key=getnbkey())!=10 && key!=13 && key!=27)
		usleep(100000);
	
	return(key==10 || key==13);
	}

		
int main()
	{
	initscr();
	cbreak(); noecho();
	define_colours();
	
#ifndef AV_BUILD
	if((keyptr=open("/dev/tty",O_RDONLY|O_NONBLOCK))<0)
		fprintf(stderr,"Couldn't open /dev/tty."),bombout();
#endif
	
	srandom(time(NULL));
	signal(SIGINT,request_quit);
	
	start_sndserv();
	
	while(titlescr())
		{
		playgame();
		gameover();
		}
	
	queuesam(KILL_SNDSERV,0);
	use_colour(COLOR_WHITE,A_NORMAL);
	clear();
	cbreak();
	echo();
	endwin();
	exit(0);
	}
