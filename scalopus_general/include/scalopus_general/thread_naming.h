#include "scalopus_general/internal/thread_naming.h"

// Macro to register the thread name for the thread id. This MUST be called from within the forked thread for it to
// work.
#define TRACE_THREAD_NAME(name)                                                                                        \
  TRACE_THREAD_NAME_ONCE(name, SCALOPUS_MAKE_UNIQUE(scalopus_threadname_id_))                                          \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)
