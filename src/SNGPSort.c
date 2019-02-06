#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

#define RANDOM_SEED 1892

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 7
#define NUM_PRIMITIVES NUM_TERMINALS+NUM_FUNCTIONS
#define MAX_ARITY 3

#define MAX_OPS 3

#define POPULATION_SIZE 20

#define NUM_TESTS 5

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

int index = 0;

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
    double fitness;
    double oldFitness;
    int operands[MAX_ARITY];
    int predecessors[POPULATION_SIZE];
} Node;

Node population[POPULATION_SIZE];

void printPrimitiveTable(){
    
    for(int i = 0; i < NUM_PRIMITIVES; i++){
        
        TableEntry entry = primitiveTable[i];
        printf("i: %d, e: %d, a: %d n: %s\n", i, entry.primitive, entry.arity, entry.name);
        
    }
    
    printf("\n");
    
}

void addPredecessor(int popIndex, int predIndex){
    
    //If the node is a terminal, we don't need to maintain its predecessor array
    if(popIndex < NUM_TERMINALS) return;
    
    Node* node = &population[popIndex];
    
    int* predArray = node->predecessors;
    
    int nextValue = predArray[0];
    
    while(nextValue != 0 && predArray[nextValue] >= predIndex){
        
        nextValue = predArray[nextValue];
        
        
    }
    
    if(nextValue != predIndex){
        
        predArray[predIndex] = predArray[nextValue];
        
        predArray[nextValue] = predIndex;
        
    }
    
    
    
}

void removePredecessor(int popIndex, int predIndex){
    
    //If the node is a terminal, we don't need to maintain its predecessor array
    if(predIndex < NUM_TERMINALS) return;
    
    int* predArray = population[popIndex].predecessors;
    
    
    
}

//Returns random number in interval [min,max] inclusive
int randRange(int min, int max){
    
    return min + rand()/(RAND_MAX/(max-min+1)+1);
    
}

void initialisePopulation(){
    
    for(int terminalIndex = 0; terminalIndex < NUM_TERMINALS; terminalIndex++){
        
        population[terminalIndex].primitive = terminalIndex;
        
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
            "Index: %d primitive: %s Arity: %d\nFitness: %f OldFitness: %f\nOperands:", 
            popIndex,
            primitiveTable[node.primitive].name,
            primitiveTable[node.primitive].arity,
            node.fitness,
            node.oldFitness
        );
        
        for(int j = 0; j < primitiveTable[node.primitive].arity; j++ ){
            printf("%d ",node.operands[j]);
        }
        
        printf("\nPredecessors: ");
        
        for(int j = node.predecessors[0]; j < POPULATION_SIZE && j != 0; j = node.predecessors[j]){
            
            printf("%d ",j);
            
        }
        
        printf("\n\n");
    }
    
    printf("\n");
    
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
			
            for(index = start; index <= end && index < len; ++index){
				
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

float evaluateNode(int popIndex, int testSet, int testNum){
    
    Array* test = tests[testSet][testNum];
    
	//If inversions not counted, count inversions
    if(test->inversions == -1){
        
        memcpy(results, test->arr, test->size);
        countInversions(test);
        
    }
	
	index = 0;
    
	memcpy(results, test, arrayMem(test->size));
	execute(popIndex);
	
	float fitness = 1.0/(1+countInversions(results));
    
    return fitness; 
    
}

float evaluatePopulationSNGP_A(){
    
    float totalFitness = 0;
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
        
		float nodeTotalFitness = 0;
		
        for(int testNum = 0; testNum < NUM_TESTS; testNum++){
            
			nodeTotalFitness += evaluateNode(popIndex, generation, testNum);
            
        }
		
		population[popIndex].oldFitness = population[popIndex].fitness;
		
		population[popIndex].fitness = nodeTotalFitness / NUM_TESTS;
		
		totalFitness += population[popIndex].fitness;
        
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
	population[5].operands[0] = 1;
	population[5].operands[1] = 1;
	
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
	
	population[9].primitive = SWAP;
	population[9].operands[0] = 6;
	population[9].operands[1] = 0;
	
}

void testExecution(){
	
	initialiseExamplePopulation();
	printPopulation();
	
    results = malloc(arrayMem(maxTestSize));
    
    printf("Test data initialised\n");
    
	int testResult = evaluateNode(8,2,1);
    
	for(int i = 0; i < results->size; i++){
        
        printf("%d ", results->arr[i]);
        
    }
	
}


void successorMutate(int popIndex){
	
	
	Node* node = &population[popIndex];
	
	int randomOperandIndex = randRange(0, primitiveTable[node->primitive].arity-1);
	
	removePredecessor(node->operands[randomOperandIndex], popIndex);
	
	int newOperand = randRange(0,popIndex-1);
	
	node->operands[randomOperandIndex] = newOperand;
	
	addPredecessor(newOperand, popIndex);
}

int main(int argc, char* argv[]){
    
    printf("Start\n\n");
    printf(argv[1]);
    printf("\n\n");
    
    printPrimitiveTable();
   
    if(init(argv[1])){
        printf("File Not Found");
        return 1;
    }
    
    //printTestData();
    
    printf("SNGP/A Fitness: %f\n\n",evaluatePopulationSNGP_A());
    
    printPopulation();
	
    return 0;
}