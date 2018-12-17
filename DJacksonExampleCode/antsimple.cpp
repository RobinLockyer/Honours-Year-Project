// Genetic programming code to solve the Santa Fe artificial ant problem
// Copyright David Jackson   2005

#pragma hdrstop
#include <condefs.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mem.h>
#include <conio.h>
#include <values.h>
#include <string.h>


#define POPULATION_SIZE  500
#define PARTITION_SIZE   (POPULATION_SIZE/(MAX_INIT_DEPTH - 1))
// Need to ensure that pop. size is a precise multiple of (MAX_INIT_DEPTH-1)

#define PCROSS           0.818
#define PROB_TERM        0.1
#define MAX_GEN          50
#define TOURN_SIZE       5
#define MAX_PROG_SIZE    1000
#define NUM_RUNS         100
#define TRUE             1
#define FALSE            0

#define MAX_INIT_DEPTH   6    // Max init size = 364 if all fns triadic

#define MATRIX_SIZE      32
#define NUM_PELLETS      89
#define MAX_STEPS        600

#define NUM_TERMINALS    3
#define NUM_FUNCTIONS    3

int    seed;

int intrnd () /* 1<=seed<=m */
{
  double const a    = 16807;      /* ie 7**5 */
  double const m    = 2147483647; /* ie 2**31-1 */

  double temp = seed * a;
  seed = (int) (temp - m * floor ( temp / m ));
  return seed;
}


double rand_0to1()
{ return ( intrnd() / 2147483647.0 ); }


double random_number()
{
  double r;
  while ((r = rand_0to1() ) >= 1.0 ) ; /* ensure we don't return 1.0 */

  return ( r );

} /* end random_number()*/

#define Randint(UPPER) ((int)(random_number()*(UPPER)))

enum tokens {DUMMY, LEFT, RIGHT, MOVE, IF_FOOD_AHEAD, PROGN2, PROGN3};

struct token_entry
{
   char *name;
   int arity_sub1;
   // function
};

struct token_entry token_table [] =
     { {"# ", -1},     // String end. Arity must be -1 for sub-string search
       {"LEFT ", -1},
       {"RIGHT ", -1},
       {"MOVE ", -1},
       {"IF_FOOD_AHEAD ", 1},
       {"PROGN2 ", 1},
       {"PROGN3 ", 2}
     };


struct prog
{
  char code[MAX_PROG_SIZE+1];    // allow for string terminator
  int proglen;
  int fitness;
};

struct prog population [POPULATION_SIZE];

int best_prog;
int smallest_proglen;
int best_fitness;


int correct; int trace=FALSE;

char *nodep;
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


void output_program(struct prog *progp)
{
  char *codep = progp->code;
  char token;

  while (token = *codep++)
     fputs(token_table[token].name, stdout);
  putchar('\n');
}


void print_matrix(void)
{  int i, j; char symbs[] = " OX";

   for (i=0; i < MATRIX_SIZE; i++)
   {  for (j=0; j < MATRIX_SIZE; j++)
          putchar(symbs[trace_matrix[i][j]]);
      putchar('\n');
   }
}


void skip_arg(void)
{  int count = 0;
   while ((count += token_table[*nodep++].arity_sub1) >= 0)   ;
}

void evaltree(void)
{  if (pellets == 0 || steps > MAX_STEPS) return;

   switch (*nodep++)
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
                    {  evaltree();
                       skip_arg();
                    }
                    else
                    {  skip_arg();
                       evaltree();
                    }

                    return;

      case PROGN2:  evaltree(); evaltree(); return;

      case PROGN3:  evaltree(); evaltree(); evaltree(); return;
    
   }
}


