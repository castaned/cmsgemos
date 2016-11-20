#ifndef GEM_UTILS_DB_GEMDBTOVFATOBJ_H
#define GEM_UTILS_DB_GEMDBTOVFATOBJ_H

#include "log4cplus/logger.h"

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Table.h"
//#include "gem/hw/vfat/HwVFAT2.h"

#include "gem/hw/vfat/VFAT2Settings.h"
#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include "gem/hw/vfat/VFAT2Enums2Strings.h"
#include "gem/hw/vfat/VFAT2Strings2Enums.h"
//#include "gem/hw/vfat/VFAT2EnumStrings.h"

#include "gem/hw/vfat/exception/Exception.h"


namespace gem {
  namespace hw {
    namespace utils {
      
      class GEMDBtoVFATobj
      {
	
      public:
	
	GEMDBtoVFATobj();
	~GEMDBtoVFATobj();

      protected:
	void getVFATparamfromDB(gem::hw::vfat::VFAT2ControlParams &params, xdata::Table &vfatdb);
	
      };
    }
  }
}
#endif
