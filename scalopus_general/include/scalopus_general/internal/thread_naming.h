#include "scalopus_general/internal/helper_macros.h"
#include "scalopus_general/internal/thread_name_tracker.h"

// Macro to register the thread name once.
#define TRACE_THREAD_NAME_ONCE(name, have_done_once_varname)                                                           \
  static thread_local bool have_done_setup_varname = false;                                                            \
  if (!have_done_setup_varname)                                                                                        \
  {                                                                                                                    \
    have_done_setup_varname = true;                                                                                    \
    scalopus::ThreadNameTracker::getInstance().setCurrentName(name);                                                   \
  }
