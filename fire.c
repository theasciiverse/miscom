#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <curses.h>

#include "global.h"
#include "settings.h"
#include "draw.h"
#include "sound.h"
#include "unix.h"
#include "fire.h"


/* current explosion channel to use - we cycle through them */
int expchan=EXPLODE_CHAN1;


void circle(int xx,int yy,int num)
	{
	int x,y;
	int sx,sy;
	int ex,ey;
	int dx,dy;

	sx=xx-RADIUS_ROOT;
	sy=yy-RADIUS_ROOT;
	
	ex=xx+RADIUS_ROOT;
	ey=yy+RADIUS_ROOT;		

	for (x=sx;x<ex;x++)
		{
		for (y=sy;y<ey;y++)
			{
			dx=x-xx;
			dy=y-yy;

			if ( ((dx*dx)+(dy*dy))<RADIUS)
				{	
				setpixel(x,y,num);
				}
			}
		}
	}


void addfire(int x,int y,int exp_type)
	{
	int ii;
	int a;
	int found=0;	
	a=9999;	

	for (ii=0;ii<MAXMISSILES;ii++)
		{
		if (fireballs[ii].active==0)
			{
			a=ii;
			found=1;
			break;
			}
			
		}

	if (found==0)
		fprintf(stderr,"can't happen\n"),bombout();

	fireballs[a].x=x;
	fireballs[a].y=y;
	fireballs[a].radius=RADIUS;
	fireballs[a].countdown=DURATION;
	fireballs[a].active=1;
	fireballs[a].exp_type=exp_type;
	
	queuesam(expchan,EXPLODE_SAMPLE);
	if(++expchan>EXPLODE_CHAN3) expchan=EXPLODE_CHAN1;
	use_colour(COLOR_RED,A_BOLD);
  	circle(x,y,1);
	use_colour(COLOR_WHITE,A_NORMAL);
	}

void udfire()
	{
	int i;
	int numblatted=0;
	int numactive=0;

	for (i=0;i<MAXMISSILES;i++)
		{
		if (fireballs[i].active==1)
			{
			numactive++;
			fireballs[i].countdown-- ;
			
			if (fireballs[i].countdown<1)
				{
				fireballs[i].active=0;
				numblatted++;			

				num_missiles--;
				use_colour(COLOR_RED,A_BOLD);
  				circle(fireballs[i].x,fireballs[i].y,0);
				use_colour(COLOR_WHITE,A_NORMAL);
				
				/* if y is greater than, say, 20 then
				 * redraw cities */
				dispcities();
				}
	
			
			}
		}
	}

int chkfire(int x,int y)
	{
	int i;
	int dx,dy;
	
	for (i=0;i<MAXMISSILES;i++)
		{
		if (fireballs[i].active==1)
			{
			dx=x-fireballs[i].x;
			dy=y-fireballs[i].y;
			
			if ( ((dx*dx)+(dy*dy))<fireballs[i].radius)
				{
				/* we got a nuke */
				/* increment score */
				Score+=EXP_SCORE;
				dispscore();
				return(NUKE);
				}
			}
		}
	return(NO_NUKE);
	}				


void addmis(int fromx,int fromy,int tx,int ty,int mis_type,int speed,int cit)
	{
	int i;
	int a=-1;
	
	for (i=0;i<MAXMISSILES;i++)
		{
		if (missiles[i].active==0)
			{
			a=i;
			break;
			}
		}
	
	if(a==-1) return;
	
	num_missiles++;

	missiles[a].active=1;
	missiles[a].fromx=fromx;
	missiles[a].fromy=fromy;
	missiles[a].x=fromx;
	missiles[a].y=fromy;	
	missiles[a].spc=1;
	missiles[a].speed=speed;
	missiles[a].tx=tx;
	missiles[a].ty=ty;
	missiles[a].mis_type=mis_type;
	missiles[a].city=cit;
	missiles[a].mis_subtype=0;
	}	

void addnewicbm()
	{
	int startx;
	int target;
	int cit;
	
	cit=random()%9;
	target=CtyPos[cit];	

	startx=random()%70;
	if (startx==target) startx+=2;
	addmis(startx,1,target,40,ICBM,Delay/2,cit);
	}

void addplaneicbm(int startx,int y)
	{
	int target;
	int cit;
	cit=random()%9;
	target=CtyPos[cit];
	if (startx==target) startx++;
	if(y<40)
		addmis(startx,y,target,40,ICBM,Delay/2,cit);
	}

int absolut(int a)
	{
	if (a<0) a*=-1;
	return(a);
	}

int choose(int x)
	{
	int a,b,c;

	a=x-CtyPos[0];
	b=x-CtyPos[4];	
	c=x-CtyPos[8];

	a=absolut(a);
	b=absolut(b);
	c=absolut(c);

	if (Ammo[0]==0) a=9999;
	if (Ammo[1]==0) b=9999;
	if (Ammo[2]==0) c=9999;

	if ( (a==b)&&(a<9999) ) return(1);
	if ( (b==c)&&(b<9999) ) return(3);

	if ( (a<b) && (a<c) ) return(1);
	if ( (b<a) && (b<c) ) return(2);
	if ( (c<a) && (c<b) ) return(3);
	return(0);
	}	
	
void addnewabm(int x,int y)
	{
	int startx;
	int site;

	startx=72;
	
	random();	
	if (x<35) startx=0; 
	
	site=choose(x);
	if (site==1) startx=CtyPos[0];
	if (site==2) startx=CtyPos[4];
	if (site==3) startx=CtyPos[8];	

	/*startx=CtyPos[4];

*/	if (startx==x) startx++;

	if (site!=0)	
		{
		queuesam(ABM_CHANNEL,ABM_SAMPLE);
		addmis(startx,39,x,y,ABM,0,10);
		Ammo[site-1]--;

		dispammo();
		}
	}	
	

void spliticbm()
	{
	int i,a;
	int target;
	int city;

	a=0;
	
	for (i=0;i<MAXMISSILES;i++)
		{
		if ( (missiles[i].active==1) &&(missiles[i].mis_type==ICBM) )
			{	
			a++;
			}
		}
	if (a==0) return;
	
	/* choose one */

	for (i=0;i<MAXMISSILES;i++)
		{
		if ( (missiles[i].active==1)&&(missiles[i].mis_type==ICBM))
			{
			a=i;
			break;
			}
		}
	/* a is the one were going to split */
	
	city=random()%9;
	target=CtyPos[city];

	if (missiles[a].x==target) target++;

	if(missiles[a].y<40)
		addmis(missiles[a].x, missiles[a].y,
			target, 40,
			ICBM,missiles[a].speed*3/4,city);
	}	


void makesmart()
	{
	int i;
	int numactive=0;
	int a=-1;

	for (i=0;i<MAXMISSILES;i++)
		{
		if ( (missiles[i].active==1) && (missiles[i].mis_type==ICBM) )
			{
			numactive++;
			}
		}


	if (numactive==0) return;


	for (i=0;i<MAXMISSILES;i++)
		{
		if ( (missiles[i].active==1) && (missiles[i].mis_type==ICBM) )
			{
			a=i;
			break;
			}
		}
	
	if(a!=-1) missiles[a].mis_subtype=SMART;
	}
