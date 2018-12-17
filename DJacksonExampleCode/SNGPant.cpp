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

#define POPULATION_SIZE  20   // size 50 gives 16 solns with 25000 ops
// 40 gives 13; 30 gives 12; 20 gives 10; 10 gives 3
// 60 gives 17
#define MAX_OPS          25000

#define TOURN_SIZE       1
#define NUM_RUNS         100
#define TRUE             1
#define FALSE            0

#define MATRIX_SIZE      32
#define NUM_PELLETS      89
#define MAX_STEPS        600

#define MAX_ARITY        3
#define NUM_TERMINALS    3
#define NUM_FUNCTIONS    3

typedef unsigned short int popindex;

enum tokens {LEFT, RIGHT, MOVE, IF_FOOD_AHEAD, PROGN2, PROGN3};

struct token_entry
{
   char *name;
   int arity;
   // function
};

struct token_entry token_table [] =
     {
       {"LEFT ", 0},
       {"RIGHT ", 0},
       {"MOVE ", 0},
       {"IF_FOOD_AHEAD ", 2},
       {"PROGN2 ", 2},
       {"PROGN3 ", 3}
     };

struct prog
{
  char node;
  int fitness;
  popindex preds[POPULATION_SIZE]; //predecessors
  popindex firstp;
  popindex succs[MAX_ARITY];
  int oldfitness;
};

struct prog population [POPULATION_SIZE];

int best_prog;
int best_fitness;
int total_fitness;
int correct, init_fitness;

popindex ufirst;
popindex ulist[POPULATION_SIZE];

int evals = 0;

int trace=FALSE;
int row, col, next_row, next_col, heading, steps, pellets;

enum {NORTH, EAST, SOUTH, WEST};

