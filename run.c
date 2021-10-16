/*
   -*- Copyright 2021 muzzletov -*- 
                LOL
*/

#include "run.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <util.h>

int status;
pid_t cPid;
int master;
char *dataPtr = NULL;

int kill_child() {
  kill(cPid, SIGKILL);
  return waitpid(cPid, &status, NULL);
}

void clean_up() {
    free(dataPtr);
    close(master);
}

int state() {
  int pstatus = waitpid(cPid, &status, WNOHANG);

  if (pstatus == -1) {
    clean_up();
  }

  return pstatus;
}

char *read_bytes_for_fd(int);

char *read_bytes() {
  return read_bytes_for_fd(master);
}

char *read_bytes_for_fd(int fd) {
  if(state() == -1) return NULL;

  int nread = read(fd, dataPtr+1, 254);

  if (nread == 0) {
    clean_up();
    return NULL;
  }

  if(nread == -1) {
    *(dataPtr) = 1;
  } else {
    *(dataPtr) = nread+1;
  }

  return dataPtr;
}

int start(char *exec) {
  //ROFLMAO
  char* args[] = {"ls", "-G", "/Users", NULL};

  int status = 0;

  cpid = forkpty(&master, NULL, NULL, NULL);

  if (cPid == -1) {
        perror("forkpty fail");
        exit(EX_OSERR);
  }

  if (cPid == 0) {

    execvp(args[0], args);
    perror("execvp fail");
    close(master);
    exit(EX_OSERR);
  }

  dataPtr = malloc(sizeof(char)*256);

  printf("forkpty success\n");
  fcntl(master, F_SETFL, fcntl(master, F_GETFL) | O_NONBLOCK);

  return 0;
}
