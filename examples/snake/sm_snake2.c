#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "../../src/d2mce.h"

#define UP      105
#define DOWN    107
#define LEFT    106
#define RIGHT   108
#define QUIT    113

#define WIDTH     50
#define HEIGHT    20
#define STARTUP   1
#define STARTLEFT 1

#define FALSE   0
#define TRUE    1

#define SNAKEBODY   48
#define SNAKELENGTH 5

#define LUCORNER 47
#define LDCORNER 92
#define RUCORNER 92
#define RDCORNER 47
#define HWALL    124
#define WWALL    45

#define F1	1000
#define P1      1001
#define S1      1002
#define P2	1100
#define S2	1101
#define PRO	0666
#define SNAKESIZE	WIDTH*HEIGHT
#define FOODSIZE	6
#define PLAYERSIZE	6
typedef struct struct_point {
	short x;
	short y;
}SNAKE;

typedef struct struck_food {
	short x;
	short y;
	char eat;
}FOOD;

typedef struct struct_player {
	int length;
	int score;
	SNAKE body[WIDTH*HEIGHT];
}PLAYER;

void init();
void *Keydown(void *arg);
void Playgame();
void PaintFood();
void PaintSnake();
void PaintWall();
void PaintScore();
//void moveSnake(SNAKE *snake,PLAYER *player);
void moveSnake(PLAYER *player);
void gameOver();
int CheckHitSnake();
//void CheckEat(SNAKE *snake,PLAYER *player);
void CheckEat(PLAYER *player);
//int CheckHit(SNAKE *snake,PLAYER *player);
int CheckHit(PLAYER *player1, PLAYER *player2);

int kbhit();


FOOD *food;
//SNAKE snakea1[WIDTH*HEIGHT],snakea2[WIDTH*HEIGHT];
//SNAKE *snake1,*snake2;
//PLAYER p1,p2;
PLAYER *player1,*player2;
int key;
int node_id;
d2mce_mutex_t m1;
d2mce_mutex_t m2;
d2mce_barrier_t b1;

int main()
{
	srand(time(NULL));
	init();
	Playgame();
	gameOver();
	return 0;
}
void init()
{
	int i;
	initscr();
	noecho();
	d2mce_init();
	node_id =d2mce_join("snake", "eps", 0);
	d2mce_mutex_init(&m1,"m1");
	d2mce_mutex_init(&m2,"m2");
	d2mce_barrier_init(&b1,"b1");
	player1 = d2mce_malloc("p1", sizeof(PLAYER));
	player2 = d2mce_malloc("p2", sizeof(PLAYER));
	food = d2mce_malloc("food", sizeof(FOOD));
	print_info();
	if (node_id==0) {
		player1->score=0;
		player1->length=SNAKELENGTH;
		for (i=0;i<player1->length;i++) {
			player1->body[i].x=STARTLEFT+WIDTH/2-i;
			player1->body[i].y=STARTUP+HEIGHT/2;
		}
		d2mce_store(player1);
	} else if(node_id ==1){
        player2->score=0;
        player2->length=SNAKELENGTH;
		for (i=0;i<player2->length;i++) {
			player2->body[i].x=STARTLEFT+WIDTH/2-i;
			player2->body[i].y=STARTUP+HEIGHT/2;
		}
		d2mce_store(player2);
	}
//	d2mce_barrier(&b1, 2);
//	d2mce_load(player1);
//	d2mce_load(player2);

	key=RIGHT;
	food->eat=TRUE;
}

