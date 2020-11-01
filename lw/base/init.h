#pragma once

namespace lw {

/**
 * Initializes liblw from the command-line arguments.
 *
 * Arguments consumed by liblw are removed from the argument vector and the
 * argument count is updated appropriately. Some built-in flags are handled
 * automatically during initialization, such as printing usage if `--help` is
 * passed.
 *
 * @param argc  The number of arguments given to the program.
 * @param argv  The vector or command line arguments.
 *
 * @return
 *  Returns true if the program should continue to execute after initialization.
 *  May return false if initialization has handled the expected run of the
 *  command, for instance printing usage if `--help` was passed as a flag.
 */
bool init(int* argc, const char** argv);

}
