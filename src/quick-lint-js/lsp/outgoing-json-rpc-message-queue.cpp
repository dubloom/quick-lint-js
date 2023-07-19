// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#if defined(__EMSCRIPTEN__)
// No LSP on the web.
#else

#include <quick-lint-js/container/byte-buffer.h>
#include <quick-lint-js/lsp/outgoing-json-rpc-message-queue.h>
#include <utility>
#include <vector>

namespace quick_lint_js {
LSP_Endpoint_Remote::~LSP_Endpoint_Remote() = default;

Byte_Buffer& Outgoing_JSON_RPC_Message_Queue::new_message() {
  return this->messages_.emplace_back();
}

void Outgoing_JSON_RPC_Message_Queue::send(LSP_Endpoint_Remote& remote) {
  for (Byte_Buffer& notification_json : this->messages_) {
    remote.send_message(std::move(notification_json));
  }
  this->messages_.clear();
}
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
