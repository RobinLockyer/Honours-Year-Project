#include <stdio.h>
#include <stdlib.h>

#define RANDOM_SEED 1894

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMITIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

//Macro to define which version of SNGP is used
#define evaluatePopulation(updateList,testSet) evaluatePopulationSNGP_B((updateList),(testSet))

//The maximum number of times we apply the successor mutate operation
#define MAX_OPS 5000
#define NUM_GENERATIONS MAX_OPS+1
#define POPULATION_SIZE 200
#define NUM_TESTS 15
#define MAX_RUNS 5
#define NUM_TEST_SETS 3000

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
int updateList[POPULATION_SIZE];

int index = 0;
int progIterations = 0;
#define MAX_PROG_ITERATIONS 10000

int success = 0;

//For SNGP/A
float totalNodeFitness = 0;
//For SNGP/B
float bestNodeFitness = 0;

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
    double fitness;
    double oldFitness;
    int operands[MAX_ARITY];
    int predecessors[POPULATION_SIZE];
} Node;

Node population[POPULATION_SIZE];



void addPredecessor(int popIndex, int newPredIndex){
    
    //If the node is a terminal, we don't need to maintain its predecessor array
    if(popIndex < NUM_TERMINALS) return;
    
    Node* node = &population[popIndex];
    
    int* predArray = node->predecessors;
    
    int nextValue = 0;
    
    while(predArray[nextValue] != 0 && predArray[nextValue] < newPredIndex){
        
        nextValue = predArray[nextValue];
        
    }
    
    if(predArray[nextValue] != newPredIndex){
        
        predArray[newPredIndex] = predArray[nextValue];
        
        predArray[nextValue] = newPredIndex;
        
    }
    
    
    
}

int removePredecessor(int popIndex, int newPredIndex){
    
    //If the node is a terminal, we don't need to maintain its predecessor array
    if(popIndex < NUM_TERMINALS) return 0;
    
    Node* node = &population[popIndex];
    
    int* predArray = node->predecessors;
    
    int nextValue = 0;
    
    while(predArray[nextValue] != 0 && predArray[nextValue] != newPredIndex){
        
        nextValue = predArray[nextValue];
        
    }
    
    //if predIndex not found in predArray, return error
    if(predArray[nextValue] == 0) return 1;
    
    predArray[nextValue] = predArray[newPredIndex];
    
    predArray[newPredIndex] = 0;
    
    return 0;
}

//Returns random number in interval [min,max] inclusive
int randRange(int min, int max){
    
    return min + rand()/(RAND_MAX/(max-min+1)+1);
    
}

