//
// adapted from AFL++'s afl_driver.cpp - a glue between AFL++ and LLVMFuzzerTestOneInput harnesses
//

#ifdef __cplusplus
extern "C" {

#endif

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define MAX_FILE (1 * 1024 * 1024L)

/*__attribute__((weak))*/ int LLVMFuzzerTestOneInput(const uint8_t *Data,
                                                     size_t         Size);

// Execute any files provided as parameters.
static int ExecuteFilesOnyByOne(int argc, char **argv,
                                int (*callback)(const uint8_t *data,
                                                size_t         size)) {

  unsigned char *buf = (unsigned char *)malloc(MAX_FILE);

  for (int i = 1; i < argc; i++) {

    int fd = 0;

    if (strcmp(argv[i], "-") != 0) { fd = open(argv[i], O_RDONLY); }

    if (fd == -1) { continue; }

    ssize_t length = syscall(SYS_read, fd, buf, MAX_FILE);

    if (length > 0) {
      printf("Reading %zu bytes from %s\n", length, argv[i]);
      callback(buf, length);
      printf("Execution successful.\n");

    }

    if (fd > 0) { close(fd); }

  }

  free(buf);
  return 0;
}

__attribute__((weak)) int LLVMFuzzerRunDriver(
    int *argcp, char ***argvp,
    int (*callback)(const uint8_t *data, size_t size)) {

  int    argc = *argcp;
  char **argv = *argvp;

  if (argc > 1 && argv[1][0] != '-') {
    return ExecuteFilesOnyByOne(argc, argv, callback);
  }
  
  return 0;
}

__attribute__((weak)) int main(int argc, char **argv) {

  if (argc < 2 || strncmp(argv[1], "-h", 2) == 0 ||
      strcmp(argv[1], "--help") == 0) {

    printf(
        "============================== INFO ================================\n"
        "To run the target function on individual input(s) execute:\n"
        "  %s INPUT_FILE1 [INPUT_FILE2 ... ]\n"
        "===================================================================\n",
        argv[0]);
    if (argc == 2 &&
        (strncmp(argv[1], "-h", 2) == 0 || strcmp(argv[1], "--help") == 0)) {

      exit(0);

    }

  }

  return LLVMFuzzerRunDriver(&argc, &argv, LLVMFuzzerTestOneInput);
}

#ifdef __cplusplus

}

#endif