void Playgame()
{
	int reg;
	pthread_t thread1;
	clear();
	reg=pthread_create(&thread1, NULL, Keydown,NULL);
	if (node_id==0) {
		while (key!=QUIT) {
			clear();
			PaintWall();
			PaintScore();
			PaintFood();
			moveSnake(player1);
			d2mce_mutex_lock(&m1);
			d2mce_load(player2);
		//	d2mce_store(player2);
			d2mce_store(player1);
			d2mce_mutex_unlock(&m1);
			CheckEat(player1);
			if (CheckHit(player1, player2))
				break;
			PaintSnake();
			move(STARTUP+HEIGHT,STARTLEFT+WIDTH+2);
			refresh();
			usleep(200000);
		}
	} else if(node_id==1) {
		while (key!=QUIT) {
			clear();
			PaintWall();
			PaintScore();
			PaintFood();
			moveSnake(player2);
			d2mce_mutex_lock(&m1);
			d2mce_load(player1);
			//d2mce_store(player1);
			d2mce_store(player2);
			d2mce_mutex_unlock(&m1);
			CheckEat(player2);
			if (CheckHit(player2, player1))
				break;
			PaintSnake();
			move(STARTUP+HEIGHT,STARTLEFT+WIDTH+2);
			refresh();
			usleep(200000);
		}

	}

}
void gameOver()
{
	move(STARTUP+HEIGHT/2,STARTLEFT+WIDTH/2-4);
	addstr("Game Over");
	refresh();
	d2mce_finalize();

	getch();
	endwin();
}
void PaintFood()
{
	d2mce_mutex_lock(&m2);
	d2mce_load(food);
	if (food->eat) {
		move( food->y, food->x);
		addch(' ');
		while (1) {
			food->x= rand()%(WIDTH-3)+STARTLEFT+1;
			food->y= rand()%(HEIGHT-3)+STARTUP+1;
			if (!CheckHitSnake())
				break;
		}
		food->eat=FALSE;
		d2mce_store(food);
	}
	d2mce_mutex_unlock(&m2);

	//d2mce_barrier(&b1, d2mce_getNodeNum());
//	d2mce_load(food);
	move( food->y, food->x);
	addch('@');
}
void PaintSnake()
{
	int i;
	for (i=0;i<player1->length;i++) {
		move( player1->body[i].y, player1->body[i].x);
		addch(SNAKEBODY);
	}

	if (player2->length>0) {
		for (i=0;i<player2->length;i++) {
			move( player2->body[i].y, player2->body[i].x);
			addch(SNAKEBODY);
		}
	}
}
void PaintWall()
{
	int i;
	int up=STARTUP;
	int down=up+HEIGHT;
	int left=STARTLEFT;
	int right=left+WIDTH;


	for (i=(left+1); i<=(right-1); i++) {
		move(up, i);
		addch( WWALL);
		move(down, i);
		addch( WWALL);
	}
	for (i=(up+1); i<=(down-1); i++) {
		move(i, left);
		addch(HWALL);
		move(i, right);
		addch(HWALL);
	}
	move(up, left);
	addch( LUCORNER);
	move(up, right);
	addch(RUCORNER);
	move(down, left);
	addch(LDCORNER);
	move(down, right);
	addch( RDCORNER);

}
void PaintScore()
{
	move(STARTUP+2, STARTLEFT+WIDTH+2);
	printw("Player1: %d" , player1->score);
	move(STARTUP+3, STARTLEFT+WIDTH+2);
	printw("Player2: %d" , player2->score);
}
void *Keydown(void *arg)
{
	char keydown;
	while (1) {
		if (kbhit()) {
			keydown=getch();
			if (keydown==UP && key!=DOWN)
				key=UP;
			else if (keydown==DOWN && key!=UP)
				key=DOWN;
			else if (keydown==LEFT && key!=RIGHT)
				key=LEFT;
			else if (keydown==RIGHT && key!=LEFT)
				key=RIGHT;
			else if (keydown==QUIT)
				key=QUIT;
		}
		if (key==QUIT)
			break;
		usleep(50000);
	}
	pthread_exit("1");
}
void moveSnake(PLAYER *player)
{
	SNAKE next;
	int i;
	switch (key) {
	case UP:
		next.x=player->body[0].x;
		next.y=player->body[0].y-1;
		break;
	case DOWN:
		next.x=player->body[0].x;
		next.y=player->body[0].y+1;
		break;
	case LEFT:
		next.x=player->body[0].x-1;
		next.y=player->body[0].y;
		break;
	case RIGHT:
		next.x=player->body[0].x+1;
		next.y=player->body[0].y;
		break;
	}
	for (i=player->length-1 ;i>0 ;i--) {
		player->body[i].x=player->body[i-1].x;
		player->body[i].y=player->body[i-1].y;
	}
	player->body[0].x=next.x;
	player->body[0].y=next.y;
}
int CheckHitSnake()//food check
{
	int i;

	for (i=0;i<=player1->length;i++) {
		if (food->x==player1->body[i].x && food->x==player1->body[i].y)
			return TRUE;
	}
	for (i=0;i<=player2->length;i++) {
		if (food->x==player2->body[i].x && food->x==player2->body[i].y)
			return TRUE;
	}
	return FALSE;
}
void CheckEat(PLAYER *player)
{
	if (food->x==player->body[0].x && food->y==player->body[0].y) {
		player->score+=10;
		food->eat=TRUE;
	//	d2mce_store(food);
		player->length=player->length+1;
	}
}
int CheckHit(PLAYER *player1, PLAYER *player2)
{
	int i;
	if (player1->body[0].x <= STARTLEFT || player1->body[0].x>=(STARTLEFT+WIDTH))
		return TRUE;
	if (player1->body[0].y <= STARTUP || player1->body[0].y>=(STARTUP+HEIGHT))
		return TRUE;
	for (i=1 ;i<player1->length;i++) {
		if (player1->body[0].x== player2->body[i].x && player1->body[0].y==player2->body[i].y)
			return TRUE;
	}
	return FALSE;
}
int kbhit()
{
	int n;
	ioctl(0, FIONREAD, &n);
	return n;
}


