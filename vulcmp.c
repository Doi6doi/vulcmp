#include "vulcmp.h"
#include "vulkan/vulkan.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define VCP_DEBUG( ... ) { fprintf(stderr, __VA_ARGS__ ); }
#define VCP_FAIL( ... ) { VCP_DEBUG( __VA_ARGS__ ); exit(1); }
#define VCP_REALLOC( p, type, n ) (type *)realloc( p, (n)*sizeof(type) )

VkResult vcpResult = VK_SUCCESS;

typedef struct Vcp__Storage {
   VcpVulcomp vulcomp;
   VkBuffer buffer;
   VkDeviceMemory memory;
   uint64_t size;
   void * address;
   bool coherent;
   bool todev;
} Vcp_Storage;


typedef struct Vcp__Task {
   VcpVulcomp vulcomp;
   VkShaderModule shader;
   VkCommandBuffer command;
   VkDescriptorSetLayout desclay;
   VkDescriptorPool descs;
   VkDescriptorSet desc;
   VkPipelineLayout pipelay;
   VkPipeline pipe;
   VkFence fence;
   bool running;
   uint32_t gx, gy, gz;
   VcpStr entry;
   uint32_t nstorage;
   VcpStorage * storages;
} Vcp_Task;


typedef struct Vcp__Vulcomp {
   VkInstance instance;
   VkPhysicalDevice physical;
   int32_t family;
   VkDevice device;
   VkQueue queue;
   VkCommandPool commands;
   uint32_t nstorage;
   VcpStorage * storages;
   uint32_t ntask;
   VcpTask * tasks;
} Vcp_Vulcomp;


VcpVulcomp vcp_init( VcpStr appName, uint32_t flags ) {
   vcpResult = VK_ERROR_OUT_OF_HOST_MEMORY;
   VcpVulcomp ret = VCP_REALLOC( NULL, Vcp_Vulcomp, 1 );
   if ( ! ret ) return NULL;
   ret->instance = 0;
   ret->physical = 0;
   ret->family = -1;
   ret->device = 0;
   ret->queue = 0;
   ret->nstorage = 0;
   ret->storages = NULL;
   ret->ntask = 0;
   ret->tasks = NULL;
   ret->commands = 0;
   VkApplicationInfo ai = {
	  .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	  .pNext = NULL,
	  .pApplicationName = appName,
	  .applicationVersion = 0,
	  .pEngineName = NULL,
	  .engineVersion = 0,
	  .apiVersion = 0
   };
   VcpStr layers[1];
   int nlayers = 0;
   if ( flags & VCP_VALIDATION )
      layers[nlayers++] = "VK_LAYER_KHRONOS_validation";
   VkInstanceCreateInfo ici = {
	  .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .pApplicationInfo = &ai,
	  .enabledLayerCount = nlayers,
	  .ppEnabledLayerNames = layers,
	  .enabledExtensionCount = 0,
	  .ppEnabledExtensionNames = NULL
   };
   vcpResult = vkCreateInstance( &ici, NULL, &(ret->instance) );
   return ret;
}


void vcp_select_physical( VcpVulcomp v, VcpScorer s ) {
   uint32_t nphys;
   if (( vcpResult = vkEnumeratePhysicalDevices(
      v->instance, & nphys, NULL )))
      return;
   VkPhysicalDevice * phys = VCP_REALLOC( NULL, VkPhysicalDevice, nphys );
   if ( ! phys ) {
      vcpResult = VK_ERROR_OUT_OF_HOST_MEMORY;
	  return;
   }
   if (( vcpResult = vkEnumeratePhysicalDevices( 
      v->instance, & nphys, phys )))
      return;
   int best = -1;
   v->physical = 0;
   for ( int i=0; i < nphys; ++i) {
	  int si = (*s)( phys+i );
	  if ( si > best ) {
         vcpResult = VK_SUCCESS;
		 v->physical = phys[i];
		 best = si;
      }
   }
   if ( ! v->physical )
      vcpResult = VCP_NOPHYSICAL;
   phys = VCP_REALLOC( phys, VkPhysicalDevice, 0 );
}


