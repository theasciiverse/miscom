#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <curses.h>

#include "global.h"
#include "unix.h"
#include "wwn.h"
#include "draw.h"


/* see if colour is supported, and if it's wanted */
#if defined(ACS_HLINE) && !defined(NO_COLOUR) && !defined(NO_COLOR)
#define HAVE_COLOUR
#endif

static int colour=0;




void use_colour(int colnum,int other_attr)
{
#ifdef HAVE_COLOUR
if(colnum==0)
  attrset(A_NORMAL);
else
  if(colour)
    attrset(COLOR_PAIR(colnum)|other_attr);
  else
    attrset(other_attr);
#endif
}


void define_colours()
{
#ifndef HAVE_COLOUR
/* this is a nop without colour support */
#else
int f;

/* make it use colour */
start_color();

colour=has_colors();

if(!colour) return;

/* declare colour pairs we need */

for(f=1;f<=7;f++)
  init_pair(f,f,0);
#endif
}



void wipe_area()
	{
	int x,y;

	for(y=0;y<MAX_Y;y++)
		{
		for (x=0;x<MAX_X;x++)
			{
			setpixel(x,y,0);
			}
		}
	}	

void dispcities()
	{
	int i;

	use_colour(COLOR_GREEN,A_BOLD);
	for (i=1;i<8;i++)
		{
		move(20,CtyPos[i]-1);
		if (Cities[i]==1)
		addstr("///.");
		}
	move(20,CtyPos[0]-1);
	if (Cities[0]==1) addstr(" /| ");
	move(20,CtyPos[4]-1);
	if (Cities[4]==1) addstr(" /\\ ");
	move(20,CtyPos[8]-1);
	if (Cities[8]==1) addstr("|\\");
	use_colour(COLOR_WHITE,A_NORMAL);
	}


void landscape()
	{
	int i;

	dispcities();
	move(21,1);
	use_colour(COLOR_GREEN,A_NORMAL);
	
	for (i=0;i<MAX_X;i++)
		{
		if (random()%2==0)
			{
			addch('`');
			}
		else
			{	
			addch('\'');	
			}
		}
	
	use_colour(COLOR_WHITE,A_NORMAL);
	}


void oneammodisp(int base)
	{
	int x=0;
	int numwhole;
	char topchar,bottomchar;
	
	switch(base)
		{
		case 0: x=1; break;
		case 1: x=34; break;
		default: x=67;
		}

	if (Ammo[base]==0)
		{
		move(22,x);
		addstr("      ");
		move(23,x);
		addstr("      ");
		return;
		}

	if (Ammo[base]>18)
		{
		move(22,x); addstr(":::::");
		move(23,x); addstr(":::::");
		}

	switch(Ammo[base]%4)
		{
		case 3:
			topchar='.';
			bottomchar=':';
			break;
		case 2:
			topchar=' ';
			bottomchar=':';
			break;
		case 1:
			topchar=' ';
			bottomchar='.';
			break;
		default:
			topchar=' ';
			bottomchar=' ';
		}
	
	numwhole=Ammo[base]/4;
	mvaddch(22,x+numwhole,topchar);
	mvaddch(23,x+numwhole,bottomchar);
	}	


void dispammo()
	{
	oneammodisp(0);
	oneammodisp(1);
	oneammodisp(2);
	}

/* XXX! */
int dispscore()
	{
	return(1);
	use_colour(COLOR_CYAN,A_NORMAL);
  	move(23,10);
  	printw("Score: %i",Score);
	use_colour(COLOR_WHITE,A_NORMAL);
	}

void drawptr(int x,int y)
	{
	use_colour(COLOR_YELLOW,A_BOLD);
  	move(y/2,x);
  	printw("><");
	use_colour(COLOR_WHITE,A_NORMAL);
	}

void undrawptr(int x,int y)
	{
	redrawpix(x,y);	
	redrawpix(x,y-1);
	redrawpix(x+1,y);
	}


	
void drawline(int x1,int y1,int x2,int y2,int xx,int yy,int num)
	{
	int x,y;
	struct direction ret;

	ret.finished=0;

	x=x1;
	y=y1;
	
	while (ret.finished!=1)
		{
		ret=wwn(x1,y1,x,y,x2,y2);
		if (ret.finished==1 || ((x==xx) && (y==yy))) return;
		x+=ret.dx;
		y+=ret.dy;
		setpixel(x,y,num);
		}
	}


	
int redrawpix(int x,int y)
	{
	if (chkcoords(x,y)==-1)
		return(-1);

	setpixel(x,y,Screen[x][y]);
	return(0);
	}

int chkcoords(int x,int y)
	{
	/* y stops below MAX_Y: Screen has MAX_Y half-rows, and row MAX_Y/2
	 * is the cities' */
	if ( (x>-1)&&(x<=MAX_X)&&(y>-1)&&(y<MAX_Y) )	
		{
		return(0);
		}
	return(-1);
	}

int setpixel(int x,int y,int num)
	{
	int total;
	int wholey;
	char outc;

	if (chkcoords(x,y)==-1)
		return(-1);

	Screen[x][y]=num;

	total=0;
	
	wholey=y/2;
	wholey*=2;	

	if ( Screen[x][wholey]==1) total=1;
	if ( Screen[x][wholey+1]==1) total+=2;	

	move(y/2,x);
	switch(total)
		{
		case 1: outc='`'; break;
		case 2: outc='.'; break;
		case 3: outc=':'; break;
		default:
			outc=' ';
		}
	
	addch(outc);
	return(0);
	}

