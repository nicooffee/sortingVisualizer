#include <stdlib.h>
#include "array.h"

struct Array{
    void **array;
    int capacity;
    int length; 
};



Array array_create(){
    Array A = NULL;
    A = (struct Array*) calloc(1,sizeof(struct Array));
    A->capacity = 1;
    A->length = 0;
    A->array = (void**) calloc(1,sizeof(void*));
    return A;
}



void *array_get(Array A,int index){
    return (A==NULL ||\
            A->array==NULL ||\
            A->length<=index)?\
                NULL:\
                A->array[index];
}



void array_add(Array A, void *elem){
    if(A->capacity==A->length) array_new_capacity(A,A->capacity*2);
    A->array[A->length++] = elem;
    return;
}



void *array_set(Array A,int index,void *elem){
    void *aux;
    if(A==NULL || A->array==NULL || elem==NULL) return NULL;
    if(index<0 || index>=A->length) return NULL;
    aux = A->array[index];
    A->array[index] = elem;
    return aux;
}



void *array_del(Array A,int index){
    void *aux = NULL;
    int i;
    if(A==NULL || A->array==NULL || A->length<=index) return NULL;
    aux = A->array[index];
    for(i=index;i<A->length-1;i++)
        A->array[i]=A->array[i+1];
    A->array[--A->length]=NULL;
    return aux;
}



int array_length(Array A){
    return A->length;
}



void array_swap(Array A, int iA, int iB){
    if(iA>=A->length || iB>=A->length) return;
    array_set(A,iA,array_set(A,iB,array_get(A,iA)));
}



void array_new_capacity(Array A,int newCapacity){
    void **aux;
    int capacity;
    int length;
    if(A==NULL || A->array==NULL || newCapacity<=0) return;
    aux = A->array;
    capacity = A->capacity;
    length = A->length;
    A->array = (void**) realloc(A->array,sizeof(void*)*newCapacity);
    A->capacity = newCapacity;
    if(length>newCapacity) A->length = newCapacity;
    if(A->array==NULL){
        A->array = aux;
        A->capacity = capacity;
        A->length = length;
    }
    return;
}




void array_free(Array A){
    if(A->array!=NULL) free(A->array);
    if(A!=NULL) free(A);
    return;
}



void array_free_all(Array A){
    int i=0;
    for(;i<A->length;i++)
        if(A->array[i]!=NULL)
            free(A->array[i]);
    array_free(A);
    return;
}