void vcp_select_family( VcpVulcomp v, VcpScorer s ) {
   uint32_t nqfam;
   vkGetPhysicalDeviceQueueFamilyProperties(
      v->physical, &nqfam, 0 );
   VkQueueFamilyProperties * qfams = VCP_REALLOC( NULL, VkQueueFamilyProperties, nqfam );
   if ( ! qfams ) {
      vcpResult = VK_ERROR_OUT_OF_HOST_MEMORY;
	  return;
   }
   vkGetPhysicalDeviceQueueFamilyProperties(
      v->physical, &nqfam, qfams );
   int best = -1;
   v->family = -1;
   for ( int i=0; i < nqfam; ++i ) {
	  int si = (*s)( qfams+i );
	  if ( si > best ) {
         vcpResult = VK_SUCCESS;
		 v->family = i;
		 best = si;
      }
   }
   if ( 0 > v->family )
	  vcpResult = VCP_NOFAMILY;
   qfams = VCP_REALLOC( qfams, VkQueueFamilyProperties, 0 );
}

int vcp_error() {
	return (int)vcpResult;
}

void vcp_check_fail() {
   if ( VK_SUCCESS != vcpResult && VK_TIMEOUT != vcpResult )
      VCP_FAIL( "Error: vulcomp %d\n", vcpResult );
}

/// free command
static void vcp_free_command( VcpTask t ) {
   vkFreeCommandBuffers( t->vulcomp->device,
      t->vulcomp->commands, 1, & t->command );
   t->command = 0;
}

/// remove task from list
static void vcp_task_remove( VcpTask t ) {
   VcpVulcomp v = t->vulcomp;
   VcpTask * tt = v->tasks;
   int n = v->ntask;
   for ( int i = n-1; 0 <= i; --i ) {
      if (tt[i] == t ) {
	 tt[i] = tt[n-1];
	 v->tasks = VCP_REALLOC( tt, VcpTask, n-1 );
         -- v->ntask;
	 return;
      }
   }
}


/// free up task
void vcp_task_free( VcpTask t ) {
   vcp_task_remove( t );
   VkDevice d = t->vulcomp->device;
   if ( t->command )
	  vcp_free_command( t );
   if ( t->fence ) {
	  vkDestroyFence( d, t->fence, NULL );
	  t->fence = 0;
   }
   if ( t->pipe ) {
	  vkDestroyPipeline( d, t->pipe, NULL );
	  t->pipe = 0;
   }
   if ( t->pipelay ) {
	  vkDestroyPipelineLayout( d, t->pipelay, NULL );
	  t->pipelay = 0;
   }
   if ( t->descs ) {
	  vkDestroyDescriptorPool( d, t->descs, NULL );
	  t->descs = 0;
   }
   if ( t->desclay ) {
	  vkDestroyDescriptorSetLayout( d, t->desclay, NULL );
	  t->desclay = 0;
   }
   if ( t->shader ) {
	  vkDestroyShaderModule( d, t->shader, NULL );
	  t->shader = 0;
   }
   t->entry = VCP_REALLOC( (char *)t->entry, char, 0 );
   t->storages = VCP_REALLOC( t->storages, VcpStorage, 0 );
   t = VCP_REALLOC( t, Vcp_Task, 0 );
}

/// remove storage from list
static void vcp_storage_remove( VcpStorage s ) {
   VcpVulcomp v = s->vulcomp;
   VcpStorage * ss = v->storages;
   int n = v->nstorage;
   for ( int i = n-1; 0 <= i; --i ) {
      if (ss[i] == s ) {
	 ss[i] = ss[n-1];
	 v->storages = VCP_REALLOC( ss, VcpStorage, n-1 );
         -- v->nstorage;
	 return;
      }
   }
}

/// free up storage
void vcp_storage_free( VcpStorage s ) {
   vcp_storage_remove( s );
   if ( s->memory ) {
      vkFreeMemory( s->vulcomp->device, s->memory, NULL );
      s->memory = 0;
   }
   if ( s->buffer ) {
      vkDestroyBuffer( s->vulcomp->device, s->buffer, NULL );
      s->buffer = 0;
   }
   s = VCP_REALLOC( s, Vcp_Storage, 0 );
}


void vcp_done( VcpVulcomp v ) {
    for ( int i=v->nstorage-1; 0 <= i; --i )
       vcp_storage_free( v->storages[i] );
    for ( int i=v->ntask-1; 0 <= i; --i )
       vcp_task_free( v->tasks[i] );
    if ( v->commands ) {
       vkDestroyCommandPool( v->device, v->commands, NULL );
       v->commands = 0;
    }
    if ( v->device ) {
       vkDestroyDevice( v->device, NULL );
       v->device = 0;
    }
    vkDestroyInstance( v->instance, NULL );
    v = VCP_REALLOC( v, Vcp_Vulcomp, 0 );
}