void initialisePopulation(){
    
    for(int terminalIndex = 0; terminalIndex < NUM_TERMINALS; terminalIndex++){
        
        Node* node = &population[terminalIndex];
        
        node->primitive = terminalIndex;
        
        node->fitness = -1;
        node->oldFitness = -1;
        
    }
    
    for(int functionIndex = NUM_TERMINALS; functionIndex < POPULATION_SIZE; functionIndex++){
        
        Node* node = &population[functionIndex];
        
        Primitive primitive = randRange(NUM_TERMINALS,NUM_PRIMITIVES-1);
        
        node->primitive = primitive;
        node->fitness = -1;
        node->oldFitness = -1;
        
        for(int operandIndex = 0; operandIndex < MAX_ARITY; operandIndex++){
            
            if( operandIndex < primitiveTable[primitive].arity ){
                
                int randomOperand = randRange(0,functionIndex-1);
                
                node->operands[operandIndex] = randomOperand;
                
                addPredecessor(randomOperand, functionIndex);
                
            } else{
                
                node->operands[operandIndex] = -1;
                
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

int execute(int popIndex){
    
    Node* node = &population[popIndex];
    
    switch(node->primitive){
        
        case INDEX:{
            
            return index;
            
        }break;
            
        case LENGTH:{
            
            return results->size;
        
        }break;
            
        case ITERATE:{
                
            int len = results->size;
            int start = execute(node->operands[0]);
            int end = execute(node->operands[1]);
            int functionIndex = node->operands[2];
            
            int oldIndex = index;
            
            for(index = start; index <= end && index < len && progIterations < MAX_PROG_ITERATIONS; ++index, ++progIterations){
                
                execute(functionIndex);
            }
            
            index = oldIndex;
            
            return (end < len) ? end : len ;
                
        }break;
            
        case SWAP:{
            
            int x = execute(node->operands[0]);
            int y = execute(node->operands[1]);
            
            //If x or y is not a valid index, return 0
            if(x<0 || y<0 || x>=results->size || y>=results->size) return 0;
            
            int t = results->arr[x];
            results->arr[x] = results->arr[y];
            results->arr[y] = t;
            
            return x;
            
        }break;
            
        case SMALLEST:{
            
            int x = execute(node->operands[0]);
            int y = execute(node->operands[1]);
            
            if(results->arr[x] < results->arr[y]){
                return x;
            }
            else{
                
                return y;
                
            }
            
        }break;
            
        case LARGEST:{
            
            int x = execute(node->operands[0]);
            int y = execute(node->operands[1]);
            
            if(results->arr[x] > results->arr[y]){
                return x;
            }
            else{
                
                return y;
                
            }
            
        }break;
            
        case SUB:{
            
            int x = execute(node->operands[0]);
            int y = execute(node->operands[1]);
            
            return x-y;
            
        }break;
            
        case INC:{
            
            int x = execute(node->operands[0]);
            
            return x+1;
            
        }break;
            
        case DEC:{
            
            int x = execute(node->operands[0]);
            
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

float evaluateNode(int popIndex, int testSet){
    
    float nodeTotalFitness = 0;
        
    for(int testNum = 0; testNum < NUM_TESTS; testNum++){
        
        nodeTotalFitness += testNode(popIndex, testSet, testNum);
        
    }
    
    population[popIndex].oldFitness = population[popIndex].fitness;
    
    population[popIndex].fitness = nodeTotalFitness / NUM_TESTS;
    
    if(population[popIndex].fitness > 0.8) success = 1;
    
    return population[popIndex].fitness;
    
}

float evaluatePopulationSNGP_A(int* updateList, int testSet){
    
    if(updateList == NULL){
    
        totalNodeFitness = 0;

        for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
            
            totalNodeFitness += evaluateNode(popIndex, testSet);
            
        }
    } else{
        
        for(int nextUpdateNode = updateList[0]; nextUpdateNode != 0; nextUpdateNode = updateList[nextUpdateNode]){
            
            totalNodeFitness += evaluateNode(nextUpdateNode, testSet);
            
            totalNodeFitness -= population[nextUpdateNode].oldFitness;
            
        }
    }
    
    return totalNodeFitness/POPULATION_SIZE;
    
}

float evaluatePopulationSNGP_B(int* updateList, int testSet){
    
    if(updateList == NULL){
    
        bestNodeFitness = evaluateNode(0, testSet);

        for(int popIndex = 1; popIndex < POPULATION_SIZE; popIndex++){
            
            float nodeFitness = evaluateNode(popIndex, testSet);
            if(nodeFitness > bestNodeFitness) bestNodeFitness = nodeFitness;
            
        }
    } else{
        
        for(int nextUpdateNode = updateList[0]; nextUpdateNode != 0; nextUpdateNode = updateList[nextUpdateNode]){
            
            evaluateNode(nextUpdateNode, testSet);
            
        }
        
        bestNodeFitness = population[0].fitness;
        
        for(int i = 1; i<POPULATION_SIZE; ++i){
            
            if(bestNodeFitness < population[i].fitness) bestNodeFitness = population[i].fitness;
            
        }
    }
    
    return totalNodeFitness/POPULATION_SIZE;
}

void successorMutate(int popIndex, int randomOperandIndex, int newOperandValue){
    
    Node* node = &population[popIndex];
    
    removePredecessor(node->operands[randomOperandIndex], popIndex);
    
    node->operands[randomOperandIndex] = newOperandValue;
    
    addPredecessor(newOperandValue, popIndex);
}

void addToUpdateList(int popIndex){
    
    int nextValue = 0;
    
    while(updateList[nextValue] != 0 && updateList[nextValue] < popIndex){
        
        nextValue = updateList[nextValue];
        
    }
    
    if(updateList[nextValue] != popIndex){
        
        updateList[popIndex] = updateList[nextValue];
        
        updateList[nextValue] = popIndex;
        
    }
    
    
}

//updateList[0] must be set to 0 before this function is called
void buildUpdateList(int popIndex){
    
    addToUpdateList(popIndex);
    
    int* predArray = population[popIndex].predecessors;

    int nextPredecessor = predArray[0];
    
    while(nextPredecessor != 0){
        
        addToUpdateList(nextPredecessor);
        buildUpdateList(nextPredecessor);
        nextPredecessor = predArray[nextPredecessor];
        
    }
    
}

void restoreFitnessValues(int* list){
    
    int nextNode = list[0];
    
    while(nextNode != 0 && nextNode < POPULATION_SIZE){
        
        population[nextNode].fitness = population[nextNode].oldFitness;
        
        nextNode = list[nextNode];
        
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

#include "SNGP_Sort_Debug.c"

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
    
    /*
    printf("Inversions %d", countInversions(tests[0][0]));
    
    printIntArray(mergeBuffer1,20);
    
    printIntArray(mergeBuffer2,20);*/
    
    
    for(int run = 0; run<MAX_RUNS; ++run){
        
        if(success==1) break;
        
        printf("\n\nRun %d\n\n",run);
        
        initialisePopulation();
        //initialiseExamplePopulation();
        float oldFitness = -1;
        
        //evaluate the initial population (generation 0)
        float fitness = evaluatePopulation(NULL, 0);
        
        
        for(int generation = 1; generation < NUM_GENERATIONS; ++generation){
            
            if(success==1) break;
            
            
            
            int randomNodeIndex = randRange(NUM_TERMINALS, NUM_PRIMITIVES-1);
            Node* randomNode = &population[randomNodeIndex];
            int randomOperandIndex = randRange(0, primitiveTable[randomNode->primitive].arity-1);
            int oldOperandValue = randomNode->operands[randomOperandIndex];
            int randomOperandValue = randRange(0,randomNodeIndex-1);
            
            successorMutate(randomNodeIndex, randomOperandIndex, randomOperandValue);
            
            updateList[0] = 0;
            
            buildUpdateList(randomNodeIndex);
            
            oldFitness = fitness;
            
            fitness = evaluatePopulation(updateList, generation % NUM_TEST_SETS);
            
            if(oldFitness >= fitness){
                
                restoreFitnessValues(updateList);
                
                fitness = oldFitness;
                
                removePredecessor(randomOperandValue, randomNodeIndex);
                
                randomNode->operands[randomOperandIndex] = oldOperandValue;
                
                addPredecessor(oldOperandValue, randomNodeIndex);
                
            }      
            
            if(generation%100 == 0){
                
                printf("\n%d", generation);
                
                
            }
            
        }
    }
    printPopulation();
    
    
    
    return 0;
}