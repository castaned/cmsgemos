#include "gem/hw/utils/GEMDBtoVFATobj.h"
#include "toolbox/TimeInterval.h"

#include "xcept/tools.h"
#include "xgi/Utils.h"
#include "xgi/Method.h"
#include "xdaq/WebApplication.h"
#include "cgicc/HTMLClasses.h"


#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>


gem::hw::utils::GEMDBtoVFATobj::GEMDBtoVFATobj()
{
}

gem::hw::utils::GEMDBtoVFATobj::~GEMDBtoVFATobj()
{
}


void gem::hw::utils::GEMDBtoVFATobj::getVFATparamfromDB(gem::hw::vfat::VFAT2ControlParams &VFAT2Params, xdata::Table &VFATDB){ 


  INFO("Enter getVFATParamfromDB");
  
  VFAT2Params.trigMode  = gem::hw::vfat::StringToTriggerMode.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR0_TRGMODE")->toString()));
  VFAT2Params.msPol     =  gem::hw::vfat::StringToMSPolarity.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR0_MSPOLARITY")->toString()));
  VFAT2Params.calPol    = gem::hw::vfat::StringToCalPolarity.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR0_CALPOLARITY")->toString()));
  VFAT2Params.calibMode = gem::hw::vfat::StringToCalibrationMode.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR0_CALMODE")->toString()));
  
  VFAT2Params.dacMode    = gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR1_DACSELECT")->toString()));
  VFAT2Params.probeMode  = gem::hw::vfat::StringToProbeMode.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR1_PROBEMODE")->toString()));
  VFAT2Params.lvdsMode   = gem::hw::vfat::StringToLVDSPowerSave.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR1_LVDSPWRSAV")->toString()));
  VFAT2Params.reHitCT    = gem::hw::vfat::StringToReHitCT.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR1_REHITCT")->toString()));
  
  VFAT2Params.hitCountMode = gem::hw::vfat::StringToHitCountMode.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR2_HITCNTSEL")->toString()));
  VFAT2Params.msPulseLen   = gem::hw::vfat::StringToMSPulseLength.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR2_MSPLSLEN")->toString()));
  VFAT2Params.digInSel     = gem::hw::vfat::StringToDigInSel.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR2_DIGINSEL")->toString()));
  
  VFAT2Params.trimDACRange = gem::hw::vfat::StringToTrimDACRange.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR3_TRIMDACRNG")->toString()));
  VFAT2Params.padBandGap   = gem::hw::vfat::StringToPbBG.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR3_PB_BNDGAP")->toString()));
  VFAT2Params.sendTestPattern = gem::hw::vfat::StringToDFTestPattern.at(boost::to_upper_copy(VFATDB.getValueAt(0,"CR3_DFTST")->toString()));

  INFO("Finishing the CR0...CR3  ");

  
  //====== Convertion from xdata::Serializable to int then uint8_t
  unsigned long rowIndex=0;
  xdata::Serializable* xds_BIASIPREAMPIN   = VFATDB.getValueAt(rowIndex,"BIAS_IPREAMPIN");
  xdata::Serializable* xds_BIASIPREAMPFEED = VFATDB.getValueAt(rowIndex,"BIAS_IPREAMPFEED");
  xdata::Serializable* xds_BIASIPREAMPOUT  = VFATDB.getValueAt(rowIndex,"BIAS_IPREAMPOUT");
  xdata::Serializable* xds_BIASISHAPER     = VFATDB.getValueAt(rowIndex,"BIAS_ISHAPER");
  xdata::Serializable* xds_BIASICOMP       = VFATDB.getValueAt(rowIndex,"BIAS_ICOMP");
  xdata::Serializable* xds_BIASLATENCY     = VFATDB.getValueAt(rowIndex,"BIAS_LATENCY");
  xdata::Serializable* xds_BIASVTHRESHOLD1 = VFATDB.getValueAt(rowIndex,"BIAS_VTHRESHOLD1");
  xdata::Serializable* xds_BIASVTHRESHOLD2 = VFATDB.getValueAt(rowIndex,"BIAS_1VTHRESHOLD2");
  
  VFAT2Params.latency = static_cast<uint8_t>(std::stoi(xds_BIASLATENCY->toString()));
  VFAT2Params.iPreampIn = static_cast<uint8_t>(std::stoi(xds_BIASIPREAMPIN->toString()));
  VFAT2Params.iPreampFeed = static_cast<uint8_t>(std::stoi(xds_BIASIPREAMPFEED->toString()));
  VFAT2Params.iPreampOut = static_cast<uint8_t>(std::stoi(xds_BIASIPREAMPOUT->toString()));
  VFAT2Params.iShaper = static_cast<uint8_t>(std::stoi(xds_BIASISHAPER->toString()));
  VFAT2Params.iComp = static_cast<uint8_t>(std::stoi(xds_BIASICOMP->toString()));
  
  VFAT2Params.vThresh1 = static_cast<uint8_t>(std::stoi(xds_BIASVTHRESHOLD1->toString()));
  VFAT2Params.vThresh2 = static_cast<uint8_t>(std::stoi(xds_BIASVTHRESHOLD2->toString()));

}