static int vcp_phyiscal_type_score( VkPhysicalDeviceType pdt ) {
   switch ( pdt ) {
	  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return 5;
	  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 4;
	  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return 3;
	  case VK_PHYSICAL_DEVICE_TYPE_CPU: return 2;
	  default: return 0;
   }
} 

int vcp_physical_score( void * p ) {
   VkPhysicalDevice pd = *(VkPhysicalDevice *)p;
   VkPhysicalDeviceProperties pdp;
   vkGetPhysicalDeviceProperties( pd, &pdp );
   return vcp_phyiscal_type_score( pdp.deviceType );
}

int vcp_family_score( void * f ) {
   VkQueueFamilyProperties * qfp = (VkQueueFamilyProperties *)f;
   if ( qfp->queueFlags & VK_QUEUE_COMPUTE_BIT )
      return 1;
      else return -1;
}

/// create device
static void vcp_create_device( VcpVulcomp v ) {
   float priority = 0;
   VkDeviceQueueCreateInfo qci = {
	  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .queueFamilyIndex = v->family,
	  .queueCount = 1,
	  .pQueuePriorities = &priority
   };	
   VkDeviceCreateInfo dci = {
	  .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .queueCreateInfoCount = 1,	
	  .pQueueCreateInfos = &qci,
	  .enabledLayerCount = 0,
	  .ppEnabledLayerNames = NULL,
	  .enabledExtensionCount = 0,
	  .ppEnabledExtensionNames = NULL,
	  .pEnabledFeatures = NULL
   };
   if (( vcpResult = vkCreateDevice( 
      v->physical, &dci, NULL, &v->device )))
      return;
}

/// create device queue
static void vcp_create_queue( VcpVulcomp v ) {
   vkGetDeviceQueue( v->device, v->family, 0, &v->queue );
}


/// create descriptor layout
static void vcp_create_desclay( VcpTask t ) {
   VkDescriptorSetLayoutBinding dlbs[ t->nstorage ];
   for ( int i=0; i < t->nstorage; ++i ) {
      VkDescriptorSetLayoutBinding dlb = {
         .binding = i,
       	 .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
	 .pImmutableSamplers = NULL
      };
      dlbs[i] = dlb;
   }
   VkDescriptorSetLayoutCreateInfo dli = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .bindingCount = t->nstorage,
      .pBindings = dlbs
   };
   vcpResult = vkCreateDescriptorSetLayout( t->vulcomp->device,
      & dli, NULL, & t->desclay );
}


/// create pipeline layout
static void vcp_create_pipelay( VcpTask t ) {
   VkPipelineLayoutCreateInfo pli = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .setLayoutCount = 1,
      .pSetLayouts = & t->desclay,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = 0
   };
   vcpResult = vkCreatePipelineLayout( t->vulcomp->device, &pli, NULL,
      & t->pipelay );
}


/// create pipeline
static void vcp_create_pipe( VcpTask t ) {
   VkComputePipelineCreateInfo pci = {
	  .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .stage = {
		 .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		 .pNext = NULL,
		 .flags = 0,
		 .stage = VK_SHADER_STAGE_COMPUTE_BIT,
		 .module = t->shader,
		 .pName = t->entry,
		 .pSpecializationInfo = NULL
      },
      .layout = t->pipelay,
      .basePipelineHandle = 0,
      .basePipelineIndex = 0
   };
   vcpResult = vkCreateComputePipelines( t->vulcomp->device, 0, 1,
      & pci, NULL, & t->pipe );
}      
      	

/// create command pool
static void vcp_create_commands( VcpVulcomp v ) {
   VkCommandPoolCreateInfo pci = {
	  .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .queueFamilyIndex = v->family
   };
   vcpResult = vkCreateCommandPool( v->device, &pci, NULL, & v->commands );
}


