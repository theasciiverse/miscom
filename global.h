extern int Screen[80][40];
extern int CtyPos[];
extern int split_prob[],smart_prob[];

extern int Level;
extern int Score;
extern int Ammo[3];
extern int Cities[20];
extern int Delay;
extern int keyptr;
extern int num_missiles;

struct Missile {
	int active;
	int fromx;
	int fromy;
	int x;
	int y;
	int speed;
	int spc;
	int tx;
	int ty;
	int mis_type;
	int city;
	int mis_subtype;
	};

struct Fireball {
	int x;
	int y;
	int radius;
	int countdown;
	int active;
	int exp_type;
	};

struct direction {
	int dx;
	int dy;
	int finished;
	};

#define MAXMISSILES (100)
#define ICBM (532)
#define ABM (234)
#define NUKE (432)
#define NO_NUKE (123)
#define MAX_X (71)
#define MAX_Y (40)
#define SMART (56)
#define DULL (0)
#define LEVEL_COMPLETE (543)
#define LEVEL_FAILED (6765)
#define LEVEL_QUIT (4321)

#define CURHOME_X (30)
#define CURHOME_Y (23)

#define REFORM --

extern struct Missile missiles[MAXMISSILES];
extern struct Fireball fireballs[MAXMISSILES];

 
