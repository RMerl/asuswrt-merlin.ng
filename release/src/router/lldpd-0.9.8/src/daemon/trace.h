#ifdef ENABLE_DTRACE
# include "probes.h"
# define TRACE(probe) probe
# define TRACE_ENABLED(probe) probe ## _ENABLED()
#else
# define TRACE(probe)
# define TRACE_ENABLED(probe) (0)
#endif
