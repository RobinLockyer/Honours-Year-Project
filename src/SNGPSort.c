#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define RANDOM_SEED 1892

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMITIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define MAX_OPS 3

#define POPULATION_SIZE 10

#define NUM_TESTS 5
#define LINE_LENGTH 255

typedef struct{
    int size;
    int inversions;
    int arr[];
} Array;

#define arrayMem(x) (sizeof(Array) + sizeof(int) * (x))

Array* tests[MAX_OPS][NUM_TESTS];
int maxTestSize;

Array* results = NULL;
int* mergeBuffer1 = NULL;
int* mergeBuffer2 = NULL;
int index;

int generation = 0;

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
} Primitive;

typedef struct {
    Primitive primitive;
    int arity;
    char* name;
} TableEntry;

TableEntry primitiveTable[NUM_PRIMITIVES] = {
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
    Primitive primitive;
    int fitness;
    int oldFitness;
    int operands[MAX_ARITY];
    bool predecessors[POPULATION_SIZE];
} Node;

Node population[POPULATION_SIZE];

void printPrimitiveTable(){
    
    for(int i = 0; i < NUM_PRIMITIVES; i++){
        
        TableEntry entry = primitiveTable[i];
        printf("i: %d, e: %d, a: %d n: %s\n", i, entry.primitive, entry.arity, entry.name);
        
    }
    
    printf("\n");
    
}

//Returns random number in interval [min,max] inclusive
int randRange(int min, int max){
    
    return min + rand()/(RAND_MAX/(max-min+1)+1);
    
}

void initialisePopulation(){
    
    for(int tIndex = 0; tIndex < NUM_TERMINALS; tIndex++){
        
        population[tIndex].primitive = tIndex;
        for(int oIndex = 0; oIndex < MAX_ARITY; oIndex++){
            
             population[tIndex].operands[oIndex] = -1;
            
        }
    }
    
    for(int fIndex = NUM_TERMINALS; fIndex < POPULATION_SIZE; fIndex++){
        
        Node* node = &population[fIndex];
        
        Primitive primitive = randRange(NUM_TERMINALS,NUM_PRIMITIVES-1);
        
        node->primitive = primitive;
        node->fitness = -1;
        node->oldFitness = -1;
        
        for(int oIndex = 0; oIndex < MAX_ARITY; oIndex++){
            
            if(oIndex<primitiveTable[primitive].arity){
                
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
    maxTestSize = 0;
    
    //Iterate through test data file until we have a test set for each generation or EOF has been reached
    while(setNum < MAX_OPS){
        
        char firstC = getc(file);
        
        //If this line is blank, then we have reached the end of this test set
        if(firstC == '\n'){
            ++setNum;
            testNum = 0;
            continue;
        }
        //If end of file has been reached, exit loop
        else if(firstC == EOF){
            
            break;
            
        }
        
        ungetc(firstC,file);
        
        int arrSize = 0;
		int inversions = 0;
        
        fscanf(file, "%d %d", &arrSize, &inversions);
        
        tests[setNum][testNum] = malloc( arrayMem(arrSize) );
        
        if(arrSize > maxTestSize) maxTestSize = arrSize;
        
        Array* test = tests[setNum][testNum];
        test->size = arrSize;
        test->inversions = inversions;
        
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
            
            Array* test = tests[i][j];
            
            printf("Test: %d-%d Size: %d Inversions: %d\n", i, j, test->size, test->inversions);
            
            for(int k = 0; k < test->size; k++){
                
                printf("%d " , test->arr[k]);
                
            }
            
            printf("\n");
            
        }
        
        printf("\n");
        
    }
    
    printf("\n");
    
}

int init(char* path){
    if(path == NULL) return 1;
    if(initialiseTestData(path)) return 1;
    srand(RANDOM_SEED);
    initialisePopulation();
    results = malloc( arrayMem(maxTestSize) );
    mergeBuffer1 = malloc( sizeof(int) * maxTestSize );
    mergeBuffer2 = malloc( sizeof(int) * maxTestSize );
    return 0;
    
}

void printPopulation(){
    
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
        Node node = population[popIndex];
        printf(
            "Index: %d primitive: %s Arity: %d\nFitness: %d OldFitness: %d\nOperands:", 
            popIndex,
            primitiveTable[node.primitive].name,
            primitiveTable[node.primitive].arity,
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

int execute(Node* node){
    
    switch(node->primitive){
        
        case INDEX:{
            
            return index;
            
        }break;
            
        case LENGTH:{
            
            return results->size;
        
        }break;
            
        case ITERATE:{
                
            int len = results->size;
            int start = execute(&population[node->operands[0]]);
            int end = execute(&population[node->operands[1]]);
            Node* function = &population[node->operands[2]];
            
            for(index = start; index < end || index < len; index++){
                execute(function);
            }
            
            return (end < len) ? end : len ;
                
        }break;
            
        case SWAP:{
            
            int x = node->operands[0];
            int y = node->operands[1];
            int t = results->arr[x];
            results->arr[x] = results->arr[y];
            results->arr[y] = t;
            
            return x;
            
        }break;
            
        case SMALLEST:{
            
            int x = node->operands[0];
            int y = node->operands[1];
            
            if(results->arr[x] < results->arr[y]){
                return x;
            }
            else{
                
                return y;
                
            }
            
        }break;
            
        case LARGEST:{
            
            int x = node->operands[0];
            int y = node->operands[1];
            
            if(results->arr[x] > results->arr[y]){
                return x;
            }
            else{
                
                return y;
                
            }
            
        }break;
            
        case SUB:{
            
            int x = node->operands[0];
            int y = node->operands[1];
            
            return x-y;
            
        }break;
            
        case INC:{
            
            int x = node->operands[0];
            
            return x+1;
            
        }break;
            
        case DEC:{
            
            int x = node->operands[0];
            
            return x-1;
            
        }break;
            
        default:
            printf("\nINVALID PRIMITIVE (%d)\n", node->primitive);
            return -1;
    }
    
}

int countInversionsRec(int* arr, int* working, int size, int offset){
    
    if(size <= 1){
        
        return 0;
        
    }
    
    int sizeA = size / 2;
    int sizeB = size - sizeA;
    
    int* A = working + offset;
    int* B = &working[sizeA] + offset;
    
    int inversions = countInversionsRec(working,arr,sizeA,offset) + countInversionsRec(working,arr,sizeB,sizeA+offset);

    int aCounter = 0;
    int bCounter = 0;
    int cCounter = offset;
    
    
    while( aCounter < sizeA && bCounter < sizeB ){
        
        if( A[aCounter] < B[aCounter] ){
            
            arr[cCounter] = A[aCounter];
            aCounter++;
            
        }else{
            
            arr[cCounter] = B[bCounter];
            inversions += (sizeA - aCounter);
            bCounter++;
            
        }
        
        cCounter++;
        
    }
    
    if(aCounter < sizeA){
        
        memcpy( &arr[cCounter], &A[aCounter], (sizeA - aCounter) * sizeof(int));
        
    } else if(bCounter < sizeB){
        
        memcpy( &arr[cCounter], &B[bCounter], (sizeB - bCounter) * sizeof(int));
        
    }    
    
    return inversions;
    
}

int countInversions(Array* arr){
    
    if(mergeBuffer1 == NULL || mergeBuffer2 == NULL) return -1;
    
    memcpy(mergeBuffer1,arr->arr,arr->size*sizeof(int));
    memcpy(mergeBuffer2,arr->arr,arr->size*sizeof(int));
    
    arr->inversions = countInversionsRec(mergeBuffer1, mergeBuffer2, arr->size, 0);
    
    return arr->inversions;
    
}

int test(Node* node, int testSet, int testNum){
    
    Array* test = tests[testSet][testNum];
    
    if(test->inversions == -1){
        
        memcpy(results, test->arr, test->size);
        //test->inversions = countInversions(arrBuffer, test->size, 0);
        
    }
    
    memcpy(results, test, arrayMem(test->size));
    
    execute(node);
    
    
    
    return 0; //countInversions(results->arr, test->size, 0);
    
}

int evaluatePopulationSNGP_A(){
    
    int totalFitness = 0;
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
        
        Node* node = &population[popIndex];
        
        
        for(int testNum = 0; testNum < NUM_TESTS; testNum++){
            
            totalFitness += test(node, generation, testNum);
            
        }
        
    }
    
    return totalFitness/POPULATION_SIZE;
    
}

void initialiseExamplePopulation(){
	
	population[0].primitive = INDEX;
	
	population[1].primitive = LENGTH;
	
	population[2].primitive = INC;
	population[2].operands[0] = 0;
	
	population[3].primitive = SMALLEST;
	population[3].operands[0] = 2;
	population[3].operands[1] = 0;
	
	population[4].primitive = SWAP;
	population[4].operands[0] = 3;
	population[4].operands[1] = 0;
	
	population[5].primitive = SUB;
	population[4].operands[0] = 1;
	population[4].operands[1] = 1;
	
	population[6].primitive = DEC;
	population[6].operands[0] = 1;
	
	population[7].primitive = ITERATE;
	population[7].operands[0] = 5;
	population[7].operands[1] = 6;
	population[7].operands[2] = 4;
	
	population[8].primitive = ITERATE;
	population[8].operands[0] = 5;
	population[8].operands[1] = 6;
	population[8].operands[2] = 7;
	
}

int main(int argc, char* argv[]){
    
    printf("Start\n\n");
    printf(argv[1]);
    printf("\n\n");
    
    printPrimitiveTable();
    
	/*
    if(init(argv[1])){
        printf("File Not Found");
        return 1;
    }
    
    printTestData();
    
    evaluatePopulationSNGP_A();
    
    printPopulation();
    */
	
	initialiseExamplePopulation();
	printPopulation();
	
	initialiseTestData(argv[1]);
	test(&population[8],2,1);
	
	
    
    
    return 0;
}