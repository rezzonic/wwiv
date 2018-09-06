/**************************************************************************/
/*                                                                        */
/*                                WWIV Version 5                          */
/*             Copyright (C)1998-2016, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#include "bbs/exec.h"
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <pty.h>

#include "bbs/bbs.h"
#include "bbs/remote_io.h"

#include "core/log.h"

static int UnixSpawn(const std::string& cmd, char* environ[], int flags) {
  if (cmd.empty()) {
    return 1;
  }
  const int sock = a()->remoteIO()->GetDoorHandle();
  int pid = -1;
  int master_fd = -1;
  if (flags & EFLAG_STDIO) {
    pid = forkpty(&master_fd, nullptr, nullptr, nullptr);
  } else {
    pid = fork();
  }
  if (pid == -1) {
    auto e = errno;
    LOG(ERROR) << "Fork Failed: errno: '" << e << "'";
    // Fork failed.
    return -1;
  }
  if (pid == 0) {
    // Were' in the child.
    const char* argv[4] = { "/bin/sh", "-c", cmd.c_str(), 0 };
    execv("/bin/sh", const_cast<char ** const>(argv));
    exit(127);
  }

  // In the parent now.
  // remove the echo
  struct termios tios;
  tcgetattr(master_fd, &tios);
  tios.c_lflag &= ~(ECHO | ECHONL);
  tcsetattr(master_fd, TCSAFLUSH, &tios);

  for (;;) {
    fd_set rfd;
    FD_ZERO(&rfd);

    FD_SET(master_fd, &rfd);
    FD_SET(sock, &rfd);

    struct timeval tv = {1, 0};
    auto ret = select(std::max<int>(sock, master_fd)+1,
		      &rfd, nullptr, nullptr, &tv);
    if (ret < 0) {
      break;
    }
    int status_code = 0;
    pid_t wp = waitpid(pid, &status_code, WNOHANG);
    if (wp == -1 || wp > 0) {
      // -1 means error and >0 is the pid
      break;
    }
    if (FD_ISSET(sock, &rfd)) {
      char input;
      read(sock, &input, 1);
      write(master_fd, &input, 1);
      VLOG(3) << "read from socket, write to term: '" << input << "'";
    }
    if (FD_ISSET(master_fd, &rfd)) {
      char input;
      read(master_fd, &input, 1);
      write(sock, &input, 1);
    }
  }
  
  // We probably won't get here, but just in case we do...
  for (;;) {
    LOG(INFO) << "about to call waitpid at the end";
    // In the parent, wait for the child to terminate.
    int status_code = 1;
    if (waitpid(pid, &status_code, 0) == -1) {
      if (errno != EINTR) {
        return -1;
      }
    } else {
      return status_code;
    }
  }

  // Should never happen.
  return -1;
}

int exec_cmdline(const std::string cmdline, int flags) {
  if (flags & EFLAG_FOSSIL) {
    LOG(ERROR) << "EFLAG_FOSSIL is not supported on UNIX";
  }
  if (flags & EFLAG_COMIO) {
    LOG(ERROR) << "EFLAG_COMIO is not supported on UNIX";
  }

  if (a()->context().ok_modem_stuff()) {
    a()->remoteIO()->close(true);
  }

  int i = UnixSpawn(cmdline, nullptr, flags);

  // reengage comm stuff
  if (a()->context().ok_modem_stuff()) {
    a()->remoteIO()->open();
  }
  return i;
}


