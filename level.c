#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include "global.h"
#include "settings.h"
#include "draw.h"
#include "fire.h"
#include "unix.h"
#include "level.h"

int split_prob[9]=
	{ 
	0,
	0, 
	2000, 
	1000, 
	500,
 	350, 
	225, 
	180, 
	80
	};

int smart_prob[9]=
	{
	0,
	0,
	0,
	1000,
	500,
	250,
	125,
	80,
	10
	};



void lev_complete()
	{
	int x,y;
	int abms;

	char tempstring[1024];
	
	for (y=1;y<MAX_Y;y++)
		{
		for (x=1;x<MAX_X;x++)
			{
			setpixel(x,y,0);
			}
		}
	abms=Ammo[0]+Ammo[1]+Ammo[2];
	

	use_colour(COLOR_CYAN,A_BOLD);

	sprintf(tempstring,"You have completed level %i",Level+1);
	centre(tempstring,5);

	move(1,1);
	refresh();
	usleep(600000);
	
	sprintf(tempstring,"%i cities remaining: %i pts",
				numalive(),
				numalive()*CITY_SCORE);
	centre(tempstring,10);

	move(1,1);
	refresh();
	usleep(300000);
	
	sprintf(tempstring,"%i ABM's remaining: %i pts",
				abms,abms*ABM_SCORE);

	if ( abms!=0) centre(tempstring,12);
	move(1,1);	
	refresh();
	usleep(300000);
	Score+=( (abms*ABM_SCORE)+(numalive()*CITY_SCORE) );
	
	sprintf(tempstring,"Score: %i",Score);
	centre(tempstring,14);
	move(1,1);
	refresh();
	sleep(5);

	use_colour(COLOR_WHITE,A_NORMAL);

	wipe_area();
	refresh();
	}
	


