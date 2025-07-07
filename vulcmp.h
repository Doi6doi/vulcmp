#ifndef VULCOMPH
#define VULCOMPH

/**
# vulcmp.h

[Vulcmp](index) is a *C* and *C++* library which
help you to make GPU computing easily.

The `vulcmp.h` header is for the *C* part.
It contains the basic types for computing
and the correspondent functions.

If compiled with a C++ compiler, all the declarations are in
the `vcpc` namespace
*/

/** ## Contents
\toc */

/** ## Details */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
}
namespace vcpc {
#endif

/// Alias for C string
typedef const char * VcpStr;

/// Opaque type for a vulcmp subsystem
typedef struct Vcp_Vulcomp * VcpVulcomp;
/// Opaque type for a GPU-usable memory
typedef struct Vcp_Storage * VcpStorage;
/// Opaque type for a GPU task (subprogram)
typedef struct Vcp_Task * VcpTask;

/// Flags for vulcomp initialization
typedef enum {
   /// Use validation layer. It will show messages about Vulkan errors
   VCP_VALIDATION=1,
   /// Needed to use atomic float operations in shaders
   VCP_ATOMIC_FLOAT=2,
   /// Needed to use uint8 and int8 types in shaders
   VCP_BIT8=4
} VcpFlags;

/// Configuration datas for one run in a multi-run task.
typedef struct Vcp_Part {
   /// Group count for X coordinate
   uint32_t countX;
   /// Group count for Y coordinate
   uint32_t countY;
   /// Group count for Z coordinate
   uint32_t countZ;
   /// Constant values for part
   void * constants;
} * VcpPart;

/** Scorer function for vulcmp
to choose physical device or queue family.
The item with the largest value will be chosen.
Negative score items will not be chosen.
\param x The item being scored
\return Score value */
typedef int32_t (* VcpScorer )( void * x);

/// Collection of predefined scorers
typedef struct Vcp_Scorers {
   /// Default physical scorer.
   /// Chooses a dedicated GPU
   VcpScorer physical_default;
   /// Scorer which chooses a CPU
   VcpScorer physical_cpu;
   /// Default family scorer.
   /// Chooses a usable queue family
   VcpScorer family_default;
} * VcpScorers;

/// Vulcmp error values
typedef enum {
   /// Successful execution
   VCP_SUCCESS    = 0,
   /// Timeout during [#vcp_task_wait]
   VCP_TIMEOUT    = 2,
   /// Not enough host memory
   VCP_HOSTMEM    = -1,
   /// No physical device found
   VCP_NOPHYSICAL = -10001,
   /// No queue family found
   VCP_NOFAMILY   = -10002,
   /// No GPU memory found
   VCP_NOMEMORY   = -10003,
   /// Cannot open file
   VCP_NOFILE     = -10004,
   /// Task already running
   VCP_RUNNING    = -10005,
   /// Empty group size given
   VCP_NOGROUP    = -10006,
   /// Empty storage handle given
   VCP_NOSTORAGE  = -10007,
   /// Empty task handle given
   VCP_NOTASK     = -10008,
   /// Storage size mismatch
   VCP_ADDRESS    = -10009
}  VcpResult;

/** Error code of last vulcmp function call
\return Error code ([#VcpResult] or [VkResult](https://registry.khronos.org/vulkan/specs/latest/man/html/VkResult.html))*/
int vcp_error();

/// Check [#vcp_error] and halt program if not ok
void vcp_check_fail();

/** Initialize vulcomp system
Can be called more times if more GPU-s are used.
\param appName Name of the application (forwarded to vulkan)
\param flags Or-ed [#VcpFlags] values
\return Vulcmp handle */
VcpVulcomp vcp_init( VcpStr appName, uint32_t flags );

/** Get Vulcmp creation flags
\param v Vulcmp handle
\return Flags used for initialization */
uint32_t vcp_flags( VcpVulcomp v );

/** Terminate vulcomp system
\param v Vulcmp handle */
void vcp_done( VcpVulcomp v );

/** Set scorer for choosing physical device.
Must be called before any task or storage creation.
\param v Vulcmp handle
\param s Physical device scorer. Called with [VkPhysicalDevice](https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDevice.html) */
void vcp_select_physical( VcpVulcomp v, VcpScorer s );

/** Set scorer for choosing queue family.
Must be called before any task or storage creation.
\param v Vulcmp handle
\param s Queue family scorer. Called with [VkQueueFamilyProperties](https://registry.khronos.org/vulkan/specs/latest/man/html/VkQueueFamilyProperties.html) */
void vcp_select_family( VcpVulcomp v, VcpScorer s );

/** Create storage
Returns NULL on error
\param v Vulcmp handle
\param size Memory size in bytes
\return Storage handle or NULL on error */
VcpStorage vcp_storage_create( VcpVulcomp v, uint64_t size );

/** Get CPU-address of storage
\param s Storage handle
\return Memory address */
void * vcp_storage_address( VcpStorage s );

/** Get storage size
\param s Storage handle
\return Memory size in bytes */
uint64_t vcp_storage_size( VcpStorage s );

/** Dispose storage
Usually not needed because [#vcp_done] frees used storages.
\param s Storage handle */
void vcp_storage_free( VcpStorage s );

/** GPU-copy of a storage part
Usually faster than memcpy because data is not synced with CPU
\param src Source storage handle
\param dst Destination storage handle
\param si Index of first copied byte in `src`
\param di Index of first copied byte in `dst`
\param count Number of bytes to copy
\return `true` on success */
bool vcp_storage_copy( VcpStorage src, VcpStorage dst, uint32_t si, uint32_t di,
   uint32_t count );

/** Create a GPU task
\param v Vulcmp handle
\param data Address of [SPIR-V](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html) compute module
\param size Size of `data` in bytes
\param entry Name of executable function in `data`
\param nstorage Number of storages used by task
\param constsize Size of constant data in bytes
\return Task handle or NULL on error */
VcpTask vcp_task_create( VcpVulcomp v, void *data, uint64_t size, VcpStr entry,
   uint32_t nstorage, uint32_t constsize );

/** Create a GPU task by file
\param v Vulcmp handle
\param filename Name of file containing [SPIR-V](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html) compute module
\param entry Name of executable function in `data`
\param nstorage Number of storages used by task
\param constsize Size of constant data in bytes
\return Task handle or NULL on error */
VcpTask vcp_task_create_file( VcpVulcomp v, VcpStr filename, VcpStr entry,
   uint32_t nstorage, uint32_t constsize );

/** Setup task before first run
\param t Task handle
\param storages Handles of storages used by task
\param gx Number of groups on X coordinate
\param gy Number of groups on Y coordinate
\param gz Number of groups on Z coordinate
\param constants Constant values for the task */
void vcp_task_setup( VcpTask t, VcpStorage * storages,
   uint32_t gx, uint32_t gy, uint32_t gz, void * constants );

/** Setup multi-run task.
\param t Task handle
\param nparts Number of consecutive runs of `t`
\return Pointer to [#Vcp_Part] array. The values can be modified and will be used on task run. */
VcpPart vcp_task_parts( VcpTask t, uint32_t nparts );

/** Start task on GPU
\param t Task handle */
void vcp_task_start( VcpTask t );

/** Wait for task to finish
\param t Task handle
\param timeoutMsec Timeout in milliseconds
\return `true` on success or error, `false` on timeout */
bool vcp_task_wait( VcpTask t, uint32_t timeoutMsec );

/** Dispose task
Usually not needed because [#vcp_done] frees up all task resources.
\param t Task handle */
void vcp_task_free( VcpTask t );

#ifdef __cplusplus
}
#endif

#endif // VULCOMPH
