#include "vulcmp.hpp"
#include "vulcmp.h"

using namespace vcpc;

namespace vcp {

static void check() {
   int e = vcp_error();
   if ( 0 != e )
      throw VExc(e);
}

static void * checkP( void * p ) {
   if ( !p )
      throw VExc(vcp_error());
   return p;
}

VExc::VExc(int code) {
   this->code = code;
}

Vulcomp::Vulcomp( Str name, uint32_t flags ) {
   impl = checkP( vcp_init( name, flags ));
}

Vulcomp::~Vulcomp() {
   vcp_done( (VcpVulcomp)impl );
}

void * Vulcomp::handle() {
   return impl;
}

Storage::Storage( Vulcomp & v, uint64_t size ) {
   impl = checkP( vcp_storage_create( (VcpVulcomp)v.handle(), size ));
}

Storage::~Storage() {
   vcp_storage_free( (VcpStorage)impl );
}

void * Storage::address() {
   return checkP( vcp_storage_address( (VcpStorage)impl ));
}

void * Storage::handle() {
   return impl;
}

Task::Task( Vulcomp & v, Str filename, Str entry, uint32_t nstorage,
   uint32_t constsize )
{
   nstor = nstorage;
   impl = checkP( vcp_task_create_file( (VcpVulcomp)v.handle(),
      filename, entry, nstor, constsize ));
}

Task::~Task() {
   vcp_task_free( (VcpTask)impl );
}

void Task::setup( Storage * storages, uint32_t gx, uint32_t gy, uint32_t gz,
   void * constants )
{
   VcpStorage ss[nstor];
   for (int i=0; i<nstor; ++i)
      ss[i] = (VcpStorage)storages[i].handle();
   vcp_task_setup( (VcpTask)impl, ss, gx, gy, gz, constants );
   check();
}

void Task::start() {
   vcp_task_start( (VcpTask)impl );
   check();
}

bool Task::wait( uint32_t timeoutMsec ) {
   bool ret = vcp_task_wait( (VcpTask)impl, timeoutMsec );
   if (ret) check();
   return ret;
}

}
