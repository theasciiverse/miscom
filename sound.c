#ifdef SOUND_SUPPORT

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#if defined(__has_include)
#  if __has_include(<sys/soundcard.h>)
#    include <sys/soundcard.h>
#  elif __has_include(<linux/soundcard.h>)
#    include <linux/soundcard.h>	/* musl has no sys/soundcard.h */
#  endif
#else
#  include <sys/soundcard.h>
#endif
#ifdef AV_BUILD
#include <signal.h>
#include <string.h>
#include <time.h>
#endif
#include "sound.h"

int sound=1;
int soundfd=-1;
static int paced=0;	/* output to a pipe, kept to time here */
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

#ifdef AV_BUILD
if(paced)
  {
  /* a pipe to aplay takes the sound as fast as it is written: keep to
   * the sound's own speed (8000 bytes a second) */
  static struct timespec next;
  struct timespec now;

  clock_gettime(CLOCK_MONOTONIC,&now);
  if(next.tv_sec==0 || now.tv_sec>next.tv_sec+1)
    next=now;
  next.tv_nsec+=(long)sound_bufsiz*125000L;	/* 1e9/8000 a byte */
  while(next.tv_nsec>=1000000000L)
    next.tv_nsec-=1000000000L,next.tv_sec++;
  if(write(soundfd,soundbuf,sound_bufsiz)<0)
    sound=0;	/* aplay is not there, or has gone */
  clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&next,NULL);
  return;
  }
#endif
write(soundfd,soundbuf,sound_bufsiz);
}


void snd_main()
{
unsigned char buf[2];
int live=1;

while(live)
  {
  int n;

  while((n=read(sndpipe[0],buf,2))!=-1)
    {
    if(n==0)
      {
      /* the game has gone without saying so (killed, or crashed): the
       * pipe has ended, as this process closed its own end of it */
      live=0;
      break;
      }
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

/* the game's end of the pipe: closed here, so that the pipe ends when
 * the game goes and this process ends with it (it used to stay behind,
 * holding the sound device, until the terminal was closed) */
close(sndpipe[1]);
sound_bufsiz=BASE_SOUND_BUFSIZ;
#ifndef SNDCTL_DSP_SETFRAGMENT		/* voxware 3 only */
sound=0;
soundfd=-1;
#else
if(sound)
  {
  soundfd=open("/dev/dsp",O_WRONLY);
#ifdef AV_BUILD
  /* no OSS device (most Linux systems today): play through ALSA's
   * aplay, which also reaches PipeWire and PulseAudio */
  if(soundfd<0)
    {
    FILE *aplay;

    signal(SIGPIPE,SIG_IGN);
    aplay=popen("exec aplay -q -t raw -f U8 -r 8000 -c 1 "
                "--buffer-time=60000 2>/dev/null","w");
    if(aplay!=NULL)
      {
      soundfd=fileno(aplay);
      paced=1;
      }
    }
#endif
  if(soundfd<0)
    {
    sound=0;
    soundfd=-1;
    }
  else
    {
    int frag;
    FILE *in;
    char buf[4352];
#ifdef AV_BUILD
    char dir[4096];

    /* the samples in sounds/ beside the program */
    if(!av_exedir(dir,sizeof dir))
      strcpy(dir,".");
    strcat(dir,"/sounds");
#else
    char const *dir=SOUNDSDIR;
#endif
  
#ifdef MORE_SOUNDBUF
    frag=(0x40000|BASE_SOUND_FRAG_PWR);
#else
    frag=(0x20000|BASE_SOUND_FRAG_PWR);
#endif
    if(!paced)
      ioctl(soundfd,SNDCTL_DSP_SETFRAGMENT,&frag);
  
    /* load in the samples */
    for(f=0;f<NUM_SAMPLES;f++)
      {
      sprintf(buf,"%s/%s",dir,samplename[f]);
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
