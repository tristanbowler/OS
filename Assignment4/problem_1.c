#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

// -- Globals --
static long num_cats;
static long num_dogs;
static long num_birds;

static long cats_played;
static long dogs_played;
static long birds_played; 

static long total_animals_played;

//Included from sample backgroun monitor
void panic_on_failure(int e, const char* file, int line){
    if(e){
        fprintf(stderr, "Failure at %s:%d\n", file, line);
        exit(-1);
    }
}

//Included from sample backgroun monitor
#define try(expr) panic_on_failure((expr), __FILE__, __LINE__)

// -- Begin counter monitor --

//Based from sample backgroun monitor
struct counter {
  pthread_mutex_t mutex;
  pthread_cond_t count_changed;
  int bird_count;
  int cat_count;
  int dog_count;
};

//Custom flag struct signaled from main to threads
struct flag{
    pthread_mutex_t mutex;
    int break_flag;
};



//Included from sample backgroun monitor
struct counter counter = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0 , 0, 0};

struct flag flag = {PTHREAD_MUTEX_INITIALIZER, 0}; 

//Based from sample backgroun monitor
void play(void) {
  try(pthread_mutex_lock(&counter.mutex));
  int i;
  total_animals_played++;
  for (i=0; i<10; i++) {
    assert(counter.cat_count >= 0 && counter.cat_count <= num_cats);
    assert(counter.dog_count >= 0 && counter.dog_count <= num_dogs);
    assert(counter.bird_count >= 0 && counter.bird_count <= num_birds);
    assert(counter.cat_count == 0 || counter.dog_count == 0);
    assert(counter.cat_count == 0 || counter.bird_count == 0);
  }
  try(pthread_mutex_unlock(&counter.mutex));
}


//Cat exiting playground
void cat_exit(void)
{
    try(pthread_mutex_lock(&counter.mutex));
    counter.cat_count--;
    try(pthread_cond_signal(&counter.count_changed));
    try(pthread_mutex_unlock(&counter.mutex));
}

//Cat entering playground
void cat_enter(void)
{
    try(pthread_mutex_lock(&counter.mutex));
    while(counter.dog_count>0 || counter.bird_count>0)
    {
        try(pthread_cond_wait(&counter.count_changed, &counter.mutex));
    }
        
    counter.cat_count++;
    cats_played++;
    
    try(pthread_cond_signal(&counter.count_changed));
    try(pthread_mutex_unlock(&counter.mutex));
}

//Entry into cat functions called by each cat on pthread_create
void* cat(void* args){
    int break_check = 0; 
    while(!break_check){
        cat_enter();
        play();
        cat_exit(); 
        try(pthread_mutex_lock(&flag.mutex));
        break_check = flag.break_flag; 
        try(pthread_mutex_unlock(&flag.mutex));
    }
    return NULL;
}

//Dog exits playground
void dog_exit(void)
{ 
    try(pthread_mutex_lock(&counter.mutex));
    counter.dog_count--;
    try(pthread_cond_signal(&counter.count_changed));
    try(pthread_mutex_unlock(&counter.mutex));
}

//Dog enters playground
void dog_enter(void)
{
    try(pthread_mutex_lock(&counter.mutex));
    while(counter.cat_count > 0){
        try(pthread_cond_wait(&counter.count_changed, &counter.mutex));
    } 

    counter.dog_count++; 
    dogs_played ++; 
    
    try(pthread_cond_signal(&counter.count_changed));
    try(pthread_mutex_unlock(&counter.mutex));
}

//Entry into dog functions called by each dog on pthread_create
void* dog(void* args){
    int break_check = 0; 
    while(!break_check){
        dog_enter();
        play();
        dog_exit(); 
        try(pthread_mutex_lock(&flag.mutex));
        break_check = flag.break_flag; 
        try(pthread_mutex_unlock(&flag.mutex));
    }
    return NULL;
}

//Bird exits playground
void bird_exit(void)
{
    try(pthread_mutex_lock(&counter.mutex));
    counter.bird_count--;
    try(pthread_cond_signal(&counter.count_changed));
    try(pthread_mutex_unlock(&counter.mutex));
}