char matrix[MATRIX_SIZE][MATRIX_SIZE];
char matrix_data[MATRIX_SIZE][MATRIX_SIZE] =
  { {0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0},
    {0,0,0,1,1,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
  };

char trace_matrix[MATRIX_SIZE][MATRIX_SIZE];

void print_matrix(void)
{  int i, j; char symbs[] = " OX";

   for (i=0; i < MATRIX_SIZE; i++)
   {  for (j=0; j < MATRIX_SIZE; j++)
          putchar(symbs[trace_matrix[i][j]]);
      putchar('\n');
   }
}

void evaltree(int mem)
{  if (pellets == 0 || steps > MAX_STEPS) return;

   struct prog *progp = &population[mem];
   popindex *s = progp->succs;

   switch (progp->node)
   {  case LEFT:    heading = (heading==NORTH? WEST : heading-1);
                    steps++;
                    return;

      case RIGHT:   heading = (heading+1) % 4;
                    steps++;
                    return;

      case MOVE:    switch (heading)
                    {  case NORTH:  if (row == 0) row = MATRIX_SIZE;
                                    row--;
                                    break;
                       case EAST:   col = (col+1) % MATRIX_SIZE;
                                    break;
                       case SOUTH:  row = (row+1) % MATRIX_SIZE;
                                    break;
                       case WEST:   if (col == 0) col = MATRIX_SIZE;
                                    col--;
                                    break;
                    }

                    if (trace)
                    {  trace_matrix[row][col] = 2;
                       print_matrix();
                       puts("Continue trace?");
                       trace = (getch() == 'y');
                    }

                    if (matrix[row][col])
                    {  matrix[row][col] = 0;
                       pellets--;
                    }
                    steps++;
                    return;

      case IF_FOOD_AHEAD:
                    switch (heading)
                    {  case NORTH:  next_row = (row == 0? MATRIX_SIZE-1: row-1);
                                    next_col = col;
                                    break;
                       case EAST:   next_row = row;
                                    next_col = (col+1) % MATRIX_SIZE;
                                    break;
                       case SOUTH:  next_row = (row+1) % MATRIX_SIZE;
                                    next_col = col;
                                    break;
                       case WEST:   next_row = row;
                                    next_col = (col == 0? MATRIX_SIZE-1: col-1);
                                    break;
                    }

                    if (matrix[next_row][next_col])
                    {  evaltree(s[0]);
                    }
                    else
                    {  
                       evaltree(s[1]);
                    }

                    return;

      case PROGN2:  evaltree(s[0]); evaltree(s[1]); return;

      case PROGN3:  evaltree(s[0]); evaltree(s[1]); evaltree(s[2]); return;
    
   }
}

void test_fitness(int member)
{  
   // Initialise the trail matrix
   memcpy(matrix, matrix_data, MATRIX_SIZE*MATRIX_SIZE);
   pellets = NUM_PELLETS;

   if (trace)
      memcpy(trace_matrix, matrix_data, MATRIX_SIZE*MATRIX_SIZE);

   // Position the ant
   row = 0; col = 0; heading = EAST; steps = 0;

   // Repeatedly evaluate tree until all pellets eaten or we time out
   do
   {  
      evaltree(member);
   }  while (pellets > 0 && steps < MAX_STEPS);


   //evals++;

   population[member].fitness = pellets;
   correct = (pellets == 0);
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
{  
   population[0].node = LEFT;
   population[0].fitness = 89;
   population[1].node = RIGHT;
   population[1].fitness = 89;
   population[2].node = MOVE;
   population[2].fitness = 86; // Repetitive moves east will find 3 pellets

   init_fitness = 264;
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
      // Now save fitness
      progp->oldfitness = progp->fitness;
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

      // Now restore fitness
      progp->fitness = progp->oldfitness;
      next = ulist[next];
   }
}


void changetree(int target, int branch, int subtree, int oldsubtree)
{
   char token = population[target].node;
   popindex *s = population[target].succs;
   int ar = token_table[token].arity;
   int unique = TRUE;

   for (int i = 0; i < ar; i++)
	   if (i != branch && s[i] == oldsubtree)
	   {   unique = FALSE;
           break;
   }

   if (unique)
      del_pred(oldsubtree, target);

   population[target].succs[branch] = subtree;
   add_pred(subtree, target);

}

void change()
{
   popindex target;
   popindex subtree;
   popindex oldsubtree;
   int oldfit = total_fitness;
   int oldbest = best_fit();

   do target = select_best(POPULATION_SIZE); while (target < NUM_TERMINALS);

   int ar = token_table[population[target].node].arity;
   int branch = Randint(ar);

   subtree = select_best(target);
   oldsubtree = population[target].succs[branch];

   changetree(target, branch, subtree, oldsubtree);

   ufirst = 0;
   build_ulist(target); // build update list
   do_updates();

   //if ((total_fitness > oldfit || best_fit() > oldbest) && !correct)
   //if (total_fitness > oldfit  && !correct)
   if (best_fit() > oldbest && !correct)
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
  //FILE *fp;

  //if ((fp = fopen("sizes90.csv", "w")) == NULL)
  //{  fprintf(stderr, "Cannot open output file\n");
  //   exit(1);
  //}

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
       } */
    }
    if (correct)
    {  //print_prog(best_prog);  //getch();
       //printf("Solns = %d\n", ++numsolns);
       //printf("Soln. is prog %d\n", best_prog);
	   show_graph(best_prog); //getch();
	   numsolns++;
	   //fprintf(fp, "%d\n", count_nodes(best_prog));

	   /* puts("See path of best prog?");
       if (getch() == 'y')
       {   trace = TRUE;
           test_fitness(best_prog);
           trace = FALSE;
       }  */
    }
    //else printf("Best fitness = %d\n", best_fitness);
  }

  //fclose(fp);
  printf("\nTIME=%d\n", time(NULL)-start);
  printf("Num solns = %d\n", numsolns);
  //printf("Fitness evals = %d\n", evals);
  puts("DONE");
  getch();
  return (0);
}

