/* virtual sound channels, used for mixing.
 * each can play at most one sample at a time.
 */
#define ABM_CHANNEL	0	/* player shot */
/* there are three explosion channels */
#define EXPLODE_CHAN1	1
#define EXPLODE_CHAN2	2
#define EXPLODE_CHAN3	3
#define EFFECT_CHANNEL	4	/* for level start/game over noises */
#define NUM_CHANNELS	5

/* sample offsets in sample[] */
#define ABM_SAMPLE	0	/* player shooting */
#define EXPLODE_SAMPLE	1	/* explosion */
#define LEVELST_SAMPLE	2	/* level start noise */
#define LEVELEND_SAMPLE	3	/* level end noise */
#define GAMEOVER_SAMPLE	4	/* played on game over */
#define CITYBANG_SAMPLE	5	/* city destroyed */
#define NUM_SAMPLES	6

#define KILL_SNDSERV	255	/* signal to sndserv to die */


extern void queuesam(int chan,int sam);
extern void start_sndserv(void);