//Bird enters playground
void bird_enter(void)
{
    try(pthread_mutex_lock(&counter.mutex));
    while(counter.cat_count > 0){
        try(pthread_cond_wait(&counter.count_changed, &counter.mutex));
    }
    counter.bird_count++;
    try(pthread_cond_signal(&counter.count_changed)); 
    birds_played++;
    try(pthread_mutex_unlock(&counter.mutex));
}

//Entry into bird functions called by each bird on pthread_create
void* bird(void* args){
    int break_check = 0; 
    while(!break_check){
        bird_enter();
        play();
        bird_exit(); 
        try(pthread_mutex_lock(&flag.mutex));
        break_check = flag.break_flag; 
        try(pthread_mutex_unlock(&flag.mutex));
    }
    return NULL;
}



/**
Your problem 1 executable should take three command line arguments: the number of cats, dogs, 
and birds. Your main function should create the proper number of threads and then they should 
repeatedly attempt to enter the playground until 10 seconds have passed, at which point each 
thread should exit. The main function should join all of the created threads, then print out 
how many times the cats, dogs, and birds were able to enterquit
 the playground.

Between 0 and 99 of each kind of animal must be supported. You must perform error checking and 
exit with a friendly error message if the number of command line arguments is incorrect or if 
an argument isn't a number of is a number that is out of range.
**/

int main(int argc, char* argv[]) 
{
    //ARGUMENT INTAKE
  if(argc !=4)
  {
    printf("Wrong number of arguments provided. Must have number of cats, dogs, and birds.\n");
  }

  cats_played =0; 
  dogs_played = 0; 
  birds_played = 0; 
  total_animals_played =0; 

  char* cat_str;
  num_cats = -1;
  num_cats = strtol(argv[1], &cat_str, 10);
  char* dog_str;
  num_dogs = -1;
  num_dogs = strtol(argv[2], &dog_str, 10);
  char* bird_str;
  num_birds = -1;
  num_birds= strtol(argv[3], &bird_str, 10);

  if(0 != strcmp(cat_str, "") || 0 != strcmp(dog_str, "")  || 0 != strcmp(bird_str, "") )
  {
    printf("Arguments must be numbers\n");
    exit(EXIT_FAILURE);
  }
  else if(num_cats < 0 || num_dogs < 0 || num_birds < 0)
  {
    printf("All arguments must be at least 0.\n");
    exit(EXIT_FAILURE);
  }
  else if(num_cats > 99 || num_dogs > 99 || num_birds > 99)
  {
    printf("All arguments must be at least 0.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    //ARGUMENTS CORRECT CREATE THREADS
    pthread_t cat_thread_ids[99];
    pthread_t dog_thread_ids[99];
    pthread_t bird_thread_ids[99];
    int c;
    for(c=0; c<num_cats; c++)
    {
        try(pthread_create(&cat_thread_ids[c], NULL, cat, (void *)num_cats));
    }
    
    int d;
    for(d=0; d<num_dogs; d++){
        try(pthread_create(&dog_thread_ids[d], NULL, dog, (void *)num_dogs));
    }
   
    int b;
    for(b=0; b<num_birds; b++){
        try(pthread_create(&bird_thread_ids[b], NULL, bird, (void *)num_birds));
    }
    
    //Take a nap
    sleep(10);

    //Throw a flag
    try(pthread_mutex_lock(&flag.mutex));
    flag.break_flag =1; 
    try(pthread_mutex_unlock(&flag.mutex));

    //Join all of the threads
    for(c=0; c<num_cats; c++){
        try(pthread_join(cat_thread_ids[c], NULL));
    }
    for(d=0; d<num_dogs; d++){
        try(pthread_join(dog_thread_ids[d], NULL)); 
    }
    for(b=0; b<num_birds; b++){
        try(pthread_join(bird_thread_ids[b], NULL));
    }   
  }

  printf("cat play = %ld, dog play = %ld, bird play= %ld\n", cats_played, dogs_played, birds_played);
  exit(0);
}