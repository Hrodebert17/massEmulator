#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
int main() {
  int pid = fork();
  if (pid == 0) {
    char *args[] = {(char *)"fzf", NULL};
    freopen("/tmp/pipeoutthem", "w", stdout);
    execvp("fzf", args);
  } else {
    waitpid(pid, NULL, 0);
    std::ifstream ifFile("/tmp/pipeoutthem", std::ios::ate);
    if (ifFile.is_open()) {
      int size = ifFile.tellg();
      char *buffer = new char[size];
      ifFile.seekg(0, std::ios::beg);
      ifFile.read(buffer, size);
      ifFile.close();
      std::string fileName;
      for (int i = 0; i < size; i++) {
        if (buffer[i] != '\n') {
          fileName += buffer[i];
        }
      }
      delete[] buffer;
    }
  }
  return 0;
}
