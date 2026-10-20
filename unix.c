#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <curses.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "global.h"
#include "draw.h"
#include "sound.h"
#include "unix.h"


void centre(char* buffer,int y)
	{
	int length;
	
	length=strlen(buffer);
	
	move(y, ((MAX_X/2)-(length/2)));
	printw("%s",buffer);
	}	

int numalive()
	{
	int numok=0;
	int i;

	for (i=1;i<4;i++)
		{
		numok+=Cities[i];
		}
	for (i=5;i<8;i++)
		{
		numok+=Cities[i];
		}
	return(numok);
	}
		

/* set by the SIGINT handler; acted on here, where curses may be used */
volatile sig_atomic_t quit_requested=0;

void request_quit(int sig)
	{
	quit_requested=1;
	}

unsigned char getnbkey()
	{
	unsigned char red;
	int ret;
	
	if (quit_requested) bombout();
	ret=read(keyptr,&red,1); 

	if (ret==-1)
		{
		red=0;
		}

	return(red);
	}	

void key_pause()
	{
	unsigned char gotkey=0;
	int i;

	/* display message */

	move(10,30);
	printw("P A U S E D");
	move(1,1);
	refresh();
	while (gotkey==0)
		{
		usleep(300000);
		gotkey=getnbkey();
		}
	for (i=30;i<42;i++)
		{
		redrawpix(i,20);
		}

	refresh();
	}


void bombout()
	{
	queuesam(KILL_SNDSERV,0);
	use_colour(COLOR_WHITE,A_NORMAL);
	clear();
	cbreak();
	echo();
        endwin();
	exit(0);
	}
		
