#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include "array/array.h"
/** ALGORTIMOS
 *  - bubblesort
 *  - quickInit (quicksort)
 *  - mergeInit (mergesort)
*/
#define ALGORITMO &mergeInit
#define DELAY 30000

#define ROJO        1
#define VERDE       2
#define AMARILLO    3
#define CYAN        4
#define MAGENTA     5
#define BLANCO      6
#define AZUL        7
#define BLANCO_ROJO     8
#define BLANCO_VERDE    9
#define BLANCO_AMARILLO 10
#define BLANCO_CYAN     11
#define BLANCO_MAGENTA  12
#define NEGRO_BLANCO    13
#define BLANCO_AZUL     14
#define BLANCO_NEGRO    15

struct Message{
    Array A;
    WINDOW *w_sort;
    WINDOW *w_data;
    void *func;
};

void init_setup();
void set_frame(WINDOW *w,int l_w1);
Array create_array(WINDOW *w);
void *print_array(void *m);
void *print_data(void *m);
int comp_int(void *a,void *b);

void *bubblesort(void *m);

int partition(Array A,int min,int max,int (*comp)(void *,void *));
void quicksort(Array A,int min,int max,int (*comp)(void *,void *));
void *quickInit(void *m);

void merge(Array A,int inicio, int mitad,int final,int (*comp)(void *,void *));
void mergesort(Array A,int inicio,int final,int (*comp)(void *,void *));
void *mergeInit(void *m);


void *stop_ejec(void *message);

pthread_mutex_t mutex_screen=PTHREAD_MUTEX_INITIALIZER;         //Mutex para sincronizar las ventanas.
pthread_mutex_t mutex_array=PTHREAD_MUTEX_INITIALIZER;    //Mutex para sincronizar el uso de lista de participantes.
pthread_cond_t  cond_swap=PTHREAD_COND_INITIALIZER;  
int swap = 0;
int compar = 0;
int posI,posJ;

int ejec_flag = 1;

int main(){
    struct Message *m = (struct Message*) calloc(1,sizeof(struct Message));
    pthread_t t_sort,t_w_sort,t_w_data,t_ejec;
    Array A;
    WINDOW *w_sort,*w_data;
    int maxx,maxy;
    init_setup();
    getmaxyx(stdscr,maxy,maxx);
    set_frame(stdscr,maxy-6);
    w_sort = derwin(stdscr,maxy-6,maxx-6,1,4);
    w_data = derwin(stdscr,3,maxx-2,maxy-4,1);
    A = create_array(w_sort);
    m->A = A;
    m->w_sort = w_sort;
    m->w_data = w_data;
    m->func = &(comp_int);
    pthread_create(&t_sort,NULL,ALGORITMO,(void *)m);
    pthread_create(&t_w_sort,NULL,&print_array,(void *)m);
    pthread_create(&t_w_data,NULL,&print_data,(void *)m);
    pthread_create(&t_ejec,NULL,&stop_ejec,(void *)m);
    pthread_join(t_sort,NULL);
    pthread_join(t_w_sort,NULL);
    pthread_join(t_w_data,NULL);
    pthread_join(t_ejec,NULL);
    print_array(m);
    array_free_all(A);
    pthread_mutex_destroy(&mutex_array);
    pthread_mutex_destroy(&mutex_screen);
    pthread_cond_destroy(&cond_swap);
    delwin(w_data);
    delwin(w_sort);
    endwin();
}



void *print_array(void *m){
    Array A = ((struct Message*) m)->A;
    WINDOW *w = ((struct Message*) m)->w_sort;
    register int i;
    int maxy = getmaxy(w);
    while(ejec_flag){
        pthread_mutex_lock(&mutex_screen);
        pthread_cond_wait(&cond_swap,&mutex_screen);
        pthread_mutex_lock(&mutex_array);
        wclear(w);
        for(i=0;i<array_length(A);i++){
            wattron(w,COLOR_PAIR(BLANCO));
            mvwprintw(w,maxy-1-*((int *)array_get(A,i)),i,"*");
            wattroff(w,COLOR_PAIR(BLANCO));
            wattron(w,COLOR_PAIR( (i==posI || i==posJ)?ROJO:VERDE ));
            mvwvline(w,maxy-*((int *)array_get(A,i)),i,' ',maxy);
            wattroff(w,COLOR_PAIR( (i==posI || i==posJ)?ROJO:VERDE ));
        }
        wnoutrefresh(w);
        pthread_mutex_unlock(&mutex_screen);
        pthread_mutex_unlock(&mutex_array);
    }
}



