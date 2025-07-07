#ifndef VULCOMPHPP
#define VULCOMPHPP

/**
# vulcmp.hpp

[Vulcmp](index) is a *C* and *C++* library which
help you to make GPU computing easily.

The `vulcmp.hpp` header is for the *C++* part.
It contains the basic classes for computing
and the correspondent methods.

All the declarations are in the `vcp` namespace
*/

/** ## Contents
\toc */

/** ## Details */

#include <stdint.h>

namespace vcp {

/// Shorthand for C string
typedef const char * Str;

/** Scorer function for vulcmp
to choose physical device or queue family.
The item with the largest value will be chosen.
Negative score items will not be chosen.
\param x The item being scored
\return Score value */
typedef int32_t (* Scorer )( void * x );


/// Flags for vulcomp initialization
typedef enum {
   /// Use validation layer. It will show messages about Vulkan errors
   VALIDATION=1,
   /// Needed to use atomic float operations in shaders
   ATOMIC_FLOAT=2,
   /// Needed to use uint8 and int8 types in shaders
   BIT8=4
} Flags;

/// Vulcmp system
class Vulcomp {
protected:
   void * impl;
public:
   /** Start Vulcmp system
   \param name Program name forwarded to vulkan
   \param flags Initialization flags ([#Flags]) */
   Vulcomp( Str name, uint32_t flags );

   /// Terminate Vulcmp system
   ~Vulcomp();

   /** Initialization flags
   \return Flag mask */
   uint32_t flags();

   /** Set scorer for choosing physical device.
   Must be called before any task or storage creation.
   \param s Physical device scorer. Called with [VkPhysicalDevice](https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDevice.html) */
   void selectPhysical( Scorer & s );

   /** Set scorer for choosing queue family.
   Must be called before any task or storage creation.
   \param s Queue family scorer. Called with [VkQueueFamilyProperties](https://registry.khronos.org/vulkan/specs/latest/man/html/VkQueueFamilyProperties.html> */
   void selectFamily( Scorer & s);

   /** Handle for C interface
   \return Vulcmp handle */
   void * handle();
};

/// GPU-accessible memory
class Storage {
protected:
   void * impl;
public:
   /** Allocate memory
   \param v Vulcmp system
   \param size Size of memory in bytes */
   Storage( Vulcomp & v, uint64_t size );

   /// Dispose memory
   ~Storage();

   /** Address
   \return CPU address of memory */
   void * address();

   /** Size
   \return Memory size in bytes */
   uint64_t size();

   /** GPU-copy of a storage part
   Usually faster than memcpy because data is not synced with CPU
   \param dst Destination storage handle
   \param si Index of first copied byte in this
   \param di Index of first copied byte in `dst`
   \param count Number of bytes to copy */
   void copy( Storage & dst, uint32_t si, uint32_t di, uint32_t count );

   /** Handle for C interface
   \return Storage handle */
   void * handle();
};

/// Exception thrown on error
struct VExc {
   /// Error code. See [C library](C)
   int code;
   /** Create new `VExc`.
   \param code Error code */
   VExc(int code);
};


/// Configuration datas for one run in a multi-run task.
struct Part {
   /// Group count for X coordinate
   uint32_t countX;
   /// Group count for Y coordinate
   uint32_t countY;
   /// Group count for Z coordinate
   uint32_t countZ;
   /// Constant values for part
   void * constants;
};


/// Task (subprogram) to run on GPU
class Task {
protected:
   uint32_t nstor;
   void * impl;
public:
   /** Create new task
   \param v Vulcmp system
   \param data Address of [SPIR-V](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html) compute module
   \param size Size of `data` in bytes
   \param entry Name of executable function in `data`
   \param nstorage Number of storages used by task
   \param constsize Size of constant data in bytes */
   Task( Vulcomp & v, void * data, uint64_t size, Str entry,
      uint32_t nstorage, uint32_t constsize );

   /** Create a GPU task by file
   \param v Vulcmp handle
   \param filename Name of file containing [SPIR-V](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html) compute module
   \param entry Name of executable function in `data`
   \param nstorage Number of storages used by task
   \param constsize Size of constant data in bytes */
   Task( Vulcomp & v, Str filename, Str entry, uint32_t nstorage, uint32_t constsize );

   /// Dispose task
   ~Task();

   /** Setup task before first run
   \param storages Handles of storages used by task
   \param gx Number of groups on X coordinate
   \param gy Number of groups on Y coordinate
   \param gz Number of groups on Z coordinate
   \param constants Constant values for the task */
   void setup( Storage * storages, uint32_t gx, uint32_t gy, uint32_t gz,
      void * constants );

   /** Setup multi-run task.
   \param nparts Number of consecutive runs of `t`
   \return Pointer to [#Part] array. The values can be modified and will be used on task run. */
   Part * parts( uint32_t nparts );

   /// Start task
   void start();

   /** Wait for task to finish
   \param timeoutMsec Timeout in milliseconds
   \return `true` on success, `false` on timeout */
   bool wait( uint32_t timeoutMsec );

   /** Handle for C interface
   \return Task handle */
   void * handle();
};

}

#endif // VULCOMPHPP