void test_fitness(int index)
{
  // Evaluate fitness of the indexed individual

  struct prog *progp = &population[index];
  int count = progp->proglen;

  // Initialise the trail matrix
  memcpy(matrix, matrix_data, MATRIX_SIZE*MATRIX_SIZE);
  pellets = NUM_PELLETS;

  if (trace)
     memcpy(trace_matrix, matrix_data, MATRIX_SIZE*MATRIX_SIZE);

  // Position the ant
  row = 0; col = 0; heading = EAST; steps = 0;

  // Repeatedly evaluate tree until all pellets eaten or we time out
  do
  {  nodep = progp->code;
     evaltree();
  }  while (pellets > 0 && steps < MAX_STEPS);

  correct = (pellets == 0);
  progp->fitness = pellets;

  if (pellets < best_fitness ||
     (pellets == best_fitness && count < smallest_proglen))
  {  best_prog = index;
     best_fitness = pellets;
     smallest_proglen = count;
  }
}

#define random_terminal() ((char)(Randint(NUM_TERMINALS) + 1))

#define random_function() ((char)(Randint(NUM_FUNCTIONS) + NUM_TERMINALS + 1))

#define random_token()  ((char)(Randint(NUM_FUNCTIONS+NUM_TERMINALS) + 1))

char *create_tree(char *tree, int depth, int full)
{  char token; int arity_m1, i;
   if (depth == 1)
      *tree++ = random_terminal();
   else
   {  *tree++ = token = (full ? random_function() : random_token());
      arity_m1 = token_table[token].arity_sub1;
      for (i=0; i <= arity_m1; i++)
         tree = create_tree(tree, depth-1, full);
   }
   return(tree);
}


int duplicate(struct prog *progp)
{  struct prog *nextprog = population;
   char *codep = progp->code;
   while (nextprog != progp)
   {  if (strcmp(nextprog->code, codep) == 0)
         return (TRUE);
      nextprog++;
   }
   return (FALSE);
}


void initialise_population()
{
  struct prog *progp = population;
  int i, j, depth, full, attempts;
  char *end_tree;

  for (i = 2; i <= MAX_INIT_DEPTH; i++)
  {
     for (j = 0; j < PARTITION_SIZE; j++)
     {  depth = i; full = j%2; attempts=0;
        do
        {
           end_tree = create_tree(progp->code, depth, full);
           *end_tree = '\0';
           progp->proglen = strlen(progp->code);
           attempts++;
           if (attempts >= 5 && depth < MAX_INIT_DEPTH)
           { attempts = 0; depth++;
           }
        } while (duplicate(progp));
        progp++;
     }
  }

  for (i=0; i < POPULATION_SIZE && !correct; i++)
     test_fitness(i);
}


int subtree_end(char tree[], int index)
{  int count = 0;
   while ((count += token_table[tree[index]].arity_sub1) >= 0) index++;
   return (index);
}

int select_best(void)
{  int minfit = MAXINT;
   int best, cand;
   int n = TOURN_SIZE;
   while (n--)
   { cand = Randint(POPULATION_SIZE);
     if (population[cand].fitness < minfit)
     {   minfit = population[cand].fitness;
         best = cand;
     }
   }
   return (best);
}


int select_worst(void)
{  int maxfit = -1;
   int worst, cand;
   int n = TOURN_SIZE;
   while (n--)
   { cand = Randint(POPULATION_SIZE);
     if (population[cand].fitness > maxfit)
     {   maxfit = population[cand].fitness;
         worst = cand;
     }
   }
   return (worst);
}


void copy(struct prog *src, struct prog *dest)
{
  memcpy(dest, src, sizeof(struct prog)); //for now
}


char *depth_ptr;

int get_depth()
{  char token = *depth_ptr++; int max = 0;
   int arity_m1 = token_table[token].arity_sub1;
   if (arity_m1 < 0)
      return (1);
   else
   {
      for (int i=0; i <= arity_m1; i++)
      { int d;
        if ((d=get_depth()) > max)
           max = d;
      }
      return (max+1);
   }
}


int depth(char *treep)
{  depth_ptr  = treep;
   return (get_depth());
}


