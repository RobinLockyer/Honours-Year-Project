// This is the new version with better efficiency

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <conio.h>
#include <string.h>
#define MAXINT           INT_MAX
#define getch            _getch
#include "myrand.h"

// P=200 and OPS=200,000 finds 2 solns for even-8
#define POPULATION_SIZE  90
#define MAX_OPS          25000

#define TOURN_SIZE       1
#define NUM_RUNS         100
#define TRUE             1
#define FALSE            0
#define LEFT             0
#define RIGHT            1

#define NUM_BITS         4
#define MAX_ARITY        2
#define NUM_TESTS        (1 << NUM_BITS) // 2^NUM_BITS
#define NUM_TERMINALS    NUM_BITS
#define NUM_FUNCTIONS    4

typedef unsigned short int popindex;

enum tokens {AND=NUM_BITS, OR, NAND, NOR};

struct token_entry
{
   char *name;
   int arity;
   // function
};

struct token_entry token_table [] =
     {
       {"D0 ", 0},
       {"D1 ", 0},
       {"D2 ", 0},
       {"D3 ", 0},
//       {"D4 ", 0},
//       {"D5 ", 0},
//       {"D6 ", 0},
//       {"D7 ", 0},
//       {"D8 ", 0},
//       {"D9 ", 0},
       {"AND ", 2},
       {"OR ", 2},
       {"NAND ", 2},
       {"NOR ", 2}
     };

struct prog
{
  char node;    // allow for string terminator
  int fitness;
  popindex preds[POPULATION_SIZE]; //predecessors
  popindex firstp;
  popindex succs[MAX_ARITY];
  char outputs[NUM_TESTS];
  int oldfitness;
  char oldoutputs[NUM_TESTS];
};

struct prog population [POPULATION_SIZE];

int best_prog;
int best_fitness;
int total_fitness;
char results[NUM_TESTS];
int correct, init_fitness = 0;

popindex ufirst;
popindex ulist[POPULATION_SIZE];



int evals = 0;

void test_fitness(int member)
{
   struct prog *progp = &population[member];
   int tests = NUM_TESTS;
   char *out = progp->outputs;
   char *res = results;
   int misses = 0;
   char *left = population[progp->succs[LEFT]].outputs;
   char *right = population[progp->succs[RIGHT]].outputs;

   //evals++;

   switch (progp->node)
   {
      case AND:   while (tests--)
                  {  *out = *left++ & *right++;
                     misses += (*out++ != *res++);
                  }
                  break;

      case OR:    while (tests--)
                  {  *out = *left++ | *right++;
                     misses += (*out++ != *res++);
                  }
                  break;

      case NAND:  while (tests--)
                  {  *out = !(*left++ & *right++);
                     misses += (*out++ != *res++);
                  }
                  break;

      case NOR:   while (tests--)
                  {  *out = !(*left++ | *right++);
                     misses += (*out++ != *res++);
                  }
                  break;

      default:    printf("Error in fitness fn.\n");
                  getch();
                  exit(1);
   }

   progp->fitness = misses;
   correct = (misses == 0);
}


void print_prog(int index)
{  struct prog *progp = &population[index];
   char token = progp->node;
   int ar = token_table[token].arity;
   fputs(token_table[token].name, stdout);
   if (ar > 0)
   {  putchar('(');
      for (int i=0; i < ar; i++)
         print_prog(progp->succs[i]);
      putchar(')');
   }
}

char nodes_used[POPULATION_SIZE];

void add_node(int index)
{   
	char token = population[index].node;
	popindex *s = population[index].succs;
	int ar = token_table[token].arity;

    nodes_used[index] = TRUE;
	for (int i=0; i < ar; i++)
		add_node(s[i]);
}

