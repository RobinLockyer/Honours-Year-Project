
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
// #include <assert.h>
#include "myrand.h"

#define POPULATION_SIZE  500
#define PARTITION_SIZE   (POPULATION_SIZE/(MAX_INIT_DEPTH - 1))
// Need to ensure that pop. size is a precise multiple of (MAX_INIT_DEPTH-1)

#define PCROSS           0.818
#define PROB_TERM        0.1
#define PMUT             0.00  // Don't mutate (not yet implemented)
#define MAX_GEN          50
#define TOURN_SIZE       5
#define MAX_PROG_SIZE    2000
#define NUM_RUNS         100
#define TRUE             1
#define FALSE            0

#define MAX_INIT_DEPTH   6  // Max init size = 63 at this depth

#define NUM_TERMINALS    4
#define NUM_FUNCTIONS    4

enum tokens {DUMMY, D0, D1, D2, D3, AND, OR, NAND, NOR};

struct token_entry
{
   char *name;
   int arity_sub1;
   // function
};

struct token_entry token_table [] =
     { {"# ", -1},     // String end. Arity must be -1 for sub-string search
       {"D0 ", -1},
       {"D1 ", -1},
       {"D2 ", -1},
       {"D3 ", -1},
       {"AND ", 1},
       {"OR ", 1},
       {"NAND ", 1},
       {"NOR ", 1}
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
int results[] = {1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1};


int perform[MAX_GEN + 1]; // Used for performance graph; made global for
                          // initialisation to zero

int correct; int trace=FALSE;


char *nodep;
int d0, d1, d2, d3;


void output_program(struct prog *progp)
{
  char *codep = progp->code;
  char token;

  while (token = *codep++)
     fputs(token_table[token].name, stdout);
  putchar('\n');
}

void skip_arg(void)
{  int count = 0;
   while ((count += token_table[*nodep++].arity_sub1) >= 0)   ;
}


int evaltree()
// Could use jump table
{  int left, right;
   switch (*nodep++)
   {  case D0:      return (d0);
      case D1:      return (d1);
      case D2:      return (d2);
      case D3:      return (d3);
      case AND:     if (!evaltree())
                    {  skip_arg(); return (0);
                    }
                    else return (evaltree());
      case OR:      if (evaltree())
                    { skip_arg(); return (1);
                    }
                    else return (evaltree());
      case NAND:    if (!evaltree())
                    {  skip_arg(); return (1);
                    }
                    else return (!evaltree());
      case NOR:     if (evaltree())
                    { skip_arg(); return (0);
                    }
                    else return (!evaltree());
      /*
      case AND:     left = evaltree(); right = evaltree();
                    return (left && right);
      case OR:      left = evaltree(); right = evaltree();
                    return (left || right);
      case NAND:    left = evaltree(); right = evaltree();
                    return (!(left && right));
      case NOR:     left = evaltree(); right = evaltree();
                    return (!(left || right));
      */
   }
}


void test_fitness(int index)
{
  // Evaluate fitness of the indexed individual

  struct prog *progp = &population[index];
  int count = progp->proglen;
  int mismatches = 0;
  int i;

  for (i=0; i < 16; i++)
  {  nodep = progp->code;
     d0 = i & 1;
     d1 = (i >> 1) & 1;
     d2 = (i >> 2) & 1;
     d3 = i >> 3;
     mismatches += (evaltree() != results[i]);
  }

  correct = (mismatches == 0);
  progp->fitness = mismatches;

  if (mismatches < best_fitness ||
     (mismatches == best_fitness && count < smallest_proglen))
  {  best_prog = index;
     best_fitness = mismatches;
     smallest_proglen = count;
  }
}

#define random_terminal() ((char)(Randint(NUM_TERMINALS) + 1))
//char random_terminal()
//{  return (Randint(NUM_TERMINALS) + 1);
//}

#define random_function() ((char)(Randint(NUM_FUNCTIONS) + NUM_TERMINALS + 1))
//char random_function()
//{  return (Randint(NUM_FUNCTIONS) + NUM_TERMINALS + 1);
//}

#define random_token()  ((char)(Randint(NUM_FUNCTIONS+NUM_TERMINALS) + 1))
//char random_token()
//{  return(Randint(NUM_FUNCTIONS+NUM_TERMINALS) + 1);
//}

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
  int i, j, depth, full;
  char *end_tree;

  for (i = 2; i <= MAX_INIT_DEPTH; i++)
  {
     for (j = 0; j < PARTITION_SIZE; j++)
     {  depth = i; full = j%2;
        do
        {  end_tree = create_tree(progp->code, depth, full);
           *end_tree = '\0';
           progp->proglen = strlen(progp->code);
           if (depth < MAX_INIT_DEPTH) depth++;
        } while (duplicate(progp));
        progp++;
     }
  }

  for (i=0; i < POPULATION_SIZE && !correct; i++)
     test_fitness(i);
}


