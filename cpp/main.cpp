#include <cstring>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

namespace {
const std::string PIPE = "|";

enum PipeEnd {
  END_READ, END_WRITE
}; // enum PipeEnd

void onError(const std::string &func) {
  std::cerr << func << " failed: " << strerror(errno) << std::endl;
}

int exec(int begin, int end, const std::vector<std::string> &argv, int *fd) {
  int status = 0;
  pid_t pid = fork();
  switch (pid) {
  case -1: // failure
    onError("fork");
    return EXIT_FAILURE;

  case 0: { // child
    std::vector<char*> args(end - begin + 1, nullptr);
    for (int i = begin; i < end; ++i) {
      args[i - begin] = const_cast<char*>(argv[i].c_str());
    }

    // Redirect input and output
    if (fd[END_READ] >= 0) {
      dup2(fd[END_READ], STDIN_FILENO);
      close(fd[END_READ]);
    }
    if (fd[END_WRITE] >= 0) {
      dup2(fd[END_WRITE], STDOUT_FILENO);
      close(fd[END_WRITE]);
    }

    execvp(args[0], &args[0]);
    _exit(EXIT_FAILURE); // there must be something wrong if reach here
  }

  default: { // parent
    if ((waitpid(pid, &status, 0) < 0) && (errno != EINTR)) {
      status = EXIT_FAILURE;
    }
    break;
  }
  }
  return status;
}
} // namespace

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "No arguments" << std::endl;
    return 0;
  }

  std::istringstream iss(argv[1]);
  std::vector<std::string> args;
  std::string arg;
  while (iss >> arg) {
    // TODO Revert escaped shell arguments
    args.emplace_back(std::move(arg));
  }
  argc = static_cast<int>(args.size());

  int fd[2] = {-1, -1};
  int begin = 0;
  for (;;) {
    int end = begin;
    while ((end < argc) && (PIPE != args[end])) {
      ++end;
    }
    if (end >= argc) {
      break;
    }
    int fd_pipe[2];
    if (pipe(fd_pipe) < 0) {
      onError("pipe");
      return EXIT_FAILURE;
    }
    fd[END_WRITE] = fd_pipe[END_WRITE];
    if (exec(begin, end, args, fd) == EXIT_FAILURE) {
      onError(args[begin]);
      return EXIT_FAILURE;
    }
    if (fd[END_READ] >= 0) {
      close(fd[END_READ]);
    }
    fd[END_READ] = fd_pipe[END_READ];
    close(fd[END_WRITE]);
    fd[END_WRITE] = -1;
    begin = end + 1;
  }
  return exec(begin, argc, args, fd);
}
