
/**
 * Internal macros, these need to be available from outside, but should not be used directly.
 */

// Create a unique ID based on the crc32 of the filename and the line number.
#define SCALOPUS_TRACKED_TRACE_ID_CREATOR() (CRC32_STR(__FILE__) + __LINE__)

// Helper Macro's to create a unique name for the RAII tracepoints.
#define SCALOPUS_CONCATENATE_DETAIL(x, y) x##y
#define SCALOPUS_CONCATENATE(x, y) SCALOPUS_CONCATENATE_DETAIL(x, y)
#define SCALOPUS_MAKE_UNIQUE(x) SCALOPUS_CONCATENATE(x, __COUNTER__)

// Create a unique ID based on the crc32 of the filename and the provided string.
#define SCALOPUS_TRACKED_TRACE_ID_STRING(s) (CRC32_STR(__FILE__) + CRC32_STR(s))
