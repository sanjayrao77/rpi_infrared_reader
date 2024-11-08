#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>

#include <pigpio.h>

#define DEBUG
#include "common/conventions.h"

#define PIN_IRSENSOR 17

struct oneevent {
	uint32_t timestamp;
	int ison;
	struct oneevent *next;
};

static struct oneevent *first,*last,*firstfree;
static pthread_mutex_t biglock=PTHREAD_MUTEX_INITIALIZER;

static struct oneevent *alloc_oneevent(void) {
struct oneevent *oe;
oe=firstfree;
if (oe) {
	firstfree=oe->next;
	return oe;
}
oe=malloc(sizeof(struct oneevent));
return oe;
}

static int addevent(uint32_t timestamp, int ison) {
struct oneevent *oe;
if (!(oe=alloc_oneevent())) GOTOERROR;
oe->timestamp=timestamp;
oe->ison=ison;
oe->next=NULL;
if (!first) {
	first=last=oe;
} else {
	last->next=oe;
	last=oe;
}
return 0;
error:
	return -1;
}

static void callback(int gpio, int level, uint32_t tick) {
int ison;

ison=0;
if (level) {
	ison=1;
	if (level==2) {
		ison=2;
	}
}
// fprintf(stderr,"%d: %d %u\n",gpio,level,tick);
pthread_mutex_lock(&biglock);
	(ignore)addevent(tick,ison);
pthread_mutex_unlock(&biglock);
}

int main(int argc, char **argv) {
int isloaded_pigpio=0;
FILE *fout=NULL;
int ourlevel;

if (!(fout=fopen("/tmp/irreader.log","ab"))) GOTOERROR;

if (gpioInitialise()<0) GOTOERROR;
isloaded_pigpio=1;

if (gpioSetMode(PIN_IRSENSOR,PI_INPUT)) GOTOERROR;
if (gpioSetPullUpDown(PIN_IRSENSOR,PI_PUD_OFF)) GOTOERROR;
if (gpioSetAlertFunc(PIN_IRSENSOR,callback)) GOTOERROR;
ourlevel=gpioRead(PIN_IRSENSOR);
fprintf(stderr,"Starting at level %d\n",ourlevel);

while (1) {
	struct oneevent *myfirst,*mylast;
	sleep(2);
	pthread_mutex_lock(&biglock);
		myfirst=first;
		first=last=NULL;
	pthread_mutex_unlock(&biglock);
	if (!myfirst) {
		fprintf(stderr,"Ready for button..\n");
		continue;
	}


	if (0) {
		struct oneevent *oe;
		int count=0;
		for (oe=myfirst;oe;oe=oe->next) {
			ourlevel=ourlevel^1;
			if (ourlevel!=oe->ison) fprintf(stderr,"%d,",oe->ison);
			count++;
		}
		fprintf(stderr,"%d flips\n",count);
	}

	if (1) {
		struct oneevent *oe;
		uint32_t basetime;

		basetime=myfirst->timestamp;
		ourlevel=ourlevel^1;
		for (oe=myfirst->next;oe;oe=oe->next) {
//			fprintf(stdout,"%c:%.3f, ",(oe->ison)?'1':'0',(double)(oe->timestamp-basetime)/1000000.0);
			ourlevel=ourlevel^1;
			if (oe->ison!=ourlevel) fprintf(stderr,"missed event! %d\n",oe->ison);
			fprintf(stdout,"%u,",(oe->timestamp-basetime));
			fprintf(fout,"%u,",(oe->timestamp-basetime));
			basetime=oe->timestamp;
		}
		fprintf(stdout,"\n");
		fprintf(fout,"\n");
		fflush(fout);
	}

	for (mylast=myfirst;mylast->next;mylast=mylast->next);
	pthread_mutex_lock(&biglock);
		if (!firstfree) {
			firstfree=myfirst;
		} else {
			mylast->next=firstfree;
			firstfree=myfirst;
		}
	pthread_mutex_unlock(&biglock);
}

fclose(fout);
gpioTerminate();
return 0;
error:
	iffclose(fout);
	if (isloaded_pigpio) gpioTerminate();
	return -1;
}
