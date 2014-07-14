/* 
 * Name: Prajwal Yadapadithaya
 * Andrew ID: pyadapad
 * */
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>
#include "cachelab.h"

struct line {
  int valid;
  unsigned tag;
  int pri;
  struct line *next;
};

/* Function to print a particular set in the cache (For debugging)*/
void printLine(struct line *start, int setIndex) {
  while(start != NULL) {
    printf("Tag: %x\tPri: %x\t",start->tag, start->pri);
    start = start->next;
  }
  printf("\n");
}

/*This is the case of eviction, to replace a particular line in a set based on the LRU priority*/
void replaceInLine(struct line *start, int pri, int newpri, int newtag) {
  struct line *temp = start;
  while(temp != NULL) {
    if(temp->pri == pri) {
      temp->pri = newpri;
      temp->tag = newtag;
      break;
    }
    temp = temp->next;
  }
}

/*This is to insert a new line in a set. i.e, when the set is not full*/
void insertInLine(struct line *start, int tag, int pri) {
  struct line *temp = malloc(sizeof(struct line));
  temp->tag = tag;
  temp->pri = pri;
  temp->valid = 1;
  while(start->next != NULL)
    start = start->next;
  start->next = temp;
  temp->next = NULL;
}

int main(int argc, char **argv) {
  int s = -1, E = -1, b = -1;
  int hits = 0, misses = 0, evictions = 0;
  int c;
  char *t;
  FILE *file;
  while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
    switch(c) {
      case 's':
	s = atoi(optarg); break;
      case 'E':
	E = atoi(optarg); break;
      case 'b':
	b = atoi(optarg); break;
      case 't':
	t = optarg; break;
      case 'v':
	printf("verbose option not implemented. Sorry!\n"); break;
      case 'h':
	printf("For help, please refer the cachelab writeup!\n"); break;
      case '?':
	break;
    }
  }
  
  if (s < 0 || E < 0 || b < 0 || t == NULL){
    printf("Invalid inputs. Please try with valid inputs\n");
    exit(0);
  }

  file = fopen(t, "r");
  if(file == NULL) {
    printf("Cannot open file. Please check if the filename entered is correct.\n");
    exit(0);
  }
  char id;
  unsigned addr;
  int size;

  int S = 1<<s;
  struct line *cache[S];
  static int initSet[32];

  while(fscanf(file, " %c %x,%d", &id, &addr, &size) > 0) {
    if(id == 'I')
      continue;

    //Compute setIndex and tag for the memory address
    int setIndex = (S - 1) & (addr >> b);
    unsigned tag = ((1 << (64 - (s + b))) - 1) & (addr >> (s + b));
    
    //Case 1: Set is empty
    if(initSet[setIndex] == 0) {
      struct line *temp = malloc(sizeof(struct line));
      initSet[setIndex]++;
      misses++;
      temp->tag = tag;
      temp->pri = initSet[setIndex];
      temp->valid = 1;
      temp->next = NULL;
      cache[setIndex] = temp;
      if (id == 'M')
	 hits++;
      continue;
    }

    //Case 2: Set is not empty
    struct line *temp = cache[setIndex];
    int count = 0; int hit = 0; int minPri = initSet[setIndex];
    while(temp != NULL && count < E) {
      if(temp->tag == tag) {
	hit = 1; hits++;
 	temp->pri = ++initSet[setIndex]; //Update the LRU priority in the case of a hit
	break;
      }
      if(temp->pri < minPri)
	minPri = temp->pri;
      temp = temp->next;
      count++;
    }
    if(hit == 1) {
      if (id == 'M')//Adding extra hit for the 'M' case
	hits++;
      continue;
    }
    if(count < E && hit == 0) {
      insertInLine(cache[setIndex], tag, ++initSet[setIndex]); //In the case of a miss with set not full, insert a new line.
      misses++;
    }
    if(count == E){
      replaceInLine(cache[setIndex], minPri, ++initSet[setIndex], tag); //In the case of a miss with set full, replace the line based on LRU priority
      misses++;
      evictions++;
    }

    if (id == 'M')
       hits++;
  }

  printSummary(hits, misses, evictions);
  return 0;
}

