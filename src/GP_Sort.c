#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RANDOM_SEED time(NULL)//1894

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMITIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define POPULATION_SIZE 1000 //1000 in final
#define MAX_PROG_SIZE 3000
#define INITIAL_MAX_DEPTH 6
#define NUM_GENERATIONS 50 //50 in final
#define NUM_TESTS 15
#define MAX_RUNS 20 //20 in final
#define NUM_TEST_SETS 3000

#define SF 5
#define OF 5

#define MAX_MUTATE_DEPTH_INCREASE 1.15

#define OUT_FILE "results\\gp_success_trace.csv"
FILE* successTrace = NULL;

int numWorkingProgs = 0;
int workingProgramme = -1;

typedef enum {
    DUMMY,
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

TableEntry primitiveTable[NUM_PRIMITIVES+1] = {
    {DUMMY,   -1, "DUMMY"},
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

Prog popBuffer1[POPULATION_SIZE];
Prog popBuffer2[POPULATION_SIZE];

Prog* population = &popBuffer1[0];
Prog* newPopulation = &popBuffer2[0];

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

//Returns random integer in interval [min,max] inclusive
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

//progNode must be set to Prog.code before this is called
//progIterations must be set to 0 before this is called
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

int res(int popIndex, int testSet, int testNum){
    
    Array* test = tests[testSet][testNum];
    
    //If inversions not counted, count inversions
    if(test->inversions == -1){
        
        memcpy(results, test->arr, test->size);
        countInversions(test);
        
    }
    
    index = 0;
    
    memcpy(results, test, arrayMem(test->size));
    
    progIterations = 0;
    progNode = population[popIndex].code;
    execute(popIndex);
    
    int iDis = test->inversions;
    int rDis = countInversions(results);
    int pDis = (rDis > iDis) ? (rDis - iDis)*100 : 0;
    
    return rDis + pDis; 
    
}

int praw(int testSet, int popIndex){
    
    int resSum = 0;
    
    for(int testNum = 0; testNum < NUM_TESTS; testNum++){
        resSum += res(popIndex,testSet,testNum);
    }
    
    if(resSum == 0){
        numWorkingProgs += 1;
        workingProgramme = popIndex;
    }
    
    return (resSum * OF) + (population[popIndex].progLen * SF);
    
}

void evaluatePopulation(int testSet){
    
    numWorkingProgs = 0;
    
    int prawTable[POPULATION_SIZE];
    int raw[POPULATION_SIZE];
    float adj[POPULATION_SIZE];
    float adjSum = 0;
    
    prawTable[0] = praw(testSet, 0);
    
    int minpraw = prawTable[0];
    
    
    for(int popIndex = 1; popIndex < POPULATION_SIZE; ++popIndex){
        
        prawTable[popIndex] = praw(testSet, popIndex);
        
        if(prawTable[popIndex] < minpraw) minpraw = prawTable[popIndex];
        
    }
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; ++popIndex){
        
        int raw = prawTable[popIndex] - minpraw;
        
        adj[popIndex] = 1.0/(1.0+raw);
        
        adjSum += adj[popIndex];
        
    }
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; ++popIndex){
        
        population[popIndex].fitness = adj[popIndex]/adjSum;
        
    }
    
    fprintf(successTrace,"%d,",numWorkingProgs);
    
}

char* createTree(char* tree, int depth, int maxNodes, int full){  
    char* treeEnd = tree;
    
    if (depth == 1 || maxNodes == 1){
        *treeEnd = randRange(1,NUM_TERMINALS);
        treeEnd++;
    }
    
    else{
        
        char primitive;
        int arity;
        do{
            
            primitive = full ? randRange(NUM_TERMINALS+1, NUM_PRIMITIVES): randRange(1,NUM_PRIMITIVES);
            arity = primitiveTable[primitive].arity;
            
        }while(arity > maxNodes-1);
        
        
        *tree = primitive;
        
        treeEnd++;
        
        
        for(int argument = 0; argument < arity; ++argument){
            
            treeEnd = createTree(treeEnd, depth-1, maxNodes - (treeEnd - tree) - arity + 1 + argument, full);
            
        }
   }
   return(treeEnd);
}

int fitnessProportionalSelection(){
    
    float randNum = (float)rand()/ (float)RAND_MAX;
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; ++popIndex ){
        
        if(population[popIndex%POPULATION_SIZE].fitness >= randNum) return popIndex;
        else randNum -= population[popIndex].fitness;
    }
    
    //If this line of code is reached it is due to errors in floating point precision
    return POPULATION_SIZE-1;
    
}

int subtreeLength(char* start){
    
    int remaining = primitiveTable[*start].arity;
    int length = 1;
    
    while(remaining > 0){
        
        remaining--;
        start++;
        remaining += primitiveTable[*start].arity;
        length++;
        
    }
    
    return length;
    
}

