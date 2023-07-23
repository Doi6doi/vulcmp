#ifndef VULCOMPH
#define VULCOMPH

#include <stdbool.h>
#include <stdint.h>

typedef const char * VcpStr;
typedef enum { VCP_VALIDATION=1, VCP_ATOMIC_FLOAT=2 } VcpFlags;
typedef struct Vcp__Vulcomp * VcpVulcomp;
typedef struct Vcp__Storage * VcpStorage;
typedef struct Vcp__Task * VcpTask;

typedef struct Vcp_Part {
   uint32_t baseX, baseY, baseZ, countX, countY, countZ;
} VcpPart;

#define VCP_NOPHYSICAL -10001
#define VCP_NOFAMILY -10002
#define VCP_NOMEMORY -10003
#define VCP_NOFILE -10004
#define VCP_RUNNING -10005
#define VCP_NOGROUP -10006
#define VCP_NOSTORAGE -10007

/// vulkan handle score function
typedef int32_t (* VcpScorer )( void * );
/// default physical device score
int vcp_physical_score( void * p );
/// default queue family score
int vcp_family_score( void * f );

/// error code
int vcp_error();
/// check result and fail if not ok
void vcp_check_fail();
/// initialize vulcomp system
VcpVulcomp vcp_init( VcpStr appName, uint32_t flags );
/// terminate vulcomp system
void vcp_done( VcpVulcomp );
/// choose best device
void vcp_select_physical( VcpVulcomp, VcpScorer );
/// choose queue
void vcp_select_queue( VcpVulcomp, VcpScorer );
/// create storage
VcpStorage vcp_storage_create( VcpVulcomp v, uint64_t size );
/// get storage memory address
void * vcp_storage_address( VcpStorage s );
/// dispose storage
void vcp_storage_free( VcpStorage s );
/// create task
VcpTask vcp_task_create( VcpVulcomp v, void *data, uint64_t size, VcpStr entry,
   uint32_t nstorage );
/// create task by file
VcpTask vcp_task_create_file( VcpVulcomp v, VcpStr filename, VcpStr entry,
   uint32_t nstorage );
/// setup task
void vcp_task_setup( VcpTask t, VcpStorage * storages,
   uint32_t gx, uint32_t gy, uint32_t gz );
/// setup task repeat
void vcp_task_parts( VcpTask t, uint32_t nparts, VcpPart * parts, uint32_t nrepeat );
/// start task
void vcp_task_start( VcpTask t );
/// wait task to finish
bool vcp_task_wait( VcpTask t, uint32_t timeoutMsec );
/// dispose task
void vcp_task_free( VcpTask t );

#endif // VULCOMPH
