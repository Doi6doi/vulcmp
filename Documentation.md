# Vulcmp documentation

Vulcmp is a simple C library to make it mush easier GPU computing in your programs. 
It is a wrapper around the Vulkan library but it is much easier to learn and use.

- [Important types](#important-types)
- [Important functions](#important-functions)
- [Less important types](#less-important-types)
- [Less important functions](#less-important-functions)
- [Initialization flags](#initialization-flags)
- [Error codes](#error-codes)
- [Example code](#example-code)

## Important types

- `VcpVulcomp`: opaque type handle for GPU access
- `VcpStorage`: opaque type handle for GPU accessible memory
- `VcpTask`: opaque type handle for GPU task (program)

## Important functions

```c
    int vcp_error()
```
Returns [error code](#error-codes) of last call. 0 means no error.

---
```c
    VcpVulcomp vcp_init( VcpStr appName, uint32_t flags ) 
```
GPU initialization.
- `appName`: name of your application
- `flags`: bitmask of [VcpFlags](#initialization-flags)
- *returns* handle to GPU processing

---
```c
void vcp_done()
```
End of GPU usage. Also frees up all corresponding storages and tasks.

---
```c
VcpStorage vcp_storage_create( VcpVulcomp v, uint64_t size )
```
Allocate GPU accessible memory for use with tasks.
- `v`: GPU handle
- `size`: memory size
- *returns* handle to allocated memory

---
```c
void * vcp_storage_address( VcpStorage s )
```
Memory address of allocated memory. It can be read or written through this pointer. Needs to be called after running GPU task in order to see data changes.
- `s`: memory handle
- *returns* pointer to memory

---
```c
uint64_t vcp_storage_size( VcpStorage s )
```
Size of allocated memory in bytes.
- `s`: memory handle
- *returns* allocated memory size

---
```c
uint64_t vcp_storage_copy( VcpStorage src, VcpStorage dst,  )
```
Size of allocated memory in bytes.
- `s`: memory handle
- *returns* allocated memory size

---
```c
VcpTask vcp_task_create( VcpVulcomp v, void * data, uint64_t size, VcpStr entry, uint32_t nstorage, uint32_t constsize )
```
Create a new GPU task.
- `v`: GPU handle
- `data`: pointer to the [SPIR-V](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html) code of module
- `size`: size of [SPIR-V](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html) module in bytes
- `entry`: entry point (function name) of task in the [SPIR-V](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html) code
- `nstorage`: number of storage buffers accessed by the task
- `constsize`: push constants size in bytes
- *returns* handle to task

---
```c
void vcp_task_setup( VcpTask t, VcpStorage * storages, uint32_t gx, uint32_t gy, uint32_t gz, void * constants )
```
Sets up task before starting. You need to determine the exact memory storages and group sizes before running the task.
- `t`: task handle
- `storages`: pointer to array of storages. Size must be same as `nstorage` in task creation.
- `gx`: group size in X dimension
- `gy`: group size in Y dimension
- `gz`: group size in Z dimension
- `constants`: push constant values for execution

---
```c
void vcp_task_start( VcpTask t )
```
Start the GPU task.
- `t`: task handle

---
```c
bool vcp_task_wait( VcpTask t, uint32_t timeoutMsec )
```
Wait for task to terminate.
- `t`: task handle
- `timeoutMsec`: maximum number of milliseconds to wait. If 0, returns immediately
- *returns* true is task ends in `timeoutMsec` or if an error occured

## Less important types

- `VcpStr`: shorthand for const char *
- `VcpFlag`: [initialization flag](#initialization-flags)
- `VcpPart`: struct used to run task for given parts of space
    - `baseX`,`baseY`,`baseZ`: coorindates of starting group
    - `countX`,`countY`,`countZ`: sizes of area to run task on
    - `constants`: pointer to shader constants if any
- `VcpScorer`: function which returns a score for an object to help vulcmp select the best. 
 Larger value is better, less than zero value means object is not suitable, so it wont be selected

## Less important functions

```c
void vcp_check_fail()
```
Checks last error code (*vcp_error*) and terminates program with an error message if it was not *VCP_SUCCESS* or *VCP_TIMEOUT*.

---
```c
VcpPart * void vcp_task_parts( VcpTask t, uint32_t nparts );
```
Setup task to run in parts. Parts will run after each other, with memory barrier between them.
Each part can have different constants
- `t`: task handle
- `nparts`: number of parts to run
- `parts`: pointer to nparts writeable records. (task manages the array)

---
```c
VcpTask vcp_task_create_file( VcpVulcomp v, VcpStr filename, VcpStr entry, uint32_t nstorage )
```
Create task directly from a file. Calls *vcp_task_create* after reading whole file to memory.
- `v`: GPU handle
- `filename`: filename to load
- `entry`: task entry point
- `nstorage`: number of storage buffers accessed by the task
- `returns` task handle

---
```c
void vcp_select_physical( VcpVulcomp v, VcpScorer s )
```
Selects best physical device for computation.
- `v`: GPU handle
- `s`: scorer function for a device (object will be a pointer to [VkPhysicalDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html))

---
```c
void vcp_select_queue( VcpVulcomp v, VcpScorer s)
```
Selects best queue family for computation.
- `v`: GPU handle
- `s`: scorer function for a queue family (object will be a pointer to [VkQueueFamilyProperties](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFamilyProperties.html))

---
```c
int vcp_physical_score( void * p )
```
Default score function for a physical device. Prefers discrete, intergated, virtual and cpu types in that order.
- `p`: the device to be scored. Pointer to [VkPhysicalDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html)
- *returns* score of physical device. 

---
```c
int vcp_family_score( void * f )
```
Default score function for a queue family. Accepts only compute queues.
- `f`: the queue family to be scored. Pointer to a [VkQueueFamilyProperties](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFamilyProperties.html) structure
- *returns* score of queue famly. 

---
```c
void vcp_storage_free( VcpStorage s )
```
Frees up resources used by `s`. You usually don't need to call this as vulcomp frees up everything on `vcp_done`. It is only needed if you wish to save memory earlier.
- `s`: storage to dispose

---
```c
void vcp_task_free( VcpTask t )
```
Frees up resources used by `t`. You usually don't need to call this as vulcomp frees up everything on `vcp_done`. It is only needed if you wish to save memory earlier.
- `t`: task to dispose

## Initialization flags

The following flags can be used in *vcp_init*

- `VCP_VALIDATION`: program will use [Vulkan validation layer](https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers) and show validation errors.
- `VCP_ATOMIC_FLOAT`: request support for atomic float operations

## Error codes

Any vulkan [VkResult](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkResult.html) code can appear as a result of *vcp_error*. There are also some vulcmp specific codes:

- `VCP_SUCCESS`: last operation terminated successfully. Same as `VK_SUCCESS`
- `VCP_TIMEOUT`: `vcp_task_wait` timeouted. Same as `VK_TIMEOUT`
- `VCP_HOSTMEM`: host memory allocation failed. Same as `VK_ERROR_OUT_OF_HOST_MEMORY`
- `VCP_NOPHYSICAL`: no suitable physical devices found
- `VCP_NOFAMILY`: no suitable queue families found
- `VCP_NOMEMORY`: no suitable GPU memory found
- `VCP_NOFILE`: file could not be read in `vcp_task_create_file`
- `VCP_RUNNING`: function could not be called because task is already running
- `VCP_NOGROUP`: a group size is 0
- `VCP_NOSTORAGE`: storage buffers havent been chosen with `vcp_task_setup` before starting task

## Example code

A vulcmp program more or less would have the following structure:

```c
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
```
    
