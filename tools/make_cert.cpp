
#include <fstream>
#include <string>
#include <string_view>

#include "botan/auto_rng.h"
#include "botan/der_enc.h"
#include "botan/pem.h"
#include "botan/rsa.h"
#include "botan/x509cert.h"
#include "botan/x509self.h"
#include "lw/base/base.h"
#include "lw/flags/flags.h"
#include "lw/format/blobs.h"
#include "lw/log/log.h"
#include "lw/memory/buffer.h"

LW_FLAG(
  std::string, out_path, "",
  "Path to save certificate to."
);

LW_FLAG(
  std::size_t, bits, 2048,
  "Size of the private key to generate."
);

namespace {

using namespace lw;

}

int main(int argc, const char* argv[]) {
  if (!init(&argc, argv)) return 0;

  if (flags::out_path.value().empty()) {
    log(ERROR) << "Output path name is required!";
    return -1;
  }

  Botan::AutoSeeded_RNG rng;
  Botan::RSA_PrivateKey key{rng, flags::bits};
  Botan::X509_Cert_Options options;

  Botan::X509_Certificate cert =
    Botan::X509::create_self_signed_cert(options, key, "SHA-512", rng);

  Botan::DER_Encoder encoder;
  cert.encode_into(encoder);

  std::ofstream out{flags::out_path.value()};
  out << Botan::PEM_Code::encode(encoder.get_contents(), "CERTIFICATE");
  out.close();

  return 0;
}
