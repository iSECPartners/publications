#include <stddef.h>
#include <unistd.h>

/* OS() - underlying operating system; only to be used for mandated low-level services like
   virtual memory, not to choose a GUI toolkit */
#define OS(WTF_FEATURE) (defined WTF_OS_##WTF_FEATURE  && WTF_OS_##WTF_FEATURE)

#define WTF_OS_LINUX 1
#define WTF_OS_POSIX 1