void *print_data(void *m){
    WINDOW *w= ((struct Message*) m)->w_data;
    Array A = ((struct Message*) m)->A;
    pthread_mutex_lock(&mutex_array);
    mvwprintw(w,0,0,"C:\t%10d",array_length(A));
    pthread_mutex_unlock(&mutex_array);
    while(ejec_flag){
        pthread_mutex_lock(&mutex_screen);
        mvwprintw(w,1,0,"SWAP:\t%10d",swap);
        mvwprintw(w,2,0,"<:\t%10d",compar);
        wnoutrefresh(w);
        doupdate();
        pthread_mutex_unlock(&mutex_screen);
        usleep(DELAY);
    }
}









Array create_array(WINDOW *w){
    int maxx,maxy;
    int i,*aux;
    Array A=array_create();
    getmaxyx(w,maxy,maxx);
    for(i=0;i<maxx;i++)
        array_add(A,(aux=(int*) malloc(sizeof(int)),\
                    *aux = rand()%(maxy-2),\
                    aux));
    return A;
}
int comp_int(void *a,void *b){
    return *((int *)a)<=*((int *)b);
}



/*************************BUBBLE*************************/
void *bubblesort(void *m){
    register int i,j;
    Array A = ((struct Message *) m)->A;
    int (*comp)(void *,void *) = ((struct Message *) m)->func;
    for(i=0;i<array_length(A)-1 && ejec_flag;i++)
        for(j=i+1;j<array_length(A) && ejec_flag;j++){
            pthread_mutex_lock(&mutex_array);
            compar++;
            if(comp(array_get(A,j),array_get(A,i)))
                (swap++,array_swap(A,i,j));
            pthread_cond_signal(&cond_swap);
            posI = i;
            posJ = j;
            pthread_mutex_unlock(&mutex_array);
            usleep(DELAY);
        }
}
/*************************QUICK*************************/

int partition(Array A,int min,int max,int (*comp)(void *,void *)){
    void *pivot = array_get(A,max);
    int auxMin = (min-1);
    for (register int i = min;i<=max-1;i++){
        if(comp(array_get(A,i),pivot)){
            swap++;
            pthread_cond_signal(&cond_swap);
            array_swap(A,++auxMin,i);
            posI=auxMin;
            posJ=i;
        }
        compar++;
    }
        
    array_swap(A,auxMin+1,max);
    return auxMin+1;
}

void quicksort(Array A,int min,int max,int (*comp)(void *,void *)){
    if(!ejec_flag) return;
    while(min<max && ejec_flag){
        pthread_mutex_lock(&mutex_array);
        int part = partition(A,min,max,comp);
        pthread_mutex_unlock(&mutex_array);
        usleep(DELAY);
        if((part-min)<(part-max)){
            quicksort(A,min,part-1,comp);
            min=part+1;
        }
        else{
            quicksort(A,part+1,max,comp);
            max = part -1;
        }

    }
}

void *quickInit(void *m){
    Array A = ((struct Message *) m)->A;
    int (*comp)(void *,void *) = ((struct Message *) m)->func;
    quicksort(A,0,array_length(A)-1,comp);
}

/*************************MERGE*************************/

