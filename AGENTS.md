# AGENTS.md

## Repository expectations

* Treat this repository as a portable C++ logging library. Preserve existing public API/ABI conventions unless the task explicitly asks for an API change.
* Keep new third-party integrations optional. Do not add mandatory dependencies to the core log4cplus library unless explicitly requested.
* Prefer the existing log4cplus extension patterns:

  * New output integrations should be implemented as `Appender` subclasses.
  * Appenders configured from properties should provide a `helpers::Properties` constructor.
  * Register built-in appenders in the factory registry only when the feature is enabled.

* Preserve existing style:

  * Use the existing copyright/license header style.
  * Use `log4cplus::tstring`, `LOG4CPLUS_TEXT`, and existing conversion helpers where appropriate.
  * Use existing `helpers::Properties` parsing patterns.
  * Use `helpers::getLogLog()` for internal diagnostic errors/warnings.

* Do not assume Unicode is disabled. Code must build with both narrow and Unicode/tstring configurations where practical.
* Do not assume single-threaded builds. Respect `LOG4CPLUS_SINGLE_THREADED` conditionals.
* Update all relevant build systems when adding installed public headers or compiled sources:

  * CMake files
  * Autotools/Automake inputs/templates used by this repo
  * Any generated-file workflow documented by the repository

* Add or update tests/examples when behavior changes or a new appender is added.
* Before finishing, report:

  * What files changed
  * What build/test commands were run
  * Any commands that could not be run and why

## Build/test guidance

* Prefer an out-of-tree CMake build for quick validation.
* If touching Autotools inputs, inspect the repository’s documented regeneration script before modifying generated files.
* Do not vendor external dependencies.
