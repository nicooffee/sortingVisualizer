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
*/
#define ALGORITMO &bubblesort
#define DELAY 5000

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
void set_frame(WINDOW *w);
Array create_array(WINDOW *w);
void *print_array(void *m);
void *print_data(void *m);
int comp_int(void *a,void *b);

void *bubblesort(void *m);

int partition(Array A,int min,int max,int (*comp)(void *,void *));
void quicksort(Array A,int min,int max,int (*comp)(void *,void *));
void *quickInit(void *m);

pthread_mutex_t mutex_screen=PTHREAD_MUTEX_INITIALIZER;         //Mutex para sincronizar las ventanas.
pthread_mutex_t mutex_array=PTHREAD_MUTEX_INITIALIZER;    //Mutex para sincronizar el uso de lista de participantes.
pthread_cond_t  cond_swap=PTHREAD_COND_INITIALIZER;  
int swap = 0;
int compar = 0;
int posI,posJ;

int main(){
    struct Message *m = (struct Message*) calloc(1,sizeof(struct Message));
    pthread_t t_sort,t_w_sort,t_w_data;
    Array A;
    WINDOW *w_sort,*w_data;
    int maxx,maxy;
    init_setup();
    set_frame(stdscr);
    getmaxyx(stdscr,maxy,maxx);
    w_sort = derwin(stdscr,maxy-6,maxx-2,1,1);
    w_data = derwin(stdscr,3,maxx-2,maxy-4,1);
    A = create_array(w_sort);
    m->A = A;
    m->w_sort = w_sort;
    m->w_data = w_data;
    m->func = &(comp_int);
    pthread_create(&t_sort,NULL,ALGORITMO,(void *)m);
    pthread_create(&t_w_sort,NULL,&print_array,(void *)m);
    pthread_create(&t_w_data,NULL,&print_data,(void *)m);
    pthread_join(t_sort,NULL);
    pthread_join(t_w_sort,NULL);
    pthread_join(t_w_data,NULL);
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
    while(1){
        pthread_mutex_lock(&mutex_screen);
        pthread_cond_wait(&cond_swap,&mutex_screen);
        pthread_mutex_lock(&mutex_array);
        wclear(w);
        for(i=0;i<array_length(A);i++){
            wattron(w,COLOR_PAIR( (i==posI || i==posJ)?ROJO:VERDE ));
            mvwvline(w,maxy-1-*((int *)array_get(A,i)),i+1,' ',maxy);
            mvwprintw(w,maxy-1-*((int *)array_get(A,i)),i+1,"*");
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
    while(1){
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
    for(i=0;i<maxx-2;i++)
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
    for(i=0;i<array_length(A)-1;i++)
        for(j=i+1;j<array_length(A);j++){
            pthread_mutex_lock(&mutex_array);
            compar++;
            if(comp(array_get(A,j),array_get(A,i)))
                (pthread_cond_signal(&cond_swap),swap++,array_swap(A,i,j));
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
    while(min<max){
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

/**************************************************/
/**************************************************/







void set_frame(WINDOW *w){
    int maxx,maxy;
    getmaxyx(w,maxy,maxx);
    wattron(w,COLOR_PAIR(BLANCO));
    box(w,'*','*');
    mvwhline(w,maxy-5,1,'*',maxx-2);
    wattroff(w,COLOR_PAIR(BLANCO));
    wnoutrefresh(stdscr);
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