void merge(Array A,int inicio, int mitad,int final,int (*comp)(void *,void *)){
    int i=0,j=0,k=inicio,lA=(mitad-inicio+1),lB=(final-mitad);
    Array B,C;
    B = array_create();
    C = array_create();
    while((i++,i-1)<lA) array_add(B,array_get(A,inicio+(i-1)));
    while((j++,j-1)<lB) array_add(C,array_get(A,mitad+1+(j-1)));
    i=j=0;
    while(i<lA && j<lB){
        pthread_mutex_lock(&mutex_array);
        if(comp(array_get(B,i),array_get(C,j)))
            array_set(A,(k++,k-1),array_get(B,(i++,i-1)));
        else
            array_set(A,(k++,k-1),array_get(C,(j++,j-1)));
        compar++;
        posI = inicio+i;
        posJ = mitad+j;
        pthread_cond_signal(&cond_swap);      
        pthread_mutex_unlock(&mutex_array);
        usleep(DELAY);
    }

    while(i<lA){
        pthread_mutex_lock(&mutex_array);
        array_set(A,(k++,k-1),array_get(B,(i++,i-1)));
        posI = inicio+i;
        posJ = mitad+j;
        pthread_cond_signal(&cond_swap);      
        pthread_mutex_unlock(&mutex_array);
        usleep(DELAY);
    } 
    while(j<lB){
        pthread_mutex_lock(&mutex_array);
        array_set(A,(k++,k-1),array_get(C,(j++,j-1)));
        posI = inicio+i;
        posJ = mitad+j;
        pthread_cond_signal(&cond_swap);      
        pthread_mutex_unlock(&mutex_array);
        usleep(DELAY);
    } 
    array_free(B);
    array_free(C);
}
void mergesort(Array A,int inicio,int final,int (*comp)(void *,void *)){
    int mitad = (inicio+final)/2;
    int i = 0;
    if(inicio == final) return;
    else if (inicio==final-1){
        if(!comp(array_get(A,inicio),array_get(A,final)))
            array_swap(A,inicio,final);
        swap++;
        return;
    }
    else if(inicio<mitad && mitad<final){
        mergesort(A,inicio,mitad,comp);
        mergesort(A,mitad+1,final,comp);
        merge(A,inicio,mitad,final,comp);
    }
    return;
}

void *mergeInit(void *m){
    Array A = ((struct Message *) m)->A;
    int (*comp)(void *,void *) = ((struct Message *) m)->func;
    mergesort(A,0,array_length(A)-1,comp);
}

/**************************************************/
/**************************************************/







void set_frame(WINDOW *w,int l_w1){
    int maxx,maxy;
    int j = 0;
    getmaxyx(w,maxy,maxx);
    wattron(w,COLOR_PAIR(BLANCO));
    box(w,'*','*');
    mvwhline(w,maxy-5,1,'*',maxx-2);
    wattroff(w,COLOR_PAIR(BLANCO));
    wattron(w,COLOR_PAIR(BLANCO_CYAN));
    for(;j<l_w1;j++)
        mvwprintw(w,l_w1-j,1,"%d",j);
    wattroff(w,COLOR_PAIR(BLANCO_CYAN));
    wnoutrefresh(w);
}



void init_setup(){
    initscr();
    srand((unsigned) time(NULL));   
    noecho();
    start_color();
    init_pair(ROJO,COLOR_RED,COLOR_RED);
    init_pair(VERDE,COLOR_GREEN,COLOR_GREEN);
    init_pair(AMARILLO,COLOR_YELLOW,COLOR_YELLOW);
    init_pair(CYAN,COLOR_CYAN,COLOR_CYAN);
    init_pair(MAGENTA,COLOR_MAGENTA,COLOR_MAGENTA);
    init_pair(BLANCO,COLOR_WHITE,COLOR_WHITE);
    init_pair(AZUL,COLOR_BLUE,COLOR_BLUE);
    init_pair(BLANCO_ROJO,COLOR_WHITE,COLOR_RED);
    init_pair(BLANCO_VERDE,COLOR_WHITE,COLOR_GREEN);
    init_pair(BLANCO_AMARILLO,COLOR_WHITE,COLOR_YELLOW);
    init_pair(BLANCO_CYAN,COLOR_WHITE,COLOR_CYAN);
    init_pair(BLANCO_MAGENTA,COLOR_WHITE,COLOR_MAGENTA);
    init_pair(BLANCO_NEGRO,COLOR_WHITE,COLOR_BLACK);
    init_pair(BLANCO_AZUL,COLOR_WHITE,COLOR_BLUE);
    init_pair(NEGRO_BLANCO,COLOR_BLACK,COLOR_WHITE);
    curs_set(FALSE);
}




void *stop_ejec(void *message){
    WINDOW *w  = ((struct Message*) message)->w_data;
    while(wgetch(w)!=27);
    pthread_cond_signal(&cond_swap);
    ejec_flag = 0;
    return NULL;
}