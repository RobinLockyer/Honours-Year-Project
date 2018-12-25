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

#define TEST_DATA_PATH "..\\data\\tests.dat"
#define NUM_TESTS 5
#define LINE_LENGTH 255

typedef struct{
    int size;
    int arr[];
} Test;

Test* tests[MAX_OPS][NUM_TESTS];

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
    
    int setNum = 0;
    int testNum = 0;
    
    //Iterate through test data file until we have a test set for each generation or EOF is reached 
    while(setNum < MAX_OPS){
        
        char firstC = getc(file);
        
        //If this line is blank, then we have reached the end of this test set
        if(firstC == '\n'){
            ++setNum;
            testNum = 0;
            continue;
        }
        
        ungetc(firstC,file);
        
        int arrSize = 0;
        
        fscanf(file, "%d", &arrSize);
        
        tests[setNum][testNum] = malloc( sizeof(Test) + arrSize *sizeof(int) );
        
        Test* test = tests[setNum][testNum];
        test->size = arrSize;
        
        for(int i = 0; i < arrSize; i++){
            
            int n;
            
            fscanf(file, "%d", &n);
            
            test->arr[i] = n;
        }        
        getc(file);
        ++testNum;
        
    }
    
    fclose(file);
    return 0;
}

void printTestData(){
    
    for(int i = 0; i < MAX_OPS; i++){
        
        printf("Set %d\n\n", i);
        
        for(int j = 0; j < NUM_TESTS; j++){
            
            Test* test = tests[i][j];
            
            printf("Test %d-%d Size %d\n", i, j, test->size);
            
            for(int k = 0; k < test->size; k++){
                
                printf("%d " , test->arr[k]);
                
            }
            
            printf("\n");
            
        }
        
        printf("\n");
        
    }
    
    printf("\n");
    
}

void init(){
    
    initialisePopulation();
    initialiseTestData(TEST_DATA_PATH);
    
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
    
    return NULL;
    
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
    
    printTestData();
    
    evaluatePopulationSNGP_A();
    
    printPopulation();
    
    return 0;
}