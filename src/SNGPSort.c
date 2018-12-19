#include <stdio.h>
#include <stdlib.h>

#define RANDOM_SEED 4

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMATIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define POPULATION_SIZE 50

typedef enum primatives {
    INDEX,
    LENGTH,
    ITERATE, 
    SWAP,
    SMALLEST,
    LARGEST,
    SUB,
    INC,
    DEC
} Primative;

typedef struct {
    Primative primative;
    int arity;
} TableEntry;

TableEntry arityTable[NUM_PRIMATIVES] = {
    {INDEX,    0},
    {LENGTH,   0},
    {ITERATE,  3},
    {SWAP,     2},
    {SMALLEST, 2},
    {LARGEST,  2},
    {SUB,      2},
    {INC,      1},
    {DEC,      1}
};

typedef struct {
    Primative primative;
    int fitness;
    int oldFitness;
    int operands[MAX_ARITY];
    int predecessors[POPULATION_SIZE];
} Node;

Node population[POPULATION_SIZE];

void printArityTable(){
    
    for(int i = 0; i < NUM_PRIMATIVES; i++){
        
        TableEntry entry = arityTable[i];
        printf("i: %d, p: %d, a: %d\n", i, entry.primative, entry.arity);
        
    }
    
    printf("\n");
    
}

//Returns random number in interval [min,max] inclusive
int randRange(int min, int max){
    
    return min + rand()/(RAND_MAX/(max-min+1)+1);
    
}

void initialisePopulation(){
    
    srand(RANDOM_SEED);
    
    for(int i = 0; i < NUM_TERMINALS; i++){
        
        population[i].primative = i;
        
    }
    
    for(int i = NUM_TERMINALS; i < POPULATION_SIZE; i++){
        
        Node* node = &population[i];
        
        Primative primative = randRange(NUM_TERMINALS,POPULATION_SIZE-1);
        
        node->primative = primative;
        node->fitness = -1;
        node->oldFitness = -1;
        
        for(int j = 0; j < arityTable[primative].arity; j++){
            
            node->operands[i] = randRange(0,i-1);
            
        }
        
        for(int j = 0; j < arityTable[primative].arity; j++){
            
            node->operands[i] = randRange(0,i-1);
            
        }
        
    }
    
}

int main(){
    
    printf("Start\n\n");
    
    printArityTable();
    
    
    
    return 0;
}