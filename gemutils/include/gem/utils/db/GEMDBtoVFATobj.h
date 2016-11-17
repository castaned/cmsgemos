#ifndef GEM_UTILS_DB_GEMDBTOVFATOBJ_H
#define GEM_UTILS_DB_GEMDBTOVFATOBJ_H

#include "log4cplus/logger.h"

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Table.h"
#include "gem/hw/vfat/HwVFAT2.h"


namespace gem {
  namespace utils {
    namespace db { 
      
      class GEMDBtoVFATobj
      {
	
      public:
	
	GEMDBtoVFATobj();
	~GEMBDtoVFATobj();
	
	gem::hw::vfat::VFAT2ControlParams VFAT2Params;
	
	VFAT2Params setVFATparams(xdata::Table VFATDB);
	
      }
    }
  }
}
  
	  
#endif
