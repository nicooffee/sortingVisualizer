typedef struct Array * Array;

extern Array    array_create();
extern void *   array_get(Array A,int index);
extern void     array_add(Array A, void *elem);
extern void *   array_set(Array A,int index,void *elem);
extern void *   array_del(Array A,int index);
extern int      array_length(Array A);
extern void     array_swap(Array A, int iA, int iB);
extern void     array_new_capacity(Array A,int newCapacity);
extern void     array_free(Array A);
extern void     array_free_all(Array A);