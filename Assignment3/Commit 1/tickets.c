#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  
  
  if(argc < 3){
    printf(2, "usage: need number of tickets and program name\n");
    exit();
  }
  
  int tickets = atoi(argv[1]);
  char* program = argv[2]; 
  settickets(tickets);
  int argc2 = argc - 1;

  


  char** argv2 = malloc( argc2 * sizeof(char*));
  int i;
  for(i = 0; i<argc2-1; i++ ){
      char * original = argv[i+2];
      char* copy = malloc(sizeof(char*));
      strcpy(copy, original);
      argv2[i] = copy; 
  }
  argv2[argc2-1] = 0; 
  exec(program, argv2);
  
   
  exit();
}