void show_graph(int index)
{
	int gsize = 0;
	
	// Make record of all nodes used
	memset(nodes_used, FALSE, POPULATION_SIZE);
	add_node(index);

	int pos = index;
	while (pos >= 0)
	{  
		if (nodes_used[pos])
		{  char token = population[pos].node;
	       popindex *s = population[pos].succs;
	       int ar = token_table[token].arity;

		   printf("%d: %s", pos, token_table[token].name); 
		   for (int i=0; i < ar; i++)
		       printf("%d ", s[i]);
		   putchar('\n');

		   gsize++;
		}

		pos--;
	}

	printf("No. nodes = %d\n", gsize);
}

int count_nodes(int index)
{  int gsize = 0;
   memset(nodes_used, FALSE, POPULATION_SIZE);
   add_node(index);

   int pos = index;
   while (pos >= 0)
   {
	   if (nodes_used[pos])
		   gsize++;
       pos--;
   }
   return (gsize);
}


void init_terms_and_results(void)
{  int i, j, misses;

   for (i=0; i < NUM_TESTS; i++)
   {  int temp, res, lsb;
      temp = i; res = 0;
      for (j=0; j < NUM_BITS; j++)
      { lsb = temp & 1;
        res ^= lsb;
        population[j].outputs[i] = (char) lsb;
        temp >>= 1;
      }
      results[i] = ~res & 1;
   }

   for (i=0; i < NUM_TERMINALS; i++)
   {  population[i].node = (char) i;
      population[i].firstp = 0;
      misses = 0;
      char *out = population[i].outputs;
      for (j=0; j < NUM_TESTS; j++)
         misses += (out[j] != results[j]);
      population[i].fitness = misses;
      init_fitness += misses;
   }
}

void add_ulist(int member)
{
   popindex *p = &ufirst;
   popindex next = *p;
   while (next && member > next)
   {  p = &ulist[next];
      next = *p;
   }
   if (member != next)  //if not already in list
   {  *p = (popindex) member;
      ulist[member] = next;
   }
}

void build_ulist(int member)
{  // Add this member to update list, then recursively add all its preds
   popindex next = population[member].firstp;
   popindex *plist = population[member].preds;
   add_ulist(member);
   while (next)
   {  add_ulist(next);
      build_ulist(next);
      next = plist[next];
   }
}

void add_pred(int member, int pred)
{
   if (member < NUM_TERMINALS) return;  // don't bother with pred list for terminals

   popindex *p = &population[member].firstp;
   popindex *plist = population[member].preds;
   popindex next = *p;
   while (next && pred > next)
   {  p = &plist[next];
      next = *p;
   }
   if (pred != next)  //if not already in list
   {  *p = (popindex) pred;
      plist[pred] = next;
   }
}


void del_pred(int member, int pred)
{  if (member < NUM_TERMINALS) return;  // don't bother with pred list for terminals

   popindex *p = &population[member].firstp;
   popindex *plist = population[member].preds;
   popindex next = *p;
   while (pred != next)
   {  p = &plist[next];
      next = *p;
   }
   *p = plist[pred];
}


#define random_function() ((char)(Randint(NUM_FUNCTIONS) + NUM_TERMINALS))

void initialise_population()
{
   struct prog *progp = &population[NUM_TERMINALS];
   int i, j;

   for (i=NUM_TERMINALS; i < POPULATION_SIZE; i++)
   {
      progp->node = random_function();
      progp->firstp = 0;

      int ar = token_table[progp->node].arity;

      for (j=0; j < ar; j++)
      {  int subtree = Randint(i);    // random for now
         progp->succs[j] = (popindex) subtree;
         add_pred(subtree, i);
      }

      test_fitness(i);
      total_fitness += population[i].fitness;
      progp++;
   }
}


int select_best(int limit)
{  int minfit = MAXINT;
   int best, cand;
   int n = TOURN_SIZE;
   while (n--)
   { cand = Randint(limit);
     if (population[cand].fitness < minfit)
     {   minfit = population[cand].fitness;
         best = cand;
     }
   }
   return (best);
}


