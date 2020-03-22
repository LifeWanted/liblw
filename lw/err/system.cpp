#include "lw/err/system.h"

#include <errno.h>
#include <experimental/source_location>
#include <string.h>

#include "lw/err/canonical.h"

namespace lw {
using ::std::experimental::source_location;

void check_system_error(const source_location& loc) {
  // TODO: Add support for other OS errors.
  check_system_error(errno, loc);
}

void check_system_error(int err_code, const source_location& loc) {
  switch (err_code) {
    case 0: { return; }

    case EAGAIN:
    // case EWOULDBLOCK: // Same as EAGAIN
    case ECONNABORTED:
    case EDEADLK:
    // case EDEADLOCK: // Same as EDEADLK
    case ENETRESET:
    case ERESTART: {
      throw Aborted(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case EALREADY:
    case EEXIST:
    case EINPROGRESS: {
      throw AlreadyExists(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case ECANCELED:
    case ECONNRESET:
    case EINTR:
    case ENOLINK: {
      throw Cancelled(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case ETIME:
    case ETIMEDOUT: {
      throw DeadlineExceeded(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case EBADFD:
    case ECHILD:
    case EISCONN:
    case EISDIR:
    case EISNAM:
    case EKEYEXPIRED:
    case ENODATA:
    case ENONET:
    case ENOPKG:
    case ENOTBLK:
    case ENOTCONN:
    case ENOTEMPTY:
    case ENOTTY:
    case ENOTUNIQ:
    case EPIPE:
    case ESHUTDOWN:
    case EUNATCH: {
      throw FailedPrecondition(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case EBADE:
    case ECOMM:
    case EHWPOISON:
    case EIDRM:
    case EIO:
    case EL2HLT:
    case EL2NSYNC:
    case EL3HLT:
    case EL3RST:
    case ELIBACC:
    case ELIBBAD:
    case ELIBSCN:
    case ELIBEXEC:
    case ENOTRECOVERABLE:
    case ERFKILL: {
      throw Internal(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case E2BIG:
    case EBADF:
    case EBADMSG:
    case EBADR:
    case EBADRQC:
    case EBADSLT:
    case EDESTADDRREQ:
    case EFAULT:
    case EILSEQ:
    case EINVAL:
    case ENAMETOOLONG:
    case ENOEXEC:
    case ENOTDIR:
    case ENOSTR:
    case ENOTSOCK:
    case EPROTOTYPE:
    case ESPIPE:
    case ESTALE: {
      throw InvalidArgument(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case ENODEV:
    case ENOENT:
    case ENOMSG:
    case ENXIO:
    case ESRCH: {
      throw NotFound(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case ECHRNG:
    case EDOM:
    // case ELNRANGE: // Not defined on Linux?
    case ERANGE: {
      throw OutOfRange(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case EACCES:
    case ECONNREFUSED:
    case EKEYREJECTED:
    case EKEYREVOKED:
    case EPERM:
    case EROFS: {
      throw PermissionDenied(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case EDQUOT:
    case EFBIG:
    case ELIBMAX:
    case ELOOP:
    case EMFILE:
    case EMLINK:
    case EMSGSIZE:
    case ENFILE:
    case ENOBUFS:
    case ENOKEY:
    case ENOLCK:
    case ENOMEM:
    case ENOSPC:
    case ENOSR:
    case EOVERFLOW:
    case ETOOMANYREFS:
    case EUSERS:
    case EXFULL: {
      throw ResourceExhausted(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case EBUSY:
    case EHOSTDOWN:
    case EHOSTUNREACH:
    case ENETDOWN:
    case ENETUNREACH:
    case ENOPROTOOPT:
    case ETXTBSY: {
      throw Unavailable(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    case EAFNOSUPPORT:
    case ENOSYS:
    case ENOTSUP:
    // case EOPNOTSUPP: // Duplicate of ENOTSUP in Linux.
    case EPFNOSUPPORT:
    case EPROTONOSUPPORT:
    case ESOCKTNOSUPPORT: {
      throw Unimplemented(loc)
        << "System error " << err_code << ": " << ::strerror(err_code);
    }

    default: {
      throw Internal()
        << "Unknown system error " << err_code << " with message: "
        << ::strerror(err_code);
    }
  }

  // TODO: Figure out what these error codes mean and how they should map.
  //
  // EMEDIUMTYPE     Wrong medium type.
  // EMULTIHOP       Multihop attempted (POSIX.1-2001).
  // ENOANO          No anode.
  // ENOMEDIUM       No medium found.
  // EOWNERDEAD      Owner died (POSIX.1-2008).
  // EPROTO          Protocol error (POSIX.1-2001).
  // EREMCHG         Remote address changed.
  // EREMOTE         Object is remote.
  // EREMOTEIO       Remote I/O error.
  // ESTRPIPE        Streams pipe error.
  // EUCLEAN         Structure needs cleaning.
  // EXDEV           Improper link (POSIX.1-2001).
}

}