static bool vcp_prepare( VcpVulcomp v ) {
   if ( ! v->physical ) {
      vcp_select_physical( v, vcp_physical_score );
      if ( vcpResult ) return false;
   }
   if ( 0 > v->family ) {
      vcp_select_family( v, vcp_family_score );
      if ( vcpResult ) return false;
   }
   if ( ! v->device ) {
      vcp_create_device( v );
      if ( vcpResult ) return false;
   }
   if ( ! v->queue ) {
	  vcp_create_queue( v );
	  if ( vcpResult ) return false;
   } 
   if ( ! v->commands ) {
	   vcp_create_commands( v );
	   if ( vcpResult ) return false;
   }
   return true;
}

/// find memory for storage
static int vcp_find_memory( VcpVulcomp v, uint32_t bits, uint32_t flags,
   bool * coherent ) 
{
   VkPhysicalDeviceMemoryProperties pmp;
   vkGetPhysicalDeviceMemoryProperties( v->physical, & pmp );
   for (int i=0; i<	pmp.memoryTypeCount; ++i ) {
	   uint32_t pf = pmp.memoryTypes[i].propertyFlags;
	   if ( bits & (1 << i) && flags == ( pf & flags )) {
		  vcpResult = VK_SUCCESS;
		  * coherent = pf & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	      return i;
	   }
   }
   vcpResult = VCP_NOMEMORY;
   return -1;
}

VcpStorage vcp_storage_create( VcpVulcomp v, uint64_t size ) {
   vcpResult = VK_ERROR_OUT_OF_HOST_MEMORY;
   VcpStorage ret = VCP_REALLOC( NULL, Vcp_Storage, 1 );
   if ( ! ret ) return NULL;
   VcpStorage * vstorages = VCP_REALLOC( v->storages,
      VcpStorage, v->nstorage+1 );
   if ( ! vstorages ) return NULL;
   vstorages[ v->nstorage++ ] = ret;
   v->storages = vstorages;
   ret->vulcomp = v;
   ret->buffer = 0;
   ret->memory = 0;
   ret->size = size;
   ret->address = NULL;
   ret->todev = true;
   ret->coherent = false;
   if ( ! vcp_prepare( v )) return ret;
   uint32_t index = v->family;
   VkBufferCreateInfo bci = {
	  .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .size = size,
	  .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	  .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	  .queueFamilyIndexCount = 1,
	  .pQueueFamilyIndices = &index
   };
   if (( vcpResult = vkCreateBuffer( 
      v->device, &bci, NULL, &ret->buffer )))
      return ret;
   VkMemoryRequirements mr;
   vkGetBufferMemoryRequirements( v->device, ret->buffer, &mr );
   int memidx = vcp_find_memory( v, mr.memoryTypeBits, 
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, & ret->coherent );
   if ( vcpResult ) return ret;
   VkMemoryAllocateInfo mai = {
	  .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	  .pNext = NULL,
	  .allocationSize = mr.size,
	  .memoryTypeIndex = memidx
   };
   if (( vcpResult = vkAllocateMemory(
      v->device, &mai, NULL, &ret->memory )))
      return ret;
   if (( vcpResult = vkBindBufferMemory(
      v->device, ret->buffer, ret->memory, 0 )))
      return ret;
   return ret;      
}

/// create a fence
static void vcp_create_fence( VcpTask t ) {
   VkFenceCreateInfo fci = {
	  .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0
   };
   vcpResult = vkCreateFence( t->vulcomp->device, &fci, NULL, &t->fence );
}


/// create descriptor pool
static void vcp_create_descs( VcpTask t ) {
   VkDescriptorPoolSize dps = {
      .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = t->nstorage
   };
   VkDescriptorPoolCreateInfo dpi = {
	  .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .maxSets = 1,
	  .poolSizeCount = 1,
	  .pPoolSizes = & dps
   };
   vcpResult = vkCreateDescriptorPool( t->vulcomp->device, & dpi, NULL,
      & t->descs );
}


/// create and fill descriptor set
static void vcp_create_desc( VcpTask t ) {
   VkDescriptorSetAllocateInfo dai = {
     .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
     .pNext = NULL,
     .descriptorPool = t->descs,
     .descriptorSetCount = 1,
     .pSetLayouts = & t->desclay
   };
   if (( vcpResult = vkAllocateDescriptorSets( t->vulcomp->device, & dai,
      & t->desc )))
      return;
   VkDescriptorBufferInfo dbi = {
      .offset = 0,
      .range = VK_WHOLE_SIZE
   };
   VkWriteDescriptorSet wd = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = NULL,
      .dstSet = t->desc,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pImageInfo = NULL,
      .pBufferInfo = & dbi,
      .pTexelBufferView = NULL
   };
   int n = t->nstorage;
   for ( int i=0; i<n; ++i ) {
      wd.dstBinding = i,
      dbi.buffer = t->storages[i]->buffer;
      vkUpdateDescriptorSets( t->vulcomp->device, 1, &wd, 0, NULL );
   }
}