int best_fit(void)
{  best_fitness = MAXINT;
   for (int i=0; i < POPULATION_SIZE; i++)
   {  if (population[i].fitness < best_fitness)
      {  best_fitness = population[i].fitness;
         best_prog = i;
      }
   }
   return best_fitness;
}

void do_updates(void)
{
   // Go through update list
   // Save state, update total fitness, test fitness
   popindex next = ufirst;
   while (next && !correct)
   {  struct prog *progp = &population[next];
      total_fitness -= progp->fitness;
      // Now save state. N.B. change this if not using char for outputs
      progp->oldfitness = progp->fitness;
      memcpy(progp->oldoutputs, progp->outputs, NUM_TESTS);

      test_fitness(next);
      total_fitness += progp->fitness;
      next = ulist[next];
   }
}


void restore(void)
{
   popindex next = ufirst;
   while (next)
   {  struct prog *progp = &population[next];

      // Now restore state. N.B. change this if not using char for outputs
      progp->fitness = progp->oldfitness;
      memcpy(progp->outputs, progp->oldoutputs, NUM_TESTS);
      next = ulist[next];
   }
}


void changetree(int target, int branch, int subtree, int oldsubtree)
{
   // The following needs to be made more general
   if (population[target].succs[LEFT] != population[target].succs[RIGHT])
      del_pred(oldsubtree, target);

   population[target].succs[branch] = subtree;
   add_pred(subtree, target);

}

void change()
{
   popindex target;
   popindex subtree;
   popindex oldsubtree;
   int branch = Randint(2);  // should be based on arity
   int oldfit = total_fitness;
   int oldbest = best_fit();

   do target = select_best(POPULATION_SIZE); while (target < NUM_TERMINALS);

   subtree = select_best(target);
   oldsubtree = population[target].succs[branch];

   changetree(target, branch, subtree, oldsubtree);

   ufirst = 0;
   build_ulist(target); // build update list
   do_updates();

   //if ((total_fitness > oldfit || best_fit() > oldbest) && !correct)
   if (total_fitness > oldfit  && !correct)
   {  changetree(target, branch, oldsubtree, subtree);
      restore();
      total_fitness = oldfit;
   }
}


void init_globals(void)
{
  best_prog = 0;
  best_fitness = MAXINT;
  correct = FALSE;
  total_fitness = init_fitness;
}


int main()
{
  int run; time_t start; int numsolns = 0;
  FILE *fp;

  if ((fp = fopen("SNGPeven4sizes90.csv", "w")) == NULL)
  {  fprintf(stderr, "Cannot open output file\n");
     exit(1);
  }

  //seed = time(NULL);
  seed = 1733469753;
  time(&start);
  init_terms_and_results();

  for (run=1; run <= NUM_RUNS; run++)
  {
    //int generation = 0;
	if (run%20 ==0)
    printf("\nSTARTING RUN %d...\n", run);

    init_globals();
    initialise_population();
    //show_trees(); getch();

    for (int n=0; n < MAX_OPS && !correct; n++)
    {
       change();
       best_fit();
      /* if (n % 1000 == 0)
       { printf("%d: Ave fit = %lf  Best fit = %d\n", n,
              (float)total_fitness/POPULATION_SIZE, best_fitness);
       }*/
    }
    if (correct)
    {  //print_prog(best_prog);  getch();
       //printf("Solns = %d\n", ++numsolns);
       //printf("Soln. is prog %d\n", best_prog);
	   //show_graph(best_prog); getch();
		numsolns++;
		fprintf(fp, "%d\n", count_nodes(best_prog));
    }
    //else printf("Best fitness = %d\n", best_fitness);
  }

  fclose(fp);
  printf("\nTIME=%d\n", time(NULL)-start);
  printf("Num solns = %d\n", numsolns);
  //printf("Fitness evals = %d\n", evals);
  puts("DONE");
  getch();
  return (0);
}