void create_child(int dest, struct prog *p1, int start1, int end1,
                            struct prog *p2, int start2, int end2 )
{
   struct prog child;
   int seg1size = end1-start1+1;
   int seg2size = end2-start2+1;
   if ((child.proglen = p1->proglen - seg1size + seg2size) > MAX_PROG_SIZE)
   {  copy(p1, &population[dest]);
   }
   else
   {
      strncpy(child.code, p1->code, start1);
      strncpy(&child.code[start1], &(p2->code[start2]), seg2size);
      strcpy(&child.code[start1+seg2size], &(p1->code[end1+1]));

      copy(&child, &population[dest]);
      test_fitness(dest);
   }
}


int find_first(struct prog *progp)
{
   int loc = Randint(progp->proglen);
   char *codep = progp->code;
   if (random_number() < PROB_TERM) // must return a terminal
      while (codep[loc] > NUM_TERMINALS) loc++;
   else   // must return a fn.
      while (loc > 0 && codep[loc] <= NUM_TERMINALS) loc--;

   return (loc);
}



void crossover(int mum, int dad)
{
  int mumfirst, mumlast, dadfirst, dadlast;
  struct prog *mump = &population[mum];
  struct prog *dadp = &population[dad];
  int worst1, worst2;

  do worst1 = select_worst();
  while (worst1 == mum || worst1 == dad || worst1 == best_prog);

  do worst2 = select_worst();
  while (worst2 == mum || worst2 == dad || worst2 == worst1 ||
         worst2 == best_prog);

  mumfirst = find_first(mump);
  mumlast  = subtree_end(mump->code, mumfirst);
  dadfirst = find_first(dadp);
  dadlast  = subtree_end(dadp->code, dadfirst);

  create_child(worst1, mump, mumfirst, mumlast, dadp, dadfirst, dadlast);
  if (!correct)
     create_child(worst2, dadp, dadfirst, dadlast, mump, mumfirst, mumlast);

}


void display_stats(int gen)
{ int total_fitness = 0; int i, d; int deepest = 0;
  printf("GENERATION %d\n", gen);
  for (i =0; i < POPULATION_SIZE; i++)
  {  total_fitness += population[i].fitness;
  }

  printf("Best fitness = %d, Av. fitness = %f, Best prog = %d, Size = %d, Depth=%d\n",
         best_fitness, (float) total_fitness/POPULATION_SIZE, best_prog,
         population[best_prog].proglen, depth(population[best_prog].code));

  output_program(&population[best_prog]);

}

void init_globals(void)
{
  best_prog = 0;
  smallest_proglen = MAXINT;
  best_fitness = MAXINT;
  correct = FALSE;
}


int main()
{
  int i, run; time_t start;

  seed = time(NULL);
  time(&start);

  for (run=1; run <= NUM_RUNS; run++)
  {
    int generation = 0;
    init_globals();
    initialise_population();

    printf("\nSTARTING RUN %d...\n", run);

    while (generation < MAX_GEN  && !correct )
    // Create the next generation
    {  generation++;
    
       for (i = 0; i < POPULATION_SIZE && !correct; i++)
       {  double prob = random_number();
          int mum = select_best();


          if (prob < PCROSS)
          {  int dad; // = select_best();
             while ((dad = select_best()) == mum) /* null */ ;
             i++;
             crossover(mum,dad);
          }
          else  // Reproduce mum
          {  int worst ;
             while ((worst = select_worst()) == mum  || worst == best_prog)
                   ;    // null
             copy(&population[mum], &population[worst]);
          }
       } // end for

       //display_stats(generation);
       //getch();

    } // end while

    //if (correct)
    {  display_stats(generation);  //getch();

       puts("See path of best prog?");
       if (getch() == 'y')
       {   trace = TRUE;
           test_fitness(best_prog);
           trace = FALSE;
       }

    }
  } // end for

  puts("DONE");
  printf("TIME=%d\n", time(NULL) - start);
  getch();
  return (0);
}

