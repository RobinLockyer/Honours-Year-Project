#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define RANDOM_SEED 1892

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMATIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define MAX_OPS 2

#define POPULATION_SIZE 10

//Every set has 5 tests
#define NUM_TESTS 5



typedef enum {
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
    int* results[NUM_TESTS];
    int* oldResults[NUM_TESTS];
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
    for(int tIndex = 0; tIndex < NUM_TERMINALS; tIndex++){
        
        population[tIndex].primative = tIndex;
        for(int oIndex = 0; oIndex < MAX_ARITY; oIndex++){
            
             population[tIndex].operands[oIndex] = -1;
            
        }
    }
    
    for(int fIndex = NUM_TERMINALS; fIndex < POPULATION_SIZE; fIndex++){
        
        Node* node = &population[fIndex];
        
        Primative primative = randRange(NUM_TERMINALS,NUM_PRIMATIVES-1);
        
        node->primative = primative;
        node->fitness = -1;
        node->oldFitness = -1;
        
        for(int oIndex = 0; oIndex < MAX_ARITY; oIndex++){
            
            if(oIndex<primativeTable[primative].arity){
                
                int operandIndex = randRange(0,fIndex-1);
                
                node->operands[oIndex] = operandIndex;
                
                population[operandIndex].predecessors[fIndex] = true;
                
            } else{
                
                node->operands[oIndex] = -1;
                
            }
            
        }
        
    }
    
}

int initialiseTestData(char* path){
    
    FILE* file = fopen(path,"r");
    if(!file) return 1;
    
	char stringBuffer[3*255]; //Max 255 8 bit numbers, represented by max 3 characters
	int intBuffer[255];
	
	fgets(stringBuffer, 255, file);
	
	char* nextNum = stringBuffer;
	
	for(int testNum = 0; testNum < NUM_TESTS && *nextNum != EOF; testNum++){
		
		int intCounter = 0;
		strtoI(nextNum, " ", &nextNum);
		
	}
	
	
    
    fclose(file);
    return 0;
}

void init(){
    
    initialisePopulation();
    initialiseTestData("..\\data\\tests.dat");
    
}

void printPopulation(){
    
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
        Node node = population[popIndex];
        printf(
            "Index: %d primative: %s Arity: %d\nFitness: %d OldFitness: %d\nOperands:", 
            popIndex,
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

int* test(Node* node, int testNum){
    
    node->oldResults[testNum] = node->results[testNum];
    
    return node->results[testNum];
    
}

int evaluateFitness(Node* node){
    
    return 0;
    
}

int evaluatePopulationSNGP_A(){
    
    int totalFitness = 0;
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
        
        Node* node = &population[popIndex];
        
        for(int testNum = 0; testNum < NUM_TESTS; testNum++){
            
            test(node,testNum);
            
        }
        
        totalFitness += evaluateFitness(node);
        
    }
    
    return totalFitness/POPULATION_SIZE;
    
}

int main(){
    
    printf("Start\n\n");
    
    printPrimativeTable();
    
    init();
    
    evaluatePopulationSNGP_A();
    
    printPopulation();
    
    return 0;
}