/// build command
static bool vcp_build_command( VcpTask t ) {
   VkCommandBufferBeginInfo bbi = {
	  .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .pInheritanceInfo = NULL
   };
   VkPipelineBindPoint cmp = VK_PIPELINE_BIND_POINT_COMPUTE;
   if (( vcpResult = vkBeginCommandBuffer( t->command, &bbi ))) 
      return false;
   vkCmdBindPipeline( t->command, cmp, t->pipe );
   vkCmdBindDescriptorSets( t->command, cmp, 
      t->pipelay, 0, 1, & t->desc, 0, NULL );
   vkCmdDispatch( t->command, t->gx, t->gy, t->gz );
   if (( vcpResult = vkEndCommandBuffer( t->command )))
      return false;  
   return true;
}


/// allocate and fill command
static void vcp_create_command( VcpTask t ) {
   VkCommandBufferAllocateInfo cai = {
	  .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	  .pNext = NULL,
	  t->vulcomp->commands,
	  VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	  1
   };
   vcpResult = vkAllocateCommandBuffers( t->vulcomp->device, 
      &cai, &t->command );
   if ( vcpResult ) return;
   if ( ! vcp_build_command( t ) )
      vcp_free_command( t );
}


/// turn storage towards device
static void vcp_storage_turn( VcpStorage s, bool todev ) {
   if ( s->todev == todev || s->coherent ) return;
   VkMappedMemoryRange mmr = {
	  .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
	  .pNext = NULL,
	  .memory = s->memory,
	  .offset = 0,
	  .size = s->size
   };
   VkDevice d = s->vulcomp->device;
   if ( todev ) {
	  vcpResult = vkFlushMappedMemoryRanges( d, 1, & mmr );
   } else {
	  vcpResult = vkInvalidateMappedMemoryRanges( d, 1, & mmr );
   }
   if ( ! vcpResult )
      s->todev = todev;
}


/// get storage memory address
void * vcp_storage_address( VcpStorage s ) {
   if ( ! s->address ) {
	  if (( vcpResult = vkMapMemory( s->vulcomp->device, s->memory,
	     0, s->size, 0, & s->address )))
	     return NULL;
   }
   vcp_storage_turn( s, false );
   return s->address;
}


/// prepare task for
static bool vcp_task_prepare( VcpTask t ) {
   if ( ! t->fence ) {
	  vcp_create_fence( t );
	  if ( vcpResult ) return false;
   }
   if ( ! t->desclay ) {
	  vcp_create_desclay( t );
	  if ( vcpResult ) return false;
   }
   if ( ! t->descs ) {
	  vcp_create_descs( t );
	  if ( vcpResult ) return false;
   }
   if ( ! t->desc ) {
	  vcp_create_desc( t );
	  if ( vcpResult ) return false;
   }
   if ( ! t->pipelay ) {
	  vcp_create_pipelay( t );
	  if ( vcpResult ) return false;
   }
   if ( ! t->pipe ) {
	  vcp_create_pipe( t );
	  if ( vcpResult ) return false;
   }
   if ( ! t->command ) {
	  vcp_create_command( t );
	  if ( vcpResult ) return false;
   }
   for ( int i=0; i < t->nstorage; ++i ) {
	  vcp_storage_turn( t->storages[i], true );
	  if ( vcpResult ) return false;
   } 
   return true;
}