int subtree_end(char tree[], int index)
{  int limbs = 0;
   while ((limbs += token_table[tree[index]].arity_sub1) >= 0) index++;
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


void mutate(int mum)
{

  int worst;
  struct prog *dest;
  //assert (1==0);
  while ((worst = select_worst()) == mum || worst == best_prog) /* null */ ;

  dest = &population[worst];
  copy(&population[mum], dest);

// put stuff here

  test_fitness(worst);
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

      /*
      printf("CHILD...\n");
      output_program(&child);
      getch();    */
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

  /*
  printf("\nMUM...\n");
  output_program(mump);
  printf("\nDAD...\n");
  output_program(dadp);
  */

  do worst1 = select_worst();
  while (worst1 == mum || worst1 == dad || worst1 == best_prog);

  do worst2 = select_worst();
  while (worst2 == mum || worst2 == dad || worst2 == worst1 ||
         worst2 == best_prog);

  mumfirst = find_first(mump);
  //mumfirst = Randint(mump->proglen);
  mumlast  = subtree_end(mump->code, mumfirst);
  dadfirst = find_first(dadp);
  //dadfirst = Randint(dadp->proglen);
  dadlast  = subtree_end(dadp->code, dadfirst);

  create_child(worst1, mump, mumfirst, mumlast, dadp, dadfirst, dadlast);
  if (!correct)
     create_child(worst2, dadp, dadfirst, dadlast, mump, mumfirst, mumlast);

}


void display_stats(int gen)
{ int total_fitness = 0; int i;
  printf("GENERATION %d\n", gen);
  for (i =0; i < POPULATION_SIZE; i++)
          total_fitness += population[i].fitness;
  printf("Best fitness = %d, Av. fitness = %f, Best prog = %d\n",
         best_fitness, (float) total_fitness/POPULATION_SIZE, best_prog);
  output_program(&population[best_prog]);
  /*trace=TRUE;
  test_fitness(best_prog);
  trace=FALSE;    */

}

void init_globals(void)
{
  best_prog = 0;
  smallest_proglen = MAXINT;
  best_fitness = MAXINT;
  correct = FALSE;
}


void output_results(void)
{
  FILE *fp;

  if ((fp = fopen("even4out.csv", "w")) == NULL)
  {  fprintf(stderr, "Cannot open output file even3out.csv\n");
     exit(1);
  }

  puts("Performance array...");
  for (int i=0; i <= MAX_GEN; i++)
  {
    fprintf(fp, ",%d", i);
    if (i>0)
       perform[i] += perform[i-1];
  }

  puts("\nCum. probs...");
  fprintf(fp, "\n\"P(M,i)\"");
  for (int i=0; i <= MAX_GEN; i++)
  {
     fprintf(fp, ",%d", perform[i] = (perform[i]*100)/NUM_RUNS);
  }


  puts("\nIndividuals processed...");
  fprintf(fp, "\n\"I(M,i,z)\"");
  for (int i=0; i <= MAX_GEN; i++)
  {
     if (perform[i] == 0)
        fprintf(fp, ", ");
     else
     {
        //printf("%f %d", -2.0/log10(1.0-perform[i]/100.0), ceil(-2.0/log10(1.0-perform[i]/100.0)));
        int runs = (perform[i] == 100 ? 1 :
                       (int)ceil(-2.0/log10(1.0-perform[i]/100.0)) );
        fprintf(fp, ",%d", runs*(i+1)*POPULATION_SIZE);
     }
  }
  fclose(fp);
}



int main()
{
  int i, run; time_t start;

  //seed = time(NULL);
  seed = 1733469753;
  time(&start);

  for (run=1; run <= NUM_RUNS; run++)
  {
    int generation = 0;
    printf("\nSTARTING RUN %d...\nGen: ", run);

    init_globals();
    initialise_population();

    /*
    for (i=0; i < POPULATION_SIZE; i++)
    {  output_program(&population[i]); //test_fitness(i);
       getch();
    }    
    */

    
    while (generation < MAX_GEN  && !correct )
    // Create the next generation
    {  generation++;
       //printf("%d ", generation);

       for (i = 0; i < POPULATION_SIZE && !correct; i++)
       {  double prob = random_number();
          int mum = select_best();

          if (prob < PMUT)
          {  mutate(mum);
          }
          else if (prob < PCROSS)
          {  int dad;
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

    if (correct)
    {  display_stats(generation); // getch();
       perform[generation]++;
    }
  } // end for

  //output_results();
  printf("TIME=%d\n", time(NULL)-start);
  puts("DONE");
  getch();
  return (0);
}

