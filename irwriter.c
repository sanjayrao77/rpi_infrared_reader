#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/io.h>

#include <pigpio.h>

#define DEBUG
#include "common/conventions.h"
#include "ircode.h"

#define RPIPIN	26

static uint64_t u64time(uint64_t baseseconds) {
struct timespec ts;
uint64_t t;
if (clock_gettime(CLOCK_MONOTONIC_RAW,&ts)) return 0;
t=((ts.tv_sec-baseseconds)*1000*1000*1000)+(ts.tv_nsec);
return t;
}

int testtiming(void) {
uint64_t baseseconds;
unsigned int ui;

baseseconds=u64time(0)/1000*1000*1000;
for (ui=0;ui<10;ui++) {
	uint64_t start,stop;
	unsigned int ui;
	start=u64time(baseseconds);
	for (ui=0;ui<10000;ui++) {
		gpioWrite(RPIPIN,1);
		gpioDelay(12);
		gpioWrite(RPIPIN,0);
		gpioDelay(12);
	}
	stop=u64time(baseseconds);
	fprintf(stderr,"High test: %"PRIu64" nanoseconds elapsed, %f per cycle (goal: 26316)\n",stop-start,(double)(stop-start)/10000.0);
	sleep(1);
}
baseseconds=u64time(0)/1000*1000*1000;
for (ui=0;ui<10;ui++) {
	uint64_t start,stop;
	unsigned int ui;
	start=u64time(baseseconds);
	for (ui=0;ui<10000;ui++) {
		gpioDelay(25);
	}
	stop=u64time(baseseconds);
	fprintf(stderr,"Low test: %"PRIu64" nanoseconds elapsed, %f per cycle (goal: 26316)\n",stop-start,(double)(stop-start)/10000.0);
	sleep(1);
}
return 0;
}

static void pulsepin(int pin, int micros) {
while (micros>=26) {
	unsigned int n;
	gpioWrite(RPIPIN,1);
	n=gpioDelay(12);
	if (n>24) n=1;
	else n=25-n;
	gpioWrite(RPIPIN,0);
	gpioDelay(n);
	micros-=26;
}
}
static void sleeppin(int micros) {
gpioDelay(micros);
}

static int sendcode(int pin, int *timing) {
uint64_t baseseconds,start,stop;
int32_t expected=0;
baseseconds=u64time(0)/1000*1000*1000;
start=u64time(baseseconds);
gpioWrite(RPIPIN,0);
while (1) {
	int i1,i2;
	i1=*timing;
	if (!i1) break;

	(void)pulsepin(pin,i1);
	expected+=i1;

	timing++;
	i2=*timing;
	if (!i2) break;

	(void)sleeppin(i2);
	expected+=i2;

	timing++;
}
stop=u64time(baseseconds);
fprintf(stderr,"Elapsed microseconds:%f, expected:%d\n",(double)(stop-start)/1000.0,expected);
gpioWrite(RPIPIN,0);
return 0;
}

int main(int argc, char **argv) {
int isloaded_pigpio=0;

if (gpioInitialise()<0) GOTOERROR;
isloaded_pigpio=1;

if (gpioSetMode(RPIPIN,PI_OUTPUT)) GOTOERROR;
gpioWrite(RPIPIN,0);

// (ignore)testtiming();

if (sendcode(RPIPIN,mute_sony_ircode)) GOTOERROR;

gpioTerminate();
return 0;
error:
	if (isloaded_pigpio) gpioTerminate();
	return -1;
}
