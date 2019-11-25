#include <stdio.h>
#include <stdlib.h>
#include "array.h"

int main(){
    int i;
    int *aux;
    Array A = array_create();
    for(i=0;i<10;i++){
        aux = (int*) calloc(1,sizeof(int));
        *aux = i;
        array_add(A,aux);

    }
    printf("length: %d\n",array_length(A));
    free(array_del(A,4));
    for(i=0;i<array_length(A);i++){
        aux = array_get(A,i);
        printf("%d ",*aux);
    }
    printf("length: %d\n",array_length(A));
    array_new_capacity(A,100);
    array_swap(A,5,1);
    for(i=0;i<array_length(A);i++){
        aux = array_get(A,i);
        printf("%d ",*aux);
    }
    printf("length: %d\n",array_length(A));
    array_free_all(A);
    return 0;
}