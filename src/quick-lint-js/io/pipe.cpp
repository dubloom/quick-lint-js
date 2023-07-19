// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#if defined(__EMSCRIPTEN__)
// No pipes on the web.
#else

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <quick-lint-js/io/pipe.h>
#include <quick-lint-js/port/have.h>
#include <quick-lint-js/port/windows-error.h>

#if QLJS_HAVE_PIPE
#include <fcntl.h>
#include <unistd.h>
#endif

#if QLJS_HAVE_WINDOWS_H
#include <quick-lint-js/port/windows.h>
#endif

namespace quick_lint_js {
#if QLJS_HAVE_PIPE
Pipe_FDs make_pipe() {
  int fds[2];
  int rc = ::pipe(fds);
  if (rc == -1) {
    std::fprintf(stderr, "error: failed to create pipe: %s\n",
                 std::strerror(errno));
    std::abort();
  }
  for (int fd : fds) {
    rc = ::fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (rc == -1) {
      std::fprintf(stderr, "warning: failed to make pipe CLOEXEC: %s\n",
                   std::strerror(errno));
    }
  }
  return Pipe_FDs{
      .reader = POSIX_FD_File(fds[0]),
      .writer = POSIX_FD_File(fds[1]),
  };
}
#elif defined(_WIN32)
Pipe_FDs make_pipe() {
  HANDLE readPipe;
  HANDLE writePipe;
  ::SECURITY_ATTRIBUTES attributes = {};
  attributes.nLength = sizeof(attributes);
  attributes.bInheritHandle = true;
  if (!::CreatePipe(&readPipe, &writePipe, /*lpPipeAttributes=*/&attributes,
                    /*nSize=*/0)) {
    std::fprintf(stderr, "error: failed to create pipe: %s\n",
                 windows_last_error_message().c_str());
    std::abort();
  }
  return Pipe_FDs{
      .reader = Windows_Handle_File(readPipe),
      .writer = Windows_Handle_File(writePipe),
  };
}
#endif
}

#endif

// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew "strager" Glazar
//
// This file is part of quick-lint-js.
//
// quick-lint-js is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// quick-lint-js is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with quick-lint-js.  If not, see <https://www.gnu.org/licenses/>.
