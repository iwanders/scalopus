
/**
 * Private macros, these need to be available from outside, but should not be used directly.
 */

// Create a unique ID based on the crc32 of the filename and the line number.
#define SCALOPUS_TRACKED_TRACE_ID_CREATOR() (CRC32_STR(__FILE__) + __LINE__)

// Helper Macro's to create a unique name for the RAII tracepoints.
#define SCALOPUS_CONCATENATE_DETAIL(x, y) x##y
#define SCALOPUS_CONCATENATE(x, y) SCALOPUS_CONCATENATE_DETAIL(x, y)
#define SCALOPUS_MAKE_UNIQUE(x) SCALOPUS_CONCATENATE(x, __COUNTER__)

// Create a unique ID based on the crc32 of the filename and the provided string.
#define SCALOPUS_TRACKED_TRACE_ID_STRING(s) (CRC32_STR(__FILE__) + CRC32_STR(s))

// Macro that ensures that tracking the map between name and id is only performed once, this is achieved by using a
// static boolean.
#define TRACE_TRACKED_MAPPING_REGISTER_ONCE(name, id, have_done_setup_varname)                                         \
  static bool have_done_setup_varname = false;                                                                         \
  if (!have_done_setup_varname)                                                                                        \
  {                                                                                                                    \
    have_done_setup_varname = true;                                                                                    \
    scalopus::TraceTracker::getInstance().trackEntryExitName(id, name);                                                \
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
