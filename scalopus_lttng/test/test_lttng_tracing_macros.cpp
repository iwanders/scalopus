#include "scalopus_lttng/scope_tracing.h"

int main(int /* argc */, char** /* argv */)
{
  // Execute all public trace point macros.
  // Mostly to ensure they don't collide with each other if used multiple times
  // or create shadowed variables or something.
  TRACE_PRETTY_FUNCTION();
  TRACE_PRETTY_FUNCTION();

  TRACE_TRACKED_RAII("main");
  TRACE_TRACKED_RAII("main");

  TRACE_SCOPE_START("zz");
  TRACE_SCOPE_START("zz");
  TRACE_SCOPE_END("zz");
  TRACE_SCOPE_END("zz");

  // Check if the macro's don't expand to shadowed variables.
  {
    TRACE_PRETTY_FUNCTION();
    TRACE_PRETTY_FUNCTION();

    TRACE_TRACKED_RAII("main");
    TRACE_TRACKED_RAII("main");

    TRACE_SCOPE_START("zz");
    TRACE_SCOPE_START("zz");
    TRACE_SCOPE_END("zz");
    TRACE_SCOPE_END("zz");
  }
  return 0;
}