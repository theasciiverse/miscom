#ifdef SOUND_SUPPORT

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include "sound.h"

int sound=1;
int soundfd=-1;
int sndpipe[2];
int sound_bufsiz;

/* use 256 byte frags */
#define BASE_SOUND_FRAG_PWR	8
#define BASE_SOUND_BUFSIZ	(1<<BASE_SOUND_FRAG_PWR)


struct channel_tag {
  struct sample_tag *sample;	/* pointer to sample struct, NULL if none */
  int offset;			/* position in sample */
  } channel[NUM_CHANNELS];


/* sample filenames */
char samplename[][16]={
  "launch.raw",
  "explode.raw",
  "levelst.raw",
  "levelend.raw",
  "gameover.raw",
  "citybang.raw"
  };

/* for in-memory samples */
struct sample_tag {
  unsigned char *data;		/* pointer to sample, NULL if none */
  int length;			/* length of sample */
  } sample[NUM_SAMPLES];



/* setup a new sample to be played on a given channel. */
void queuesam(int chan,int sam)
{
unsigned char buf[2];

buf[0]=chan;
buf[1]=sam;
write(sndpipe[1],buf,2);
}


#define BYTEFIX(x)	(((x)<0)?0:(((x)>255)?255:(x)))

/* mix and play a chunk of sound to /dev/dsp. */
void snd_playchunk()
{
int f,g,v;
struct channel_tag *cptr;
static unsigned char soundbuf[1024];

if(soundfd==-1 || sound==0)
  {
  usleep(50000);
  return;
  }

for(f=0;f<sound_bufsiz;f++)
  {
  v=0;
  for(g=0,cptr=&(channel[0]);g<NUM_CHANNELS;g++,cptr++)
    if(cptr->sample!=NULL)
      {
      v+=(int)cptr->sample->data[cptr->offset++];
      if(cptr->offset>=cptr->sample->length)
        cptr->sample=NULL;
      }
    else
      v+=128;	/* make sure it doesn't click! */
      
  v/=NUM_CHANNELS;
  
  /* make it louder, as we can get away with it */
  v-=128; v*=2; v+=128;
  
  soundbuf[f]=BYTEFIX(v);
  }

write(soundfd,soundbuf,sound_bufsiz);
}


void snd_main()
{
unsigned char buf[2];
int live=1;

while(live)
  {
  while(read(sndpipe[0],buf,2)!=-1)
    {
    switch(*buf)
      {
      case KILL_SNDSERV:
        live=0;
        break;
      
      default:
        channel[buf[0]].sample=&(sample[buf[1]]);
        channel[buf[0]].offset=0;
      }
    }
  
  snd_playchunk();
  }

exit(0);
}


void start_sndserv()
{
int f;

for(f=0;f<NUM_SAMPLES;f++) sample[f].data=NULL;

pipe(sndpipe);
fcntl(sndpipe[0],F_SETFL,O_NONBLOCK);
fcntl(sndpipe[1],F_SETFL,O_NONBLOCK);

if(fork())
  /* parent - game */
  return;

/* child - sound player */

sound_bufsiz=BASE_SOUND_BUFSIZ;
#ifndef SNDCTL_DSP_SETFRAGMENT		/* voxware 3 only */
sound=0;
soundfd=-1;
#else
if(sound)
  {
  if((soundfd=open("/dev/dsp",O_WRONLY))<0)
    {
    sound=0;
    soundfd=-1;
    }
  else
    {
    int frag;
    FILE *in;
    char buf[256];
  
#ifdef MORE_SOUNDBUF
    frag=(0x40000|BASE_SOUND_FRAG_PWR);
#else
    frag=(0x20000|BASE_SOUND_FRAG_PWR);
#endif
    ioctl(soundfd,SNDCTL_DSP_SETFRAGMENT,&frag);
  
    /* load in the samples */
    for(f=0;f<NUM_SAMPLES;f++)
      {
      sprintf(buf,"%s/%s",SOUNDSDIR,samplename[f]);
      if((in=fopen(buf,"rb"))!=NULL)
        {
        fseek(in,0,SEEK_END);
        sample[f].length=ftell(in);
        if((sample[f].data=(unsigned char *)malloc(sample[f].length))==NULL)
          break;
        rewind(in);
        fread(sample[f].data,1,sample[f].length,in);
        fclose(in);
        }
      }
    }
  }
#endif

snd_main();
}


#else	/* !SOUND_SUPPORT */

/* need dummy queuesam/start_sndserv */

void queuesam(int chan,int sam)
{
}

void start_sndserv()
{
}

#endif	/* !SOUND_SUPPORT */
