/*
   -*- Copyright 2021 muzzletov -*- 
                LOL
*/

#include "run.h"

#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <util.h>

int status;
pid_t cpid;
int master;
char *dataPtr = NULL;


int kill_child() {
  return kill(cpid, SIGKILL);
  //return waitpid(cpid, &status, NULL);
}

int state() {
  int pstatus = waitpid(-1, &status, WNOHANG);

  if (pstatus == -1) {
    free(dataPtr);
    close(master);
  }

  return pstatus;
}

char *read_bytes_by_fd(int);

char *read_bytes() {
  return read_bytes_by_fd(master);
}

char *read_bytes_by_fd(int fd) {
  if(state() == -1) return NULL;

  int nread = read(fd, dataPtr+1, 254);

  if (nread == 0) {
    close(master);
    return NULL;
  }
  
  if(nread == -1) {
    *(dataPtr) = 1;
  } else {
    *(dataPtr+nread+1) = '\0';
    *(dataPtr) = nread+1;
  }
  
  return dataPtr;
}

int start(char** args) {

  int status = 0;

  cpid = forkpty(&master, NULL, NULL, NULL);

  if (cpid == -1) {
        perror("forkpty failed");
        exit(EX_OSERR);
  }

  if (cpid == 0) {

    execvp(args[0], args);
    perror("exec failed");

    exit(EX_OSERR);
  } 
  
  dataPtr = malloc(sizeof(char)*256);
  
  printf("forkpty successful\n");
  fcntl(master, F_SETFL, fcntl(master, F_GETFL) | O_NONBLOCK);

  return 0;
}
