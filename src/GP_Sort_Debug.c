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
        Prog prog = population[popIndex];
        printf(
            "Index: %d ProgLen: %d Fitness: %f \nProg: ", 
            popIndex,
            prog.progLen,
            prog.fitness
        );
        
        for(int i = 0; i < prog.progLen; ++i){
            
            
            printf("%s ",primitiveTable[prog.code[i]].name);
            
        }
        
        printf("\n\n");
    }
    
    printf("\n");
    
}

#endif