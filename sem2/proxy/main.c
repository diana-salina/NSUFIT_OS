#include <stdio.h>
#include <unistd.h>

#include "proxy.h"
#include "logger.h"

int main(void) {
  Logger* logger = createLogger("proxy.log", LOG_DEBUG);
  if (!logger) {
    return 0;
  }
  //int pid = getpid();
  //printf("PID: %d\n", pid);

  startProxy(PORT, logger);
  return 0;
}
