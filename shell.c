#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAXCHARS 256 //max chars per input is 256

char varList[1024][256]; //max number of vars is 1024 and each var has a max char length of 256
int numVars = 3;
char buf[256];
char tokens[1024][MAXCHARS];
int numTokens;
char *delim = " \t\n";

void createTokens(void) {
  /* Create the tokens */
  int i = 0;
  char *token = strtok(buf, delim);
  while (token!=NULL) {
    strcpy(tokens[i], token);
    //printf("%s\n", token); //strtok test
    token = strtok(NULL, delim);
    ++i;
  }
  numTokens = i; //set number of tokens
}

void setVar(void) {
  /* Set variables using setenv() */
  if (strcmp(tokens[0], "CWD")==0) {
    printf("CWD variable can only be changed with cd command\n");
    return;
  }
  setenv(tokens[0], tokens[2], 1);
  int varFound = 0; 
  for (int i = 0; i < numVars; ++i) { //if the var being set is already in the varList, the new value is set, but an extra copy is not added to the list.
    if (strcmp(tokens[0], varList[i])==0)
      varFound = 1;
  }
  if (!varFound) {
    numVars++;
    strcpy(varList[numVars-1], tokens[0]);
  }
}

void unset(void) {
  int varFound = 0; //varList is iterated through until the var to unset is found. Then each element in the array is replaced by the next.
  for (int i = 0; i < numVars; ++i) {
    if (strcmp(tokens[1], varList[i])==0)
      varFound = 1;
    if (varFound) 
      strcpy(varList[i], varList[i+1]);
  }
  unsetenv(tokens[1]);
  numVars--;
}

void listVars(void) {
  for (int i = 0; i < numVars; ++i) {
    printf("%s: %s\n", varList[i], getenv(varList[i]));
  }
}

void changeDirectory(void) {
  if (numTokens==1) {
    printf("must specify directory\n"); 
    return;
  }
  int error;
  error = chdir(tokens[1]);
  if (error==-1)
    printf("Directory not found\n");
}

void chooseFunc(void) {
  if (strcmp(tokens[0], "exit")==0)
    exit(0);
  else if (numTokens==3 && strcmp(tokens[1], "=")==0)
    setVar();
  else if (numTokens==1 && strcmp(tokens[0], "lv")==0)
    listVars();
  else if (numTokens==2 && strcmp(tokens[0], "unset")==0)
    unset();
  else if (strcmp(tokens[0], "#")==0) {/*do nothing for comments*/}
  else if (strcmp(tokens[0], "cd")==0)
    changeDirectory();
  else
    printf("invalid command\n");
}

void loopSetup(void) {
  char cwd[1024]; 
  getcwd(cwd, 1024);
  setenv("CWD", cwd, 1);
  strcat(cwd, " > ");
  setenv("PS", cwd, 1);
}

int main(void) {
  printf("introducing the most magnificant shell there is!\n");
  strcpy(varList[0], "PS");
  strcpy(varList[1], "PATH");
  strcpy(varList[2], "CWD");

  sigset_t block_set; //block SIGINT 
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGINT);
  sigprocmask(SIG_BLOCK, &block_set, NULL);
  
  while (1) {
    loopSetup();
    printf("%s", getenv("PS")); //prompt string printed
    if (fgets(buf, MAXCHARS, stdin) == NULL) {
      printf("\n");
      exit(0);
    }
    else if (buf[0] != ' ' && buf[0] != '\n') { //if input starts with a space or is just a newline character reprint prompt
      createTokens();
      chooseFunc();
    }
  }
}