VcpTask vcp_task_create( VcpVulcomp v, void * data, uint64_t size,
   VcpStr entry, uint32_t nstorage
) {
   vcpResult = VK_ERROR_INVALID_SHADER_NV;
   if ( 0 == size ) return NULL;
   vcpResult = VK_ERROR_OUT_OF_HOST_MEMORY;
   int es = strlen( entry );
   char * tentry = VCP_REALLOC( NULL, char, es+1 );
   if ( ! tentry ) return NULL;
   strcpy( tentry, entry );
   VcpStorage * tstorages = VCP_REALLOC( NULL, VcpStorage, nstorage );
   if ( ! tstorages ) return NULL;
   memset( tstorages, 0, nstorage * sizeof( VcpStorage ) );
   VcpTask ret = VCP_REALLOC( NULL, Vcp_Task, 1 );
   if ( ! ret ) return NULL;
   VcpTask * vtasks = VCP_REALLOC( v->tasks, VcpTask, v->ntask+1 );
   if ( ! vtasks ) return NULL;
   vtasks[ v->ntask++ ] = ret;
   v->tasks = vtasks;
   ret->vulcomp = v;
   ret->shader = 0;
   ret->fence = 0;
   ret->command = 0;
   ret->pipelay = 0;
   ret->pipe = 0;
   ret->desclay = 0;
   ret->descs = 0;
   ret->desc = 0;
   ret->running = 0;
   ret->gx = ret->gy = ret->gz = 0;
   ret->entry = tentry;
   ret->nstorage = nstorage;
   ret->storages = tstorages;
   if ( ! vcp_prepare(v) ) return ret;
   VkShaderModuleCreateInfo sci = {
	  .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	  .pNext = NULL,
	  .flags = 0,
	  .codeSize = size,
	  .pCode = data
   };
   vcpResult = vkCreateShaderModule( v->device, &sci, NULL, &ret->shader );
   return ret;
}


VcpTask vcp_task_create_file( VcpVulcomp v, VcpStr filename, 
   VcpStr entry, uint32_t nstorage )  
{
   uint64_t size = 0;	
   char * data = NULL;
   FILE * fh = fopen( filename, "rb" );
   if ( fh ) {
      fseek( fh, 0L, SEEK_END );
      size = ftell( fh );
      if ( size ) {
         data = VCP_REALLOC( NULL, char, size );
         if ( data ) {
		    fseek( fh, 0L, SEEK_SET );
		    if ( 1 == fread( data, size, 1, fh )) {
			   vcpResult = VK_SUCCESS;
		    } else {
			   vcpResult = VCP_NOFILE;
			}
		 } else {
			vcpResult = VK_ERROR_OUT_OF_HOST_MEMORY;
	     }
	  } else {
		  vcpResult = VCP_NOFILE;
	  }
	  fclose( fh );
   } else {
	  vcpResult = VCP_NOFILE;
   }
   VcpTask ret = NULL;
   if ( VK_SUCCESS == vcpResult )
      ret = vcp_task_create( v, data, size, entry, nstorage );
   if ( data )
      data = VCP_REALLOC( data, char, 0 );
   return ret;
}


void vcp_task_start( VcpTask t ) {
   if ( ! vcp_task_prepare( t ))
      return;
   VkSubmitInfo si = {
	  .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	  .pNext = NULL,
	  .waitSemaphoreCount = 0,
	  .pWaitSemaphores = NULL,
	  .pWaitDstStageMask = NULL,
	  .commandBufferCount = 1,
	  .pCommandBuffers = & t->command,
	  .signalSemaphoreCount = 0,
	  .pSignalSemaphores = NULL
   };
   vcpResult = vkQueueSubmit( t->vulcomp->queue, 1, & si, t->fence );
   t->running = ! vcpResult;
}


void vcp_task_setup( VcpTask t, VcpStorage * storages, uint32_t gx,
   uint32_t gy, uint32_t gz )
{
   for ( int i=0; i < t->nstorage; ++i ) {
	  if ( ! storages[i] ) {
		 vcpResult = VCP_NOSTORAGE;
		 return;
	  }
	  t->storages[i] = storages[i];
   }
   if ( 0 == gx * gy * gy ) {
	  vcpResult = VCP_NOGROUP;
	  return;
   }
   t->gx = gx;
   t->gy = gy;
   t->gz = gz;
   if ( ! vcp_task_prepare( t ) )
      return;
   if ( ! vcp_task_wait( t, 0 )) {
	  vcpResult = VCP_RUNNING;
      return;
   }
}


bool vcp_task_wait( VcpTask t, uint32_t timeoutMsec ) {
   if ( ! t->vulcomp->device || ! t->fence )
      return true;
   if ( ! t->running ) 
      return true;   
   vcpResult = vkWaitForFences( t->vulcomp->device, 1, & t->fence, true,
      1000*timeoutMsec );
   t->running = VK_TIMEOUT == vcpResult;
   return ! t->running;
}   



