#include "lw/io/co/co.h"

#include <algorithm>
#include <cstdint>

#include "lw/flags/flags.h"

LW_FLAG(
  std::size_t, initial_read_buffer_size, 1024*1024,
  "Ininitial size, in bytes, for read buffers."
);
LW_FLAG(
  std::size_t, maximum_read_buffer_size, 1024*1024*1024,
  "Maximum size, in bytes, for read buffers to grow to."
);
LW_FLAG(
  std::size_t, read_block_size, 1024 * 10,
  "Number of bytes to request at a time from read sources."
);

namespace lw::io::internal {
namespace {

void realloc_buffers(
  Buffer& buffer,
  Buffer& read,
  Buffer& write,
  std::size_t desired_total_size
) {
  if (desired_total_size > flags::maximum_read_buffer_size) {
    // TODO: Test integer overflow case for additional space.
    throw ResourceExhausted()
      << "CoReader buffer limited to --maximum-read-buffer-size ("
      << flags::maximum_read_buffer_size.value() << " bytes). "
      << (desired_total_size) << " bytes requested.";
  }

  // Calculate the new buffer size.
  std::size_t new_size = buffer.size() * 2;
  while (new_size < desired_total_size) new_size *= 2;
  new_size = std::min(new_size, flags::maximum_read_buffer_size.value());

  // Allocate a new buffer and copy the data over.
  Buffer new_buff{new_size};
  new_buff.copy(read.data(), read.size());
  read = Buffer{new_buff.data(), read.size()};
  write = new_buff.trim_prefix(read.size());
  buffer = std::move(new_buff);
}

}

void adjust_buffers(
  Buffer& buffer,
  Buffer& read,
  Buffer& write,
  std::size_t desired_write_size
) {
  // Calculate required additional space.
  if (desired_write_size <= write.size()) return;
  std::int64_t additional_space = desired_write_size - write.size();

  // If there isn't room in the front of the buffer, allocate a new buffer.
  if (read.data() - buffer.data() < additional_space) {
    realloc_buffers(buffer, read, write, buffer.size() + additional_space);
    return;
  }

  // There is room in the buffer, just need to move our read window up.
  buffer.copy(read.data(), read.size());
  read = Buffer{buffer.data(), read.size()};
  write = buffer.trim_prefix(read.size());
}

}
