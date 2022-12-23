# Vulcmp documentation

Vulcmp is a simple C library to make it mush easier GPU computing in your programs. 
It is a wrapper around the Vulkan library but it is much easier to learn and use.

* [Important types](#important-types)
* [Important functions](#important-functions)
* [Less important types](#less-important-types)
* [Less important functions](#less-important-functions)
* [Initialization flags](#initialization-flags)
* [Error codes](#error-codes)
* [Example code](#example-code)

## Important types

**VcpVulcomp**: opaque type handle for GPU access
**VcpStorage**: opaque type handle for GPU accessible memory
**VcpTask**: opaque type handle for GPU task (program)

## Important functions

    int vcp_error()
Returns [error code](#error-codes) of last call. 0 means no error.

---
    VcpVulcomp vcp_init( VcpStr appName, uint32_t flags )
GPU initialization.
- *appName*: name of your application
- *flags*: bitmask of *VcpFlags*
- *returns* handle to GPU processing

---
    void vcp_done()
End of GPU usage. Also frees up all corresponding storages and tasks.

---
    VcpStorage vcp_storage_create( VcpVulcomp v, uint64_t size )
Allocate GPU accessible memory for use with tasks.
- *v*: GPU handle
- *size*: memory size
- *returns* handle to allocated memory

---
    void * vcp_storage_address( VcpStorage s )
Memory address of allocated memory. It can be read or writter through this pointer.
- *s*: memory handle
- *returns* pointer to memory

---
    VcpTask vcp_task_create( VcpVulcomp v, void * data, uint64_t size, VcpStr entry, uint32_t nstorage )
Create a new GPU task.
- *v*: GPU handle
- *data*: pointer to the SPIR-V code of module
- *size*: size of SPIR-V module in bytes
- *entry*: entry point (function name) of task in the SPIR-V code
- *nstorage*: number of storage buffers accessed by the task
- *returns* handle to task

---
    void vcp_task_setup( VcpTask t, VcpStorage * storages, uint32_t gx, uint32_t gy, uint32_t gz )
Sets up task before starting. You need to determine the exact memory storages and group sizes before running the task.
- *t*: task handle
- *storages*: pointer to array of storages. Size must be same as *nstorage* in task creation.
- *gx*: group size in X dimension
- *gy*: group size in Y dimension
- *gz*: group size in Z dimension

---
    void vcp_task_start( VcpTask t )
Start the GPU task.
- *t*: task handle

---
    bool vcp_task_wait( VcpTask t, uint32_t timeoutMsec )
Wait for task to terminate.
- *t*: task handle
- *timeoutMsec*: maximum number of milliseconds to wait. If 0, returns immediately
- *returns* true is task ends in *timeoutMsec* or if an error occured

## Less important types

**VcpStr**: shorthand for const char *
**VcpFlag**: [initialization flag](#initialization-flags)
**VcpScorer**: function which returns a score for an object to help vulcmp select the best. 
 Larger value is better, less than zero value means object is not suitable, so it wont be selected

## Less important functions

    VcpTask vcp_task_create_file( VcpVulcomp *v*, VcpStr *filename*, VcpStr *entry*, uint32_t *nstorage* )
Create task directly from a file. Calls *vcp_task_create* after reading whole file to memory.
- *v*: GPU handle
- *filename*: filename to load
- *entry*: task entry point
- *nstorage*: number of storage buffers accessed by the task
- *returns* task handle

---
    void vcp_select_physical( VcpVulcomp v, VcpScorer s )
Selects best physical device for computation.
- *v*: GPU handle
- *s*: scorer function for a device (object will be a pointer to VkPhysicalDevice)

---
    void vcp_select_queue( VcpVulcomp v, VcpScorer s)
Selects best queue family for computation.
- *v*: GPU handle
- *s*: scorer function for a queue family (object will be a pointer to VkQueueFamilyProperties)

---
    int vcp_physical_score( void * p )
Default score function for a physical device
- *p*: the device to be scored. Pointer to vulkan VkPhysicalDevice
- *returns* score of physical device. Prefers discrete, intergated, virtual and cpu types in that order.

---
    int vcp_family_score**( void * f )
Default score function for a queue family
- *f*: the queue family to be scored. Pointer to a vulkan VkQueueFamilyProperties structure
- *returns* score of queue famly. Accepts only compute queues.

---
    void vcp_check_fail()
Checks last error code (*vcp_error*) and terminates program with an error message if it was not VCP_SUCCESS or VCP_TIMEOUT.

## Initialization flags

The following flags can be used in *vcp_init*

- **VCP_VALIDATION**: program will use vulkan validation layer and show validation errors.

## Error codes

Any vulkan [VkResult](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkResult.html) code can appear as a result of *vcp_error*. There are also some vulcmp specific codes:

- **VCP_SUCCESS**: last operation terminated successfully. Same as *VK_SUCCESS*
- **VCP_TIMEOUT**: *vcp_task_wait* timeouted. Same as *VK_TIMEOUT*
- **VCP_NOPHYSICAL**: no suitable physical devices found
- **VCP_NOFAMILY**: no suitable queue families found
- **VCP_NOMEMORY**: no suitable GPU memory found
- **VCP_NOFILE**: file could not be read in *vcp_task_create_file*
- **VCP_RUNNIG**: function could not be called because task is alread running
- **VCP_NOGROUP**: a group size is 0
- **VCP_NOSTORAGE**: storage buffers havent been chosen with *vcp_task_setup* before starting task

## Example code
   
A vulcmp program more or less would have the following structure:

    // initialize GPU
    VcpVulcomp v = vcp_init( "vcptest", VCP_VALIDATION );
    // allocate GPU memory
    VcpStorage s = vcp_storage_create( v, 320*200*4 );
    // create task
    VcpTask t = vcp_task_create_file( v, "example.spv", "main", 1 );
    // initialize task
    vcp_task_setup( t, &s, 320, 200, 1 );
    // start task
    vcp_task_start( t );
    // check if all went well
    vcp_check_fail();
    // wait for termination
    vcp_task_wait( t, 1000*60 );
    // use result
    void * data = vcp_storage_address( s );
    consume( data );
    // clean up
    vcp_done( v );
    