int subTreeDepth(char* start){
    
    int maxDepth = 0;
    
    int arity = primitiveTable[*start].arity;
    
    start++;
    
    for(int i = 0; i<arity; i++){
        
        int depth = subTreeDepth(start);
        
        if(depth > maxDepth) maxDepth = depth;
        
        start += subtreeLength(start);
        
    }
    
    return maxDepth+1;
    
}

void replaceSubtree(char* baseProg, int baseLen, int oldSubtree, char* newSubtree, char* newProg){
    
    memcpy(newProg, baseProg, oldSubtree);
    int subtreelen = subtreeLength(baseProg + oldSubtree);
    int newSubtreeLen = subtreeLength(newSubtree);
    memcpy(newProg + oldSubtree, newSubtree, newSubtreeLen);
    memcpy(newProg + oldSubtree + newSubtreeLen, baseProg + oldSubtree + subtreelen, baseLen - subtreelen - oldSubtree);
    newProg[baseLen-subtreelen+newSubtreeLen] = '\0';
    
}

void reproduction(int popIndex){
    
    newPopulation[popIndex] = population[fitnessProportionalSelection()];
 
}

void mutate(Prog* baseProg, int popIndex){
    
    //int selectedProg = fitnessProportionalSelection();
    
    int baseDepth = subTreeDepth(baseProg->code);
    int maxDepth = baseDepth * MAX_MUTATE_DEPTH_INCREASE;
    
    int mutatedNode = randRange(0,baseProg->progLen - 1);
    
    int subTreeDep = subTreeDepth(&baseProg->code[mutatedNode]);
    int subTreeLen = subtreeLength(&baseProg->code[mutatedNode]);
    
    char newTree[MAX_PROG_SIZE];
    createTree(newTree, maxDepth - baseDepth + subTreeDep, MAX_PROG_SIZE - baseProg->progLen + subTreeLen, 0);
    
    replaceSubtree(baseProg->code, baseProg->progLen, mutatedNode, newTree, newPopulation[popIndex].code);
    
    newPopulation[popIndex].progLen = strlen(newPopulation[popIndex].code);
    
} 

void crossover(int popIndex1, int popIndex2){
    
    int parIndex1 = fitnessProportionalSelection();
    int parIndex2 = fitnessProportionalSelection();
    while(parIndex1 == parIndex2)parIndex2 = fitnessProportionalSelection();
    
    Prog* parent1 = &population[parIndex1];
    Prog* parent2 = &population[parIndex2];
    
    int crossoverPoint1 = randRange(0,parent1->progLen-1);
    int crossoverPoint2 = randRange(0,parent2->progLen-1);
    
    replaceSubtree(parent1->code, parent1->progLen, crossoverPoint1, parent2->code + crossoverPoint2, newPopulation[popIndex1].code);
    newPopulation[popIndex1].progLen = strlen(newPopulation[popIndex1].code);
    
    if(popIndex2 < POPULATION_SIZE){
        
        replaceSubtree(parent2->code, parent2->progLen, crossoverPoint2, parent1->code + crossoverPoint1, newPopulation[popIndex2].code);
        newPopulation[popIndex2].progLen = strlen(newPopulation[popIndex2].code);
    }
    
}

void initialisePopulation(){
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; ++popIndex){
        
        Prog* prog = &population[popIndex];
        
        char* code = prog->code;
        
        createTree(code, 6, MAX_PROG_SIZE,0);
        
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
    
    successTrace = fopen(OUT_FILE,"w");
    
    for(int run = 0; run<MAX_RUNS; ++run){
        
        //if(numWorkingProgs>0) break;        
        
        printf("\n\nRun %d\n\n",run);
        
        initialisePopulation();
        
        //evaluate the initial population (generation 0)
        evaluatePopulation(0);
        
        for(int generation = 1; generation < NUM_GENERATIONS; ++generation){
            
            //if(numWorkingProgs>0) break;
            
            
            for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
                
                if(randRange(0,9) <= 8){
                        crossover(popIndex,(popIndex+1)%POPULATION_SIZE);
                        ++popIndex;
                }
                else reproduction(popIndex);
                
            }
            
            for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++)
                if(randRange(0,9) == 0){
                    Prog p = newPopulation[popIndex];
                    mutate(&p,popIndex);
                }
            
            Prog* temp = newPopulation;
            newPopulation = population;
            population = temp;
            
            evaluatePopulation(generation);
            
            if(generation%100 == 0){
                
                printf("\n%d", generation);
                
                
            }
            
            
            
        }
        
        fprintf(successTrace,"\n");
    }
    
    //printPopulation();

	if(numWorkingProgs>0){
        printf("\n\nSuccess!\n\n");
        printNode(workingProgramme);
    }
    
    fclose(successTrace);
    
    return 0;
    
}