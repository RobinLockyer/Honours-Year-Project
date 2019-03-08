#include <stdio.h>
#include <stdlib.h>

#define RANDOM_SEED 1894

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMITIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define POPULATION_SIZE 10//1000 in final
#define MAX_PROG_SIZE 2000
#define INITIAL_MAX_DEPTH 6
#define NUM_GENERATIONS 10 //50 in final
#define NUM_TESTS 15
#define MAX_RUNS 1 //20 in final
#define NUM_TEST_SETS 3000

short success = 0;

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
    char code[MAX_PROG_SIZE+1];//Includes null terminator
    double fitness;
    int progLen;
} Prog;

char* progNode = NULL;

Prog population[POPULATION_SIZE];

typedef struct{
    int size;
    int inversions;
    int arr[];
} Array;

#define arrayMem(x) (sizeof(Array) + sizeof(int) * (x))

Array* tests[NUM_GENERATIONS][NUM_TESTS];
int maxTestSize;

Array* results = NULL;
int* mergeBuffer1 = NULL;
int* mergeBuffer2 = NULL;

int index = 0;
int progIterations = 0;
#define MAX_PROG_ITERATIONS 10000

//Returns random number in interval [min,max] inclusive
int randRange(int min, int max){
    
    return min + rand()/(RAND_MAX/(max-min+1)+1);
    
}

int initialiseTestData(char* path){
    
    FILE* file = fopen(path,"r");
    
    if(!file) return 1;
    int setNum = 0;
    int testNum = 0;
    maxTestSize = 0;
    
    //Iterate through test data file until we have a test set for each generation or EOF has been reached
    while(setNum < NUM_GENERATIONS){
        
        char firstC = getc(file);
        
        
        
        //If this line is blank, then we have reached the end of this test set
        if(firstC == '\n'){
            ++setNum;
            testNum = 0;
            continue;
        
        //if we have exeeded the number of tests per set, skip the remainder of the set
        }else if(testNum >= NUM_TESTS){
            ++setNum;
            testNum = 0;
            
            do{
                fscanf(file,"%*[^\n]",NULL);
                getc(file);
                firstC = getc(file);
            }while(firstC != '\n');
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

int execute(){
    
    switch(*progNode++){
        
        case INDEX:{
            
            return index;
            
        }break;
            
        case LENGTH:{
            
            return results->size;
        
        }break;
            
        case ITERATE:{
                
            int len = results->size;
            int start = execute();
            int end = execute();
            char* function = progNode;
            
            int oldIndex = index;
            
            for(index = start; index <= end && index < len && progIterations < MAX_PROG_ITERATIONS; ++index, ++progIterations){
                
                progNode = function;
                
                execute();
            }
            
            index = oldIndex;
            
            return (end < len) ? end : len ;
                
        }break;
            
        case SWAP:{
            
            int x = execute();
            int y = execute();
            
            //If x or y is not a valid index, return 0
            if(x<0 || y<0 || x>=results->size || y>=results->size) return 0;
            
            int t = results->arr[x];
            results->arr[x] = results->arr[y];
            results->arr[y] = t;
            
            return x;
            
        }break;
            
        case SMALLEST:{
            
            int x = execute();
            int y = execute();
            
            if(results->arr[x] < results->arr[y]){
                return x;
            }
            else{
                
                return y;
                
            }
            
        }break;
            
        case LARGEST:{
            
            int x = execute();
            int y = execute();
            
            if(results->arr[x] > results->arr[y]){
                return x;
            }
            else{
                
                return y;
                
            }
            
        }break;
            
        case SUB:{
            
            int x = execute();
            int y = execute();
            
            return x-y;
            
        }break;
            
        case INC:{
            
            int x = execute();
            
            return x+1;
            
        }break;
            
        case DEC:{
            
            int x = execute();
            
            return x-1;
            
        }break;
            
        default:
            printf("\nINVALID PRIMITIVE (%d)\n", *(progNode-1));
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
        
        if( A[aCounter] <= B[bCounter] ){
            
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

float testNode(int popIndex, int testSet, int testNum){
    
    Array* test = tests[testSet][testNum];
    
    //If inversions not counted, count inversions
    if(test->inversions == -1){
        
        memcpy(results, test->arr, test->size);
        countInversions(test);
        
    }
    
    index = 0;
    
    memcpy(results, test, arrayMem(test->size));
    
    progIterations = 0;
    execute(popIndex);
    
    //if(progIterations > MAX_PROG_ITERATIONS) printf("\nMax iterations exceeded\n");
    
    int inversions = countInversions(results);
    
    float fitness;// = test->inversions - inversions;
    
    if(inversions == test->inversions && inversions!=0) fitness = 0;
    else if(test->inversions!=0) fitness = 1 - inversions/(float)test->inversions;
    else if(inversions == 0) fitness = 1;
    else fitness = -inversions;
    
    return fitness; 
    
}

char* createTree(char* tree, int depth, int full){  
     
    if (depth == 1) *tree++ = randRange(0,NUM_TERMINALS-1);
    
    else{
        
        char primitive = full ? randRange(NUM_TERMINALS, NUM_PRIMITIVES-1): randRange(0,NUM_PRIMITIVES-1);
        
        *tree = primitive;
        
        tree++;
        
        int arity = primitiveTable[primitive].arity;
        
        for(int argument = 0; argument < arity; ++argument){
            
            tree = createTree(tree, depth-1, full);
            
        }
   }
   return(tree);
}

void initialisePopulation(){
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; ++popIndex){
        
        Prog* prog = &population[popIndex];
        
        char* code = prog->code;
        
        createTree(code, 6, 0);
        
        prog->progLen = strlen(code);
        
    }
    
    
}

int init(char* path){
    if(path == NULL) return 1;
    if(initialiseTestData(path)) return 1;
    srand(RANDOM_SEED);
    results = malloc( arrayMem(maxTestSize) );
    mergeBuffer1 = malloc( sizeof(int) * maxTestSize );
    mergeBuffer2 = malloc( sizeof(int) * maxTestSize );
    return 0;
    
}

#include "GP_Sort_Debug.c"
int main(int argc, char* argv[]){
    
    printf("Start\n\n");
    printf(argv[1]);
    printf("\n\n");
    
    printPrimitiveTable();
    
    if(init(argv[1])){
        printf("File Not Found");
        return 1;
    }else{
        
        printf("\nTest file loaded\n\n");
        
    }
    
    initialisePopulation();
    
    printPopulation();
    
    return 0;
    
}