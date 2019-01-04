#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define RANDOM_SEED 1892

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMATIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define MAX_OPS 3

#define POPULATION_SIZE 10

#define TEST_DATA_PATH "data\\tests.dat"
#define NUM_TESTS 5
#define LINE_LENGTH 255

typedef struct{
    int size;
    int inversions;
    int arr[];
} Array;

#define arrayMem(x) (sizeof(Array) + sizeof(int) * (x))

Array* tests[MAX_OPS][NUM_TESTS];
Array* results;
int* arrBuffer;
int* mergeBuffer;
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
    int maxTestSize = 0;
    
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
        
        fscanf(file, "%d", &arrSize);
        
        tests[setNum][testNum] = malloc( arrayMem(arrSize) );
        
        if(arrSize > maxTestSize) maxTestSize = arrSize;
        
        Array* test = tests[setNum][testNum];
        test->size = arrSize;
        test->inversions = -1;
        
        for(int i = 0; i < arrSize; i++){
            
            int n;
            
            fscanf(file, "%d", &n);
            
            test->arr[i] = n;
        }        
        getc(file);
        ++testNum;
        
    }
    
    results = malloc( arrayMem(maxTestSize) );
    arrBuffer = malloc( arrayMem(maxTestSize) );
    mergeBuffer = malloc( arrayMem(maxTestSize) );
    
    fclose(file);
    return 0;
}

void printTestData(){
    
    for(int i = 0; i < MAX_OPS; i++){
        
        printf("Set %d\n\n", i);
        
        for(int j = 0; j < NUM_TESTS; j++){
            
            Array* test = tests[i][j];
            
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

int init(char* path){
    if(path == NULL) return 1;
    srand(RANDOM_SEED);
    initialisePopulation();
    return initialiseTestData(path);
    
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

int execute(Node* node){
    
    switch(node->primative){
        
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
            printf("\nINVALID PRIMATIVE (%d)\n", node->primative);
            return -1;
    }
    
}

#define indent(x) for(int i = 0; i<(x); i++)printf("    ")
int countInversions(int* arr, int size, int offset){
    static int x;
    indent(x);
    printf("Arr: ");
    
    
    
    
    
    for(int i = 0; i <size ; i++){
        
        printf("%d ",arr[i]);
        
    }
    
    printf("\n");
    
    if(size <= 1){
        
        return 0;
        
    }
    
    int sizeA = size / 2;
    int sizeB = size - (size / 2);
    
    int* A = mergeBuffer + offset;
    int* B = &mergeBuffer[sizeA] + offset;
    
    //memcpy(A, arr, sizeA*sizeof(int));
    //memcpy(B, &arr[sizeA], sizeB*sizeof(int));
    memcpy(A,arr,(sizeA + sizeB)*sizeof(int));
    x++;
    indent(x);
    printf("A: ");
    
    
    
    for(int i = 0; i <sizeA ; i++){
        
        printf("%d ",A[i]);
        
    }
    
    printf("\n");
    
    indent(x);
    printf("B: ");
    
    for(int i = 0; i <sizeB ; i++){
        
        printf("%d ",B[i]);
        
    }
    
    printf("\n");
    
    indent(x);
    
    printf("MB %d %d %d %d %d %d\n", 
        mergeBuffer[0],
        mergeBuffer[1],
        mergeBuffer[2],
        mergeBuffer[3],
        mergeBuffer[4],
        mergeBuffer[5]
        );
    indent(x);
    printf("ArB %d %d %d %d %d %d\n", 
        arrBuffer[0],
        arrBuffer[1],
        arrBuffer[2],
        arrBuffer[3],
        arrBuffer[4],
        arrBuffer[5]
        );
    
    int* temp = arrBuffer;
    arrBuffer = mergeBuffer;
    mergeBuffer = temp;
    
    
    x++;
    int inversions = countInversions(A,sizeA,offset) + countInversions(B,sizeB,sizeA+offset);
    x--;
    int aCounter = 0;
    int bCounter = 0;
    int cCounter = 0;
    
    
    
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
    x--;
    
    indent(x);
    printf("NA: ");
    
    
    
    for(int i = 0; i <sizeA ; i++){
        
        printf("%d ",A[i]);
        
    }
    
    printf("\n");
    
    indent(x);
    printf("NB: ");
    
    for(int i = 0; i <sizeB ; i++){
        
        printf("%d ",B[i]);
        
    }
    
    printf("\n");
    
    indent(x);
    printf("C: ");
    
    for(int i = 0; i <size ; i++){
        
        printf("%d ",arr[i]);
        
    }
    
    printf("\n");
    
    indent(x);
    printf("MB %d %d %d %d %d %d\n", 
        mergeBuffer[0],
        mergeBuffer[1],
        mergeBuffer[2],
        mergeBuffer[3],
        mergeBuffer[4],
        mergeBuffer[5]
        );
    indent(x);
    printf("ArB %d %d %d %d %d %d\n", 
        arrBuffer[0],
        arrBuffer[1],
        arrBuffer[2],
        arrBuffer[3],
        arrBuffer[4],
        arrBuffer[5]
        );
    
    return inversions;
    
}

int test(Node* node, int testSet, int testNum){
    
    Array* test = tests[testSet][testNum];
    
    if(test->inversions == -1){
        
        memcpy(arrBuffer, test->arr, test->size);
        test->inversions = countInversions(arrBuffer, test->size, 0);
        
    }
    
    memcpy(results, test, arrayMem(test->size));
    
    execute(node);
    
    
    
    return countInversions(results->arr, test->size, 0);
    
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

int main(int argc, char* argv[]){
    
    printf("Start\n\n");
    printf(argv[1]);
    printf("\n\n");
    
    printPrimativeTable();
    
    if(init(argv[1])){
        printf("File Not Found");
        return 1;
    }
    
    printTestData();
    
    evaluatePopulationSNGP_A();
    
    printPopulation();
    
    arrBuffer[0] = 6;
    arrBuffer[1] = 5;
    arrBuffer[2] = 4;
    arrBuffer[3] = 3;
    arrBuffer[4] = 2;
    arrBuffer[5] = 1;
    
    
    printf("%d\n",countInversions(arrBuffer,5,0));
    
    for(int i = 0; i <6 ; i++){
        
        printf("%d ",arrBuffer[i]);
        
    }
    
    return 0;
}