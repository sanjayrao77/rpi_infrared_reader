#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define DEBUG
#include "common/conventions.h"

struct onecommand {
	char name100[100+1];
	unsigned int count;
	struct {
		unsigned int num,max;
		unsigned int *list;
	} data;
};
SCLEARFUNC(onecommand);

static void reset_onecommand(struct onecommand *o, char *name) {
char *temp;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
strncpy(o->name100,name,100);	
#pragma GCC diagnostic pop
temp=strchr(o->name100,'\n'); if (temp) *temp=0;
o->count=0;
o->data.num=0;
}

static unsigned int slowtou(char *str) {
unsigned int ret=0;
switch (*str) {
	case '1': ret=1; break; case '2': ret=2; break; case '3': ret=3; break; case '4': ret=4; break; case '5': ret=5; break;
	case '6': ret=6; break; case '7': ret=7; break; case '8': ret=8; break; case '9': ret=9; break; case '+': case '0': break;
	default: return 0; break;
}
while (1) {
	str++;
	switch (*str) {
		case '9': ret=ret*10+9; break; case '8': ret=ret*10+8; break; case '7': ret=ret*10+7; break;
		case '6': ret=ret*10+6; break; case '5': ret=ret*10+5; break; case '4': ret=ret*10+4; break;
		case '3': ret=ret*10+3; break; case '2': ret=ret*10+2; break; case '1': ret=ret*10+1; break;
		case '0': ret=ret*10; break; default: return ret; break;
	}
}
return ret;
}

static int alloc_data_onecommand(struct onecommand *o, unsigned int count) {
unsigned int m;
unsigned int *t;
if (count<=o->data.max) return 0;
m=count+500;
if (!(t=realloc(o->data.list,m*sizeof(unsigned int)))) GOTOERROR;
o->data.list=t;
o->data.max=m;
return 0;
error:
	return -1;
}

static int adddata_onecommand(struct onecommand *o, char *line) {
unsigned int commas=0,cursor=0;
unsigned int *list;
char *temp;
for (temp=line;*temp;temp++) if (*temp==',') commas++;
if (!o->data.num) {
	if (alloc_data_onecommand(o,commas)) GOTOERROR;
	o->data.num=commas;
} else if (o->data.num!=commas) {
	fprintf(stderr,"Line has different length than expected, %u!=%u\n",o->data.num,commas);
	GOTOERROR;
}

if (!o->count) {
	memset(o->data.list,0,commas*sizeof(unsigned int));
}

list=o->data.list;
temp=line;
while (1) {
	unsigned int ui,last,avg;
	ui=slowtou(temp);
	last=list[cursor];
	if (o->count) {
		avg=last/o->count;
		if ((ui>avg*1.1)||(ui<avg*0.9)) {
			fprintf(stderr,"Abnormal value, %u out of line with %u (%u / %u)\n",
					ui,avg,last,o->count);
			GOTOERROR;
		}
	}
	list[cursor]=last+ui;
	cursor++;
	temp=strchr(temp,',');
	if (!temp) break;
	temp++;
	if (*temp=='\n') break;
}

o->count+=1;
return 0;
error:
	return -1;
}

static int print_onecommand(struct onecommand *o, FILE *fout) {
unsigned int ui;
unsigned int *list;

if (!o->data.num) return 0;
fprintf(fout,"static int %s_ircode[]={",o->name100);
list=o->data.list;
ui=o->data.num;
while (1) {
	fprintf(fout,"%u,",(*list)/o->count);
	ui--;
	if (!ui) break;
	list++;
}
fprintf(fout,"0};\n");
return 0;
}

int main(int argc, char **argv) {
FILE *fin=NULL;
char oneline[512];
struct onecommand onecommand;
int linenum=0;

clear_onecommand(&onecommand);

if (!(fin=fopen("data.txt","r"))) GOTOERROR;
while (1) {
	if (!fgets(oneline,512,fin)) break;
	linenum++;
	if (oneline[0]=='\n') {
	} else if (oneline[0]=='#') {
	} else if (isalpha(oneline[0])) {
		(ignore)print_onecommand(&onecommand,stdout);
		(void)reset_onecommand(&onecommand,oneline);
	} else {
		if (adddata_onecommand(&onecommand,oneline)) {
			fprintf(stderr,"Error in input line %d\n",linenum);
			GOTOERROR;
		}
	}
}
(ignore)print_onecommand(&onecommand,stdout);

fclose(fin);
return 0;
error:
	iffclose(fin);
	return -1;
}
