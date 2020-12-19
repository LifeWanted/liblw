#include "lw/http/https.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>

#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/err/canonical.h"
#include "lw/err/system.h"
#include "lw/http/internal/tls_state.h"
#include "lw/io/co/co.h"
#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "third_party/openssl/crypto.h"
#include "third_party/openssl/err.h"
#include "third_party/openssl/ssl.h"

namespace lw {

HttpsRouter::HttpsRouter(const Options& options):
  _tls{http::internal::TLSState::from_options(options)}
{}

co::Task<void> HttpsRouter::run(std::unique_ptr<io::CoStream> conn) {

}

}
