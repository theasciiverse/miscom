#include <stdio.h>
#include <curses.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "global.h"
#include "wwn.h"


struct direction wwn(int ox,int oy,int cx,int cy,int tx,int ty)
	{
	int accx,accy;
	int diagx,diagy;
	struct direction ret;
	float ograd;
	float ngrad;
	int compoint=0;
	int dx,dy;

	dx=tx-ox;
	dy=ty-oy;

	/* sort out straight lines */

	ret.dx=0;
	ret.dy=0;
	ret.finished=0;

	if ( (dx==0) && (dy==0) )
		{
		ret.finished=1;
		return(ret);
		}

	if ( (dx==0) && (dy>0) )
		{
		ret.dx=0;
		ret.dy=1;
		return(ret);
		}
	if ( (dx==0) && (dy<0) )
		{
		ret.dx=0;
		ret.dy=-1;
		return(ret);
		}

	if ((dy==0) && (dx>0))
		{
		ret.dx=1;
		ret.dy=0;
		return(ret);
		}

	if ((dy==0) && (dx<0))
		{
		ret.dx=-1;
		ret.dy=0;
		return(ret);
		}
	ograd=(float)dy/(float)dx;

	if (dx>0)
		{
		/* right hand side */

		if ( ograd>=1.0) 
			{
			compoint=1;
			accx=0;
			accy=1;
			diagx=1;
			diagy=1;
			}
		else if ( (ograd>=0.0)	&& (ograd<=1.0))
			{
			compoint=2;
			accx=1;
			accy=0;
			diagx=1;
			diagy=1;
			}
		else if ( (ograd<0.0) && (ograd>=-1.0))
			{
			compoint=3;
			accx=1;
			accy=0;
			diagx=1;
			diagy=-1;
			}
		else /* if ( ograd<-1.0) */
			{
			compoint=4;
			accx=0;
			accy=-1;
			diagx=1;
			diagy=-1;
			}
		}
	else
		{
		if (ograd>=1.0)
			{
			compoint=5;
			accx=0;
			accy=-1;
			diagx=-1;
			diagy=-1;
			}
		else if ( (ograd<1.0) && (ograd>=0.0))
			{
			compoint=6;
			accx=-1;
			accy=0;
			diagx=-1;
			diagy=-1;
			}
		else if ( (ograd<0.0) && (ograd>=-1.0))
			{
			compoint=7;
			accx=-1;
			accy=0;
			diagx=-1;
			diagy=1;
			}
		else /* if (ograd<-1.0) */
			{
			compoint=8;
			accx=0;
			accy=1;
			diagx=-1;
			diagy=1;
			}
		}
	if ( (ty-cy==0) && (tx-cx==0) )
		{
		ret.finished=1;
		return(ret);
		}

	if (tx-cx==0)
		{
		ret.dx=0;
		ret.dy=diagy;
		return(ret);
		}

	ngrad=(float)(ty-cy)/(float)(tx-cx);

	if ((compoint%2)==1)
		{
		if (ngrad>ograd)
			{
			ret.dx=accx;
			ret.dy=accy;
			}
		else
			{
			ret.dx=diagx;
			ret.dy=diagy;
			}
		}
	else
		{
		if (ngrad>ograd)
			{
			ret.dx=diagx;
			ret.dy=diagy;
			}
		else
			{
			ret.dx=accx;
			ret.dy=accy;
			}
		}
	
	return(ret);	

	}



	
