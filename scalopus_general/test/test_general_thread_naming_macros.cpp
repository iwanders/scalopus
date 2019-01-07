#include "scalopus_general/thread_naming.h"

int main(int /* argc */, char** /* argv */)
{
  // Execute all public macros.
  // Mostly to ensure they don't collide with each other if used multiple times
  // or create shadowed variables or something.
  TRACE_THREAD_NAME("foo");

  // Check if the macro's don't expand to shadowed variables.
  {
    TRACE_THREAD_NAME("foo");
  }
  return 0;
}