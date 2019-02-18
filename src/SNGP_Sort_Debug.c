#ifndef SNGP_SORT_DEBUG_C
#define SNGP_SORT_DEBUG_C

#include<stdio.h>

void printIntArray(int* a, int n){
    
    for(int i = 0; i<n; i++){
        
        printf("%d ", a[i]);
        
    }
    
}

void printPrimitiveTable(){
    
    for(int i = 0; i < NUM_PRIMITIVES; i++){
        
        TableEntry entry = primitiveTable[i];
        printf("i: %d, e: %d, a: %d n: %s\n", i, entry.primitive, entry.arity, entry.name);
        
    }
    
    printf("\n");
    
}

void printTestData(){
    
    for(int i = 0; i < NUM_GENERATIONS; i++){
        
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

void printPopulation(){
    
    
    for(int popIndex = 0; popIndex < POPULATION_SIZE; popIndex++){
        Node node = population[popIndex];
        printf(
            "Index: %d primitive: %s Arity: %d\nFitness: %f OldFitness: %f\nOperands: ", 
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
        
        for(int j = 0; j < POPULATION_SIZE; ++j){
            
            if(node.predecessors[j]!=0)printf("%d ",node.predecessors[j]);
            
        }
        
        printf("\n\n");
    }
    
    printf("\n");
    
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
    
	int testResult = testNode(8,2,1);
    
	for(int i = 0; i < results->size; i++){
        
        printf("%d ", results->arr[i]);
        
    }
	
}

#endif