#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

int main(){
  int rc = exec_local_cmd_loop();
  printf("cmd loop returned %d\n", rc);
}
