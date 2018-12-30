#include <scalopus_general/internal/helper_macros.h>

// Macro that ensures that tracking the map between name and id is only performed once, this is achieved by using a
// static boolean.
#define TRACE_TRACKED_MAPPING_REGISTER_ONCE(name, id, have_done_setup_varname)                                         \
  static bool have_done_setup_varname = false;                                                                         \
  if (!have_done_setup_varname)                                                                                        \
  {                                                                                                                    \
    have_done_setup_varname = true;                                                                                    \
    scalopus::ScopeTraceTracker::getInstance().insert(id, name);                                                       \
  }

// Macro to create a tracked RAII tracepoint. The tracepoint itself will store only the ID, but the singleton trace
// tracker stores the ID -> name relation provided.
#define TRACE_TRACKED_RAII_ID(name, id)                                                                                \
  TRACE_TRACKED_MAPPING_REGISTER_ONCE(name, id, SCALOPUS_MAKE_UNIQUE(scalopus_trace_id_))                              \
  scalopus::TraceRAII SCALOPUS_MAKE_UNIQUE(scalopus_trace_id_)(id);                                                    \
  do                                                                                                                   \
  {                                                                                                                    \
  } while (0)

#define TRACE_SCOPE_START_NAMED_ID(name, id)                                                                           \
  TRACE_TRACKED_MAPPING_REGISTER_ONCE(name, id, SCALOPUS_MAKE_UNIQUE(scalopus_trace_id_))                              \
  scalopus::scope_entry(id);

#define TRACE_SCOPE_END_NAMED_ID(name, id) scalopus::scope_exit(id);

// comment for stylecheck reasons
