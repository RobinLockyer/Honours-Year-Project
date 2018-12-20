#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define RANDOM_SEED 1892

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMATIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define POPULATION_SIZE 10

//TODO: Seperate generic SNGP code

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
    char* name;
} TableEntry;

TableEntry primativeTable[NUM_PRIMATIVES] = {
    {INDEX,    0, "INDEX"},
    {LENGTH,   0, "LENGTH"},
    {ITERATE,  3, "ITERATE"},
    {SWAP,     2, "SWAP"},
    {SMALLEST, 2, "SMALLEST"},
    {LARGEST,  2, "LARGEST"},
    {SUB,      2, "SUB"},
    {INC,      1, "INC"},
    {DEC,      1, "DEC"}
};

typedef struct {
    Primative primative;
    int fitness;
    int oldFitness;
    int operands[MAX_ARITY];
    bool predecessors[POPULATION_SIZE];
} Node;

Node population[POPULATION_SIZE];

void printPrimativeTable(){
    
    for(int i = 0; i < NUM_PRIMATIVES; i++){
        
        TableEntry entry = primativeTable[i];
        printf("i: %d, e: %d, a: %d n: %s\n", i, entry.primative, entry.arity, entry.name);
        
    }
    
    printf("\n");
    
}

//Returns random number in interval [min,max] inclusive
int randRange(int min, int max){
    
    return min + rand()/(RAND_MAX/(max-min+1)+1);
    
}

void initialisePopulation(){
    
    srand(RANDOM_SEED);
    
    
    //TODO: Merge this loop into next loop
    for(int i = 0; i < NUM_TERMINALS; i++){
        
        population[i].primative = i;
        for(int j = 0; j < MAX_ARITY; j++){
            
             population[i].operands[j] = -1;
            
        }
    }
    
    for(int i = NUM_TERMINALS; i < POPULATION_SIZE; i++){
        
        Node* node = &population[i];
        
        Primative primative = randRange(NUM_TERMINALS,NUM_PRIMATIVES-1);
        
        node->primative = primative;
        node->fitness = -1;
        node->oldFitness = -1;
        
        for(int j = 0; j < MAX_ARITY; j++){
            
            if(j<primativeTable[primative].arity){
                
                int operandIndex = randRange(0,i-1);
                
                node->operands[j] = operandIndex;
                
                population[operandIndex].predecessors[i] = true;
                
            } else{
                
                node->operands[j] = -1;
                
            }
            
        }
        
    }
    
}

void printPopulation(){
    
    
    for(int i = 0; i < POPULATION_SIZE; i++){
        Node node = population[i];
        printf(
            "Index: %d primative: %s Arity: %d\nFitness: %d OldFitness: %d\nOperands:", 
            i,
            primativeTable[node.primative].name,
            primativeTable[node.primative].arity,
            node.fitness,
            node.oldFitness
        );
        
        for(int j = 0; j < MAX_ARITY; j++){
            printf("%d ",node.operands[j]);
        }
        
        printf("\nPredecessors: ");
        
        for(int j = 0; j < POPULATION_SIZE; j++){
            
            if(node.predecessors[j]) printf("%d ",j);
            
        }
        
        printf("\n\n");
    }
    
    printf("\n");
    
}

int main(){
    
    printf("Start\n\n");
    
    printPrimativeTable();
    
    initialisePopulation();
    
    printPopulation();
    
    return 0;
}