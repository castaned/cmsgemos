#include <iomanip>

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"

#include "xdaq/NamespaceURI.h"

#include "gem/hw/vfat/VFAT2Manager.h"
#include "gem/hw/vfat/HwVFAT2.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::vfat::VFAT2Manager);

gem::hw::vfat::VFAT2Manager::VFAT2Manager(xdaq::ApplicationStub* s)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(s)
{
  xgi::framework::deferredbind(this, this, &VFAT2Manager::Default,       "Default"     );
  xgi::framework::deferredbind(this, this, &VFAT2Manager::RegisterView,  "RegisterView");
  xgi::framework::deferredbind(this, this, &VFAT2Manager::ControlPanel,  "ControlPanel");
  xgi::framework::deferredbind(this, this, &VFAT2Manager::ExpertView,    "ExpertView"  );
  xgi::framework::deferredbind(this, this, &VFAT2Manager::Peek,          "Peek"        );
  xgi::framework::deferredbind(this, this, &VFAT2Manager::controlVFAT2,  "controlVFAT2");

  m_device = "VFAT13";
  m_ipAddr = "192.168.0.115";
  m_settingsFile = "";

  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("device", &m_device);
  getApplicationInfoSpace()->fireItemAvailable("ipAddr", &m_ipAddr);
  getApplicationInfoSpace()->fireItemAvailable("settingsFile", &m_settingsFile);

  getApplicationInfoSpace()->fireItemValueRetrieve("device", &m_device);
  getApplicationInfoSpace()->fireItemValueRetrieve("ipAddr", &m_ipAddr);
  getApplicationInfoSpace()->fireItemValueRetrieve("settingsFile", &m_settingsFile);

  //
  // Bind SOAP callback
  //
  xoap::bind(this, &gem::hw::vfat::VFAT2Manager::onMessage, "onMessage", XDAQ_NS_URI);
}

gem::hw::vfat::VFAT2Manager::~VFAT2Manager()
{
  // p_vfatDevice->releaseDevice();
}

void gem::hw::vfat::VFAT2Manager::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
    std::stringstream ss;
    ss << "m_device=[" << m_device.toString() << "]" << std::endl;
    ss << "m_ipAddr=[" << m_ipAddr.toString() << "]" << std::endl;
    ss << "m_settingsFile=[" << m_settingsFile.toString() << "]" << std::endl;
    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::actionPerformed() Default configuration values have been loaded");
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), ss.str());
    // LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::actionPerformed()   --> starting monitoring");
    // monitorP_->startMonitoring();
  }
  // Initialize the HW device, should have picked up the device string from the xml file by now
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2Manager::VFAT2Manager::4 m_device = " << m_device.toString() << std::endl);
  std::stringstream tmpURI;
  tmpURI << "chtcp-2.0://localhost:10203?target=" << m_ipAddr.toString() << ":50001";
  p_vfatDevice = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(m_device.toString(), tmpURI.str(),
                                                            "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
  // p_vfatDevice->connectDevice();

  setLogLevelTo(uhal::Error());
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2Manager::VFAT2Manager::5 m_device = " << m_device.toString() << std::endl);

  // initialize the vfatParameters struct
  // readVFAT2Registers(vfatParams);
  p_vfatDevice->getAllSettings();
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "vfatParams:" << std::endl
                  << p_vfatDevice->getVFAT2Params() << std::endl);
  // readVFAT2Registers();
  m_vfatParams = p_vfatDevice->getVFAT2Params();
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2Manager::VFAT2Manager::6 m_device = " << m_device.toString() << std::endl);

  try {
    if (p_vfatDevice->isHwConnected()) {
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2 device is connected, continuing to read registers");
    } else {
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2 device is not connected");
      return;
    }
  }
  catch (gem::hw::vfat::exception::TransactionError& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), e.what());
    return;
  }
  catch (gem::hw::vfat::exception::InvalidTransaction& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), e.what());
    return;
  }
  // if (p_vfatDevice->isHwConnected())
  //   LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2 device is connected, continuing to read registers");
  // else {
  //   LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2 device is not connected");
  // }
}

void gem::hw::vfat::VFAT2Manager::readVFAT2Registers(gem::hw::vfat::VFAT2ControlParams& params)
{
  boost::format vfatregform("VFATS.%1%.%2%");

  // try the hardware connection
  try {
    if (p_vfatDevice->isHwConnected()) {
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2 device is connected, continuing to read registers");
    } else {
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), "VFAT2 device is not connected");
      return;
    }
  } catch (gem::hw::vfat::exception::TransactionError& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), e.what());
    return;
  } catch (gem::hw::vfat::exception::InvalidTransaction& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), e.what());
    return;
  }

  try {
    uhal::HwInterface hw = p_vfatDevice->getVFAT2HwInterface();
    std::string deviceBaseNode = p_vfatDevice->getDeviceBaseNode();
    // this conflicts with setting the base node in the HwVFAT device, need to reset
    //  p_vfatDevice->setDeviceBaseNode("");
    // this doesn't fix the problem as HwVFATw or GEMHwDevice needs to be smarter
    // better to strip away the search string from these node names
    //  std::vector<std::string> m_nodes = hw.getNodes(deviceBaseNode+".*");
    m_nodes = hw.getNodes(deviceBaseNode+".*");
    m_nodes.erase( std::remove(m_nodes.begin(),
                              m_nodes.end(),
                              deviceBaseNode),
                  m_nodes.end());

    std::vector<std::string>::iterator cleannode  = m_nodes.begin();
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "before removal::getNodes(): " << std::endl
                    << "m_nodes.size() = " << m_nodes.size() << std::endl);
    std::copy(m_nodes.begin(), m_nodes.end(), std::ostream_iterator<std::string>(std::cout, ", "));

    for (; cleannode != m_nodes.end(); ++cleannode) {
      cleannode->erase(cleannode->find(deviceBaseNode+"."),
                       std::string(deviceBaseNode+".").length());
    }

    std::vector<std::string>::const_iterator node = m_nodes.begin();
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "after removal::getNodes(): " << std::endl
                    << "m_nodes.size() = " << m_nodes.size() << std::endl);
    std::copy(m_nodes.begin(), m_nodes.end(), std::ostream_iterator<std::string>(std::cout, ", "));

    node = m_nodes.begin();
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "attempting to read the vfat registers");
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "start reading::getNodes(): " << std::endl
                    << "m_nodes.size() = " << m_nodes.size() << std::endl);

    for (; node != m_nodes.end(); ++node) {
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), "reading::getNodes(): " << std::endl
                      << "m_nodes.size() = " << m_nodes.size() << std::endl);
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), "attempting to read register " << (*node));
      m_vfatFullRegs[*node] = p_vfatDevice->readReg(p_vfatDevice->getDeviceBaseNode(), *node);
      m_vfatRegs[*node]     = p_vfatDevice->readVFATReg(*node);
      std::cout << *node << "  0x" << std::hex << m_vfatFullRegs[*node] << std::dec << std::endl;
      std::cout << *node << "  0x" << std::hex << static_cast<unsigned>(m_vfatRegs[*node])     << std::dec << std::endl;
    }
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "done reading::getNodes(): " << std::endl
                    << "m_nodes.size() = " << m_nodes.size() << std::endl);

    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::readVFAT2Registers() --> read in current parameters");
    // std::string key = boost::str(vfatregform % m_device.toString() % "Latency");
    // key = "Latency";
    params.latency = static_cast<unsigned>(m_vfatRegs["Latency"]);

    params.iPreampIn   = static_cast<unsigned>(m_vfatRegs["IPreampIn"]);
    params.iPreampFeed = static_cast<unsigned>(m_vfatRegs["IPreampFeed"]);
    params.iPreampOut  = static_cast<unsigned>(m_vfatRegs["IPreampOut"]);
    params.iShaper     = static_cast<unsigned>(m_vfatRegs["IShaper"]);
    params.iShaperFeed = static_cast<unsigned>(m_vfatRegs["IShaperFeed"]);
    params.iComp       = static_cast<unsigned>(m_vfatRegs["IComp"]);

    params.vCal     = static_cast<unsigned>(m_vfatRegs["VCal"]);
    params.vThresh1 = static_cast<unsigned>(m_vfatRegs["VThreshold1"]);
    params.vThresh2 = static_cast<unsigned>(m_vfatRegs["VThreshold2"]);
    params.calPhase = static_cast<unsigned>(m_vfatRegs["CalPhase"]);

    // counters

    // uint16_t myChipID;
    // myChipID = (m_vfatRegs["ChipID1"]<<8)|m_vfatRegs["ChipID0"];
    params.chipID = (m_vfatRegs["ChipID1"] << 8)|m_vfatRegs["ChipID0"];
    std::cout << "ChipID0 = 0x" << std::hex << static_cast<unsigned>(m_vfatRegs["ChipID0"]) << std::dec << std::endl;
    std::cout << "ChipID1 = 0x" << std::hex << static_cast<unsigned>(m_vfatRegs["ChipID1"]) << std::dec << std::endl;
    std::cout << "ChipID = 0x" << std::hex << params.chipID << std::dec << std::endl;

    // uint8_t myUpset;
    params.upsetCounter = static_cast<unsigned>(m_vfatRegs["UpsetReg"]);

    // uint32_t myCounter;
    params.hitCounter = (m_vfatRegs["HitCount2"] << 16)|
      (m_vfatRegs["HitCount1"] << 8)|
      m_vfatRegs["HitCount0"];

    // uint8_t control0, control1, control2, control3;
    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::readVFAT2Registers() --> read in control registers");
    params.control0 = static_cast<unsigned>(m_vfatRegs["ContReg0"]);
    params.control1 = static_cast<unsigned>(m_vfatRegs["ContReg1"]);
    params.control2 = static_cast<unsigned>(m_vfatRegs["ContReg2"]);
    params.control3 = static_cast<unsigned>(m_vfatRegs["ContReg3"]);

    params.runMode   = (VFAT2RunMode  )((params.control0&VFAT2ContRegBitMasks::RUNMODE ) >> VFAT2ContRegBitShifts::RUNMODE );
    params.trigMode  = (VFAT2TrigMode )((params.control0&VFAT2ContRegBitMasks::TRIGMODE) >> VFAT2ContRegBitShifts::TRIGMODE);
    params.msPol     = (VFAT2MSPol    )((params.control0&VFAT2ContRegBitMasks::MSPOL   ) >> VFAT2ContRegBitShifts::MSPOL   );
    params.calPol    = (VFAT2CalPol   )((params.control0&VFAT2ContRegBitMasks::CALPOL  ) >> VFAT2ContRegBitShifts::CALPOL  );
    params.calibMode = (VFAT2CalibMode)((params.control0&VFAT2ContRegBitMasks::CALMODE ) >> VFAT2ContRegBitShifts::CALMODE );

    params.dacMode   = (VFAT2DACMode  )((params.control1&VFAT2ContRegBitMasks::DACMODE  ) >> VFAT2ContRegBitShifts::DACMODE  );
    params.probeMode = (VFAT2ProbeMode)((params.control1&VFAT2ContRegBitMasks::PROBEMODE) >> VFAT2ContRegBitShifts::PROBEMODE);
    params.lvdsMode  = (VFAT2LVDSMode )((params.control1&VFAT2ContRegBitMasks::LVDSMODE ) >> VFAT2ContRegBitShifts::LVDSMODE );
    params.reHitCT   = (VFAT2ReHitCT  )((params.control1&VFAT2ContRegBitMasks::REHITCT  ) >> VFAT2ContRegBitShifts::REHITCT  );

    params.hitCountMode = (VFAT2HitCountMode )((params.control2&VFAT2ContRegBitMasks::HITCOUNTMODE ) >> VFAT2ContRegBitShifts::HITCOUNTMODE );
    params.msPulseLen   = (VFAT2MSPulseLength)((params.control2&VFAT2ContRegBitMasks::MSPULSELENGTH) >> VFAT2ContRegBitShifts::MSPULSELENGTH);
    params.digInSel     = (VFAT2DigInSel     )((params.control2&VFAT2ContRegBitMasks::DIGINSEL     ) >> VFAT2ContRegBitShifts::DIGINSEL     );

    params.trimDACRange    = (VFAT2TrimDACRange )((params.control3&VFAT2ContRegBitMasks::TRIMDACRANGE) >> VFAT2ContRegBitShifts::TRIMDACRANGE);
    params.padBandGap      = (VFAT2PadBandgap   )((params.control3&VFAT2ContRegBitMasks::PADBANDGAP  ) >> VFAT2ContRegBitShifts::PADBANDGAP  );
    params.sendTestPattern = (VFAT2DFTestPattern)((params.control3&VFAT2ContRegBitMasks::DFTESTMODE  ) >> VFAT2ContRegBitShifts::DFTESTMODE  );

    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::readVFAT2Registers() --> read in current parameters");
    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::readVFAT2Registers()::Control Register 0");
    LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s:%s:%s>")
                                                       % static_cast<unsigned>(params.control0)
                                                       % gem::hw::vfat::RunModeToString.at(        params.runMode  )
                                                       % gem::hw::vfat::TriggerModeToString.at(    params.trigMode )
                                                       % gem::hw::vfat::MSPolarityToString.at(     params.msPol    )
                                                       % gem::hw::vfat::CalPolarityToString.at(    params.calPol   )
                                                       % gem::hw::vfat::CalibrationModeToString.at(params.calibMode)));

    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::readVFAT2Registers()::Control Register 1");
    LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s:%s>")
                                                       % static_cast<unsigned>(params.control1)
                                                       % gem::hw::vfat::DACModeToString.at(      params.dacMode   )
                                                       % gem::hw::vfat::ProbeModeToString.at(    params.probeMode )
                                                       % gem::hw::vfat::LVDSPowerSaveToString.at(params.lvdsMode  )
                                                       % gem::hw::vfat::ReHitCTToString.at(      params.reHitCT   )));

    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::readVFAT2Registers()::Control Register 2");
    LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s>")
                                                       %static_cast<unsigned>(params.control2)
                                                       % gem::hw::vfat::HitCountModeToString.at( params.hitCountMode)
                                                       % gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen  )
                                                       % gem::hw::vfat::DigInSelToString.at(     params.digInSel    )));

    LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2Manager::readVFAT2Registers()::Control Register 3");
    LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s>")
                                                       % static_cast<unsigned>(params.control3)
                                                       % gem::hw::vfat::TrimDACRangeToString.at( params.trimDACRange    )
                                                       % gem::hw::vfat::PbBGToString.at(         params.padBandGap      )
                                                       % gem::hw::vfat::DFTestPatternToString.at(params.sendTestPattern )));
  }
  catch (uhal::exception::exception const& err) {
    std::string msg =
      toolbox::toString("unable to access VFAT2 hardware %s", (m_device.toString()).c_str());
    // remove fatals//LOG4CPLUS_FATAL(this->getApplicationLogger(), msg);
    XCEPT_RAISE(gem::hw::vfat::exception::VFATHwProblem, msg);
  }
  catch (std::exception const& err) {
    std::string msg =
      toolbox::toString("unable to access VFAT2 hardware %s", (m_device.toString()).c_str());
    // remove fatals//LOG4CPLUS_FATAL(this->getApplicationLogger(), msg);
    XCEPT_RAISE(gem::hw::vfat::exception::VFATHwProblem, msg);
  }
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "ending constructor::getNodes(): " << std::endl
                  << "m_nodes.size() = " << m_nodes.size() << std::endl);
}

void gem::hw::vfat::VFAT2Manager::Default(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), p_vfatDevice->printErrorCounts());
  this->ControlPanel(in, out);
  // this->RegisterView(in, out);
}

void gem::hw::vfat::VFAT2Manager::ControlPanel(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  try {
    // need to grab the page header and modify it here somehow...
    // gem::hw::vfat::VFAT2Manager::createHeader(out);
    // *out << "  <body>" << std::endl;
    // cgicc::HTTPResponseHeader& head = out->getHTTPResponseHeader();
    // head->addHeader();

    // head = out["head"]
    *out << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatmanager.css\"/>"          << std::endl
         << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcontrolregisters.css\"/>" << std::endl
         << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatglobalsettings.css\"/>"   << std::endl
         << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatchannelregister.css\"/>"  << std::endl
         << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcommands.css\"/>"         << std::endl;


    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "building the control panel m_vfatParams:" << m_vfatParams);
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "p_vfatDevice->getVFAT2Params():" << p_vfatDevice->getVFAT2Params());
    std::string method = toolbox::toString("/%s/controlVFAT2", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method", "POST")
      .set("action", method)
      // .set("action", "")
      // .set("onsubmit", "return false;")
         << std::endl;
    // .set("onsubmit", "return false;")
    //      << cgicc::br() << std::endl;
    // .set("action", method) << "</br>" << std::endl;
    // *out << "    <form>" << std::endl;
    /*
      cgicc::section *leftSide = new cgicc::section();
      leftSide->set("style", "display:inline-block;float:left");
      cgicc::fieldset *vfatParametersFieldset = new cgicc::fieldset();
      vfatParametersFieldset->set("class", "vfatParameters");
      vfatParametersFieldset->add(cgicc::legend("VFAT2 Parameters"));
      vfatParametersFieldset->add(cgicc::span().set("style", "display:block;float:left"));
      leftSide->add(vfatParametersFieldset);
      *out << *leftSide << std::endl;
      */
    *out << cgicc::section().set("style", "display:inline-block;float:left")       << std::endl
         << cgicc::fieldset().set("class", "vfatParameters")       << std::endl
         << cgicc::legend("VFAT2 Parameters")                     << std::endl
         << cgicc::span().set("style", "display:block;float:left") << std::endl;

    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "building the VFATInfoLayout");
    gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createVFATInfoLayout(out, m_vfatParams);
    *out << cgicc::span()     << std::endl;

    *out << std::endl;
    *out << cgicc::span().set("style", "display:block;float:right") << std::endl;
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "building the CounterLayout");
    gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createCounterLayout(out, m_vfatParams);
    *out << cgicc::span()     << std::endl;
    *out << cgicc::comment() << "ending the VFAT2 Parameters fieldset" << cgicc::comment() << std::endl
         << cgicc::fieldset() << std::endl;
    *out << std::endl;
    *out << "      <section class=\"vfatSettings\">" << std::endl;
    *out << "        <fieldset class=\"vfatGlobalSettings\">" << std::endl
         << cgicc::legend("Global settings")      << std::endl;
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "building the ControlRegisterLayout");
    gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createControlRegisterLayout(out, m_vfatParams);
    *out << cgicc::br() << std::endl
         << std::endl;
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "building the SettingsLayout");
    gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createSettingsLayout(out, m_vfatParams);

    *out << cgicc::comment() << "ending the Global settings fieldset" << cgicc::comment() << std::endl
         << "        </fieldset>" << std::endl;

    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "building the ChannelRegisterLayout");
    gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createChannelRegisterLayout(out, m_vfatParams);
    *out << cgicc::comment() << "ending the vfatSettings section" << cgicc::comment() << std::endl
         << "      </section>" << std::endl

         << cgicc::comment() << "ending the left side section" << cgicc::comment() << std::endl
      // << "    </section>" << std::endl;
         << cgicc::section()  << std::endl;


    /*
      cgicc::section *rightSide = new cgicc::section();
      rightSide->set("class", "vfatCommands");
    */
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "building the CommandLayout");
    gem::hw::vfat::VFAT2Manager::VFAT2ControlPanelWeb::createCommandLayout(out, m_vfatParams);

    *out << cgicc::section() << std::endl
         // << "Errors     :: " << (p_vfatDevice->vfatErrors_).Error        << cgicc::br() << std::endl
         // << "Invalid    :: " << (p_vfatDevice->vfatErrors_).Invalid      << cgicc::br() << std::endl
         // << "RWMismatch :: " << (p_vfatDevice->vfatErrors_).RWMismatch   << cgicc::br() << std::endl
         << "Bad headers:: " << (p_vfatDevice->m_ipBusErrs).BadHeader     << cgicc::br() << std::endl
         << "Read errors:: " << (p_vfatDevice->m_ipBusErrs).ReadError     << cgicc::br() << std::endl
         << "Timeouts   :: " << (p_vfatDevice->m_ipBusErrs).Timeout       << cgicc::br() << std::endl
         << "CH errors  :: " << (p_vfatDevice->m_ipBusErrs).ControlHubErr << cgicc::br() << std::endl
         << cgicc::section() << std::endl;

    *out << cgicc::form() << cgicc::br() << std::endl;
    *out << cgicc::script().set("type", "text/javascript")
      .set("src", "http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
         << cgicc::script() << std::endl;
    *out << cgicc::script().set("type", "text/javascript")
      .set("src", "http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
         << cgicc::script() << std::endl;

    /*
     *out << cgicc::script().set("type", "text/javascript")
     .set("src", "gemdaq/gemhardware/html/scripts/loadVFATSettings.js")
     << cgicc::script() << cgicc::br()
     << std::endl;
    */
    *out << cgicc::script().set("type", "text/javascript")
      .set("src", "/gemdaq/gemhardware/html/scripts/toggleVFATCheckboxes.js")
         << cgicc::script() << cgicc::br()
         << std::endl;

    /*
    **load here all the scripts to jquery/ajax the forms/values/files**
    *out << "    <script type=\"text/javascript\">" << std::endl
    << "      $(document).ready(function()"    << std::endl
    << "        {" << std::endl
    << "          alert(\"JQuery is working! :D\");" << std::endl
    << "        });"   << std::endl
    << "    </script>" << std::endl;
    *out << std::endl;
    *out << std::endl;
    *out << "    <script type=\"text/javascript\">" << std::endl
    << "    $(document).ready( function () {"  << std::endl
    << "    	var registerSetBoxes = [$(\"#CR0Set\"),$(\"#CR1Set\"),$(\"#CR2Set\"),$(\"#CR3Set\"), "       << std::endl
    << "    				$(\"#SetIPreampIn\"),$(\"#SetIPreampFeed\"),$(\"#SetIPreampOut\"), " << std::endl
    << "    				$(\"#SetIShaper\"),$(\"#SetIShaperFeed\"), "                         << std::endl
    << "    				$(\"#SetIComp\"), "   << std::endl
    << "    				$(\"#SetLatency\"), " << std::endl
    << "    				$(\"#SetVCal\"), "    << std::endl
    << "    				$(\"#SetVThreshold1\"),$(\"#SetVThreshold2\"), " << std::endl
    << "    				$(\"#SetCalPhase\")" << std::endl
    << "    				];"                  << std::endl
    << "    	var nBoxes = registerSetBoxes.length;"       << std::endl
    << "    	$(\"#SelectAll\").click(function(event) {"   << std::endl
    << "                   alert(\"Select all checkboxes\");"   << std::endl
    << "    		for (var i=0; i<nBoxes; ++i ) {"     << std::endl
    << "    		    registerSetBoxes[i].get(0).checked = true;" << std::endl
    << "    		};" << std::endl
    << "    	    });"    << std::endl
    << "    	$(\"#SelectNone\").click(function(event) {"  << std::endl
    << "                   alert(\"Deselect all checkboxes\");" << std::endl
    << "    		for (var i=0; i<nBoxes; ++i ) {"     << std::endl
    << "    		    registerSetBoxes[i].get(0).checked = false;" << std::endl
    << "    		};" << std::endl
    << "    	    });"    << std::endl
    << "        });"           << std::endl
    << "    </script>"         << std::endl;
    *out << std::endl;
    */
    // *out << "  </body>" << std::endl;
    // *out << "</html>" << std::endl;
  }

  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Something went wrong displaying ControlPanel xgi: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Something went wrong displaying the ControlPanel: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::hw::vfat::VFAT2Manager::ExpertView(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
}


void gem::hw::vfat::VFAT2Manager::RegisterView(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  // std::string method = toolbox::toString("/%s/writeUserRegs",getApplicationDescriptor()->getURN().c_str());

  // try the cgicc display
  try {
    *out << "<div class=\"xdaq-tab-wrapper\">"                      << std::endl;

    *out << "  <div class=\"xdaq-tab\" title=\"VFAT2 Registers\" >" << std::endl;

    *out << cgicc::fieldset().set("style", "font-size: 10pt; font-family: arial;") << std::endl;
    // *out << cgicc::legend(        "Read/Write VFAT user registers"              ) << cgicc::p() << std::endl;
    // *out << cgicc::form().set(    "method", "GET").set("action", method)           << "<br />" << std::endl;
    *out << "    <table class=\"xdaq-table\" caption=\"VFAT2 Registers\" >"       << std::endl;
    *out << "      <thead>" << std::endl;
    *out << "        <tr>"  << std::endl;
    *out << "          <th class=\"xdaq-sortable\">VFAT2 Register</th>"  << std::endl;
    *out << "          <th class=\"xdaq-sortable\">Read IPBus data</th>"      << std::endl;
    *out << "          <th class=\"xdaq-sortable\">Read VFAT data</th>"      << std::endl;
    // *out << "          <th class=\"xdaq-sortable\">Value to write</th>"  << std::endl;
    // *out << "          <th class=\"xdaq-sortable\">Execute</th>"         << std::endl;
    *out << "        </tr>"  << std::endl;
    *out << "      </thead>" << std::endl;
    *out << "      <br />"    << std::endl;

    // set up the rest by way of a loop
    *out << "      <tbody>"             << std::endl;

    std::vector<std::string>::const_iterator node  = m_nodes.begin();
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "m_nodes.size() = " << m_nodes.size());
    for (; node != m_nodes.end(); ++node) {
      *out << "      <tr>"     << std::endl;
      *out << "        <td>"   << *node << "</td>"  << std::endl;
      *out << "        <td>0x" << std::setfill('0') << std::setw(8)
           << std::hex << m_vfatFullRegs[*node]
           << std::dec << "</td>"                   << std::endl;
      *out << "        <td>0x" << std::setfill('0') << std::setw(2)
           << std::hex << m_vfatRegs[*node]
           << std::dec << "</td>"                   << std::endl;
      // *out << "        <td>" << cgicc::input().set("type", "text"
      //                                              ).set("name",boost::str(uregform%ureg)
      //                                                    ).set("value", boost::str(boost::format("0x%08x")%xreg_users_.at(ureg))
      //                                                          ).set("size", "10").set("maxlength", "32")
      //      << "        </td>" << std::endl;
      *out << "      </tr>" << std::endl;
    }


    *out << "      <br />"    << std::endl;
    *out << "      </tbody>" << std::endl;
    *out << "    </table>"   << std::endl;
    *out << "    <br />"      << std::endl;
    *out << "  </div>"       << std::endl;
    *out << "</div>"         << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Something went wrong displaying xgi: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Something went wrong displaying the registers: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::hw::vfat::VFAT2Manager::Peek(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  // default peek
}

void gem::hw::vfat::VFAT2Manager::controlVFAT2(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  try {
    cgicc::Cgicc cgi(in);
    // std::vector<cgicc::FormEntry> myElements = cgi.getElements();
    // for (unsigned int formiter = 0; formiter < myElements.size(); ++formiter) {
    //  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "form element name: "  + myElements.at(formiter).getName());
    //  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "form element value: " + myElements.at(formiter).getValue());
    // }
    // read a list of the checked checkboxes
    std::vector<std::pair<std::string, uint8_t> > regValsToSet;
    getCheckedRegisters(cgi, regValsToSet);

    //now process the form based on the button clicked
    performAction(cgi, regValsToSet);
    this->Default(in, out);
  } catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::hw::vfat::VFAT2Manager::getCheckedRegisters(cgicc::Cgicc cgi, std::vector<std::pair<std::string, uint8_t> > &regValsToSet)
  throw (xgi::exception::Exception)
{
  /****
   ***get a list of the following registers to set***

   *Address table register - checkbox name - corresponding form inputs to grab

   ContReg0 - CR0Set - CalMode:CalPolarity:MSPolarity:TriggerMode:RunMode
   ContReg1 - CR1Set - ReHitCT:LVDSPowerSave:ProbeMode:DACMode
   ContReg2 - CR2Set - DigInSel:MSPulseLength:HitCountMode
   ContReg3 - CR3Set - DFTest:PbBG:TrimDACRange

   IPreampIn   - SetIPreampIn    - IPreampIn
   IPreampFeed - SetIPreampFeed  - IPreampFeed
   IPreampOut  - SetIPreampOut   - IPreampOut
   IShaper     - SetIShaper      - IShaper
   IShpaerFeed - SetIShpaerFeed  - IShpaerFeed
   IComp       - SetIComp        - IComp

   Latency     - SetLatency      - Latency
   VCal        - SetVCal         - VCal
   VThreshold1 - SetVThreshold1  - VThreshold1
   VThreshold2 - SetVThreshold2  - VThreshold2

   CalPhase - SetCalPhase - CalPhase

   //if all selected, pass the params struct to the setter?

   ***Readonly***
   ChipID0
   ChipID1

   UpsetReg

   HitCount0
   HitCount1
   HitCount2
  **/

  // std::vector<std::pair<std::string, uint8_t> > regValsToSet;
  std::pair<std::string, uint8_t> regPair;
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "getCheckedRegisters::Getting list of checked registers and values to read/write");
  try
    {
      //cgicc::Cgicc cgi(in);
      std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();

      if (cgi.queryCheckbox("CR0Set")) {
        uint8_t regToSet = p_vfatDevice->getVFAT2Params().control0;
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the run mode to :"+
                        boost::to_upper_copy(cgi["RunMode"]->getValue()));
        p_vfatDevice->setRunMode(        (gem::hw::vfat::StringToRunMode.at(boost::to_upper_copy(cgi["RunMode"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the trigger mode to :"+
                        boost::to_upper_copy(cgi["TriggerMode"]->getValue()));
        p_vfatDevice->setTriggerMode(    (gem::hw::vfat::StringToTriggerMode.at(boost::to_upper_copy(cgi["TriggerMode"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the monostable pulse polarity to :"+
                        boost::to_upper_copy(cgi["MSPolarity"]->getValue()));
        p_vfatDevice->setMSPolarity(     (gem::hw::vfat::StringToMSPolarity.at(boost::to_upper_copy(cgi["MSPolarity"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the calibration pulse polarity to :"+
                        boost::to_upper_copy(cgi["CalPolarity"]->getValue()));
        p_vfatDevice->setCalPolarity(    (gem::hw::vfat::StringToCalPolarity.at(boost::to_upper_copy(cgi["CalPolarity"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting calibration mode to :"+
                        boost::to_upper_copy(cgi["CalMode"]->getValue()));
        p_vfatDevice->setCalibrationMode((gem::hw::vfat::StringToCalibrationMode.at(boost::to_upper_copy(cgi["CalMode"]->getValue()))), regToSet);

        LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
        regValsToSet.push_back(std::make_pair("ContReg0", regToSet));
      }

      if (cgi.queryCheckbox("CR1Set")) {
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting control register 1");
        uint8_t regToSet = p_vfatDevice->getVFAT2Params().control1;
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the probe mode to :"+
                        boost::to_upper_copy(cgi["ProbeMode"]->getValue()));
        p_vfatDevice->setProbeMode(        (gem::hw::vfat::StringToProbeMode.at(boost::to_upper_copy(cgi["ProbeMode"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the lvds power save state to :"+
                        boost::to_upper_copy(cgi["LVDSPowerSave"]->getValue()));
        p_vfatDevice->setLVDSMode(         (gem::hw::vfat::StringToLVDSPowerSave.at(boost::to_upper_copy(cgi["LVDSPowerSave"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the hit count cycle time to :"+
                        boost::to_upper_copy(cgi["ReHitCT"]->getValue()));
        p_vfatDevice->setHitCountCycleTime((gem::hw::vfat::StringToReHitCT.at(boost::to_upper_copy(cgi["ReHitCT"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the DAC mode to :"+
                        boost::to_upper_copy(cgi["DACMode"]->getValue()));
        LOG4CPLUS_DEBUG(getApplicationLogger(), boost::to_upper_copy(cgi["DACMode"]->getValue()));
        p_vfatDevice->setDACMode(          (gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(cgi["DACMode"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
        regValsToSet.push_back(std::make_pair("ContReg1", regToSet));
      }

      if (cgi.queryCheckbox("CR2Set")) {
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting control register 2");
        uint8_t regToSet = p_vfatDevice->getVFAT2Params().control2;
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the monostable pulse length to :"+
                        boost::to_upper_copy(cgi["MSPulseLength"]->getValue()));
        p_vfatDevice->setMSPulseLength((gem::hw::vfat::StringToMSPulseLength.at(boost::to_upper_copy(cgi["MSPulseLength"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the digital input pad to :"+
                        boost::to_upper_copy(cgi["DigInSel"]->getValue()));
        p_vfatDevice->setInputPadMode( (gem::hw::vfat::StringToDigInSel.at(boost::to_upper_copy(cgi["DigInSel"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the hit count mode to :"+
                        boost::to_upper_copy(cgi["HitCountMode"]->getValue()));
        p_vfatDevice->setHitCountMode( (gem::hw::vfat::StringToHitCountMode.at(boost::to_upper_copy(cgi["HitCountMode"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
        regValsToSet.push_back(std::make_pair("ContReg2", regToSet));
      }

      if (cgi.queryCheckbox("CR3Set")) {
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting control register 3");
        uint8_t regToSet = p_vfatDevice->getVFAT2Params().control3;
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the trim dac range to :"+
                        boost::to_upper_copy(cgi["TrimDACRange"]->getValue()));
        p_vfatDevice->setTrimDACRange((gem::hw::vfat::StringToTrimDACRange.at(boost::to_upper_copy(cgi["TrimDACRange"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the pad band gap to :"+
                        boost::to_upper_copy(cgi["PbBG"]->getValue()));
        p_vfatDevice->setBandgapPad(  (gem::hw::vfat::StringToPbBG.at(boost::to_upper_copy(cgi["PbBG"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), "Setting the test pattern mode to :"+
                        boost::to_upper_copy(cgi["DFTest"]->getValue()));
        p_vfatDevice->sendTestPattern((gem::hw::vfat::StringToDFTestPattern.at(boost::to_upper_copy(cgi["DFTest"]->getValue()))), regToSet);
        LOG4CPLUS_DEBUG(getApplicationLogger(), boost::str(boost::format("with value 0x%02x")%(unsigned)regToSet));
        regValsToSet.push_back(std::make_pair("ContReg3", regToSet));
      }

      if (cgi.queryCheckbox("SetIPreampIn"))
        regValsToSet.push_back(std::make_pair("IPreampIn"   , cgi["IPreampIn"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIPreampFeed"))
        regValsToSet.push_back(std::make_pair("IPreampFeed" , cgi["IPreampFeed"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIPreampOut"))
        regValsToSet.push_back(std::make_pair("IPreampOut"  , cgi["IPreampOut"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIShaper"))
        regValsToSet.push_back(std::make_pair("IShaper"     , cgi["IShaper"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIShaperFeed"))
        regValsToSet.push_back(std::make_pair("IShaperFeed" , cgi["IShaperFeed"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetIComp"))
        regValsToSet.push_back(std::make_pair("IComp"       , cgi["IComp"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetLatency"))
        regValsToSet.push_back(std::make_pair("Latency"     , cgi["Latency"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetVCal"))
        regValsToSet.push_back(std::make_pair("VCal"        , cgi["VCal"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetVThreshold1"))
        regValsToSet.push_back(std::make_pair("VThreshold1" , cgi["VThreshold1"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetVThreshold2"))
        regValsToSet.push_back(std::make_pair("VThreshold2" , cgi["VThreshold2"]->getIntegerValue()));
      if (cgi.queryCheckbox("SetCalPhase"))
        regValsToSet.push_back(std::make_pair("CalPhase"    , cgi["CalPhase"]->getIntegerValue()));
    }
  /*
    catch (const boost::exception & e)
    {
    XCEPT_RAISE(boost::exception, e);
    }
  */
  catch (const xgi::exception::Exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "getCheckedRegisters::list obtained");
  // return regValsToSet;
}

void gem::hw::vfat::VFAT2Manager::performAction(cgicc::Cgicc cgi, std::vector<std::pair<std::string, uint8_t> > regValsToSet)
  throw (xgi::exception::Exception)
{
  /** possible actions
   *Read Counters (name=ReadCounters
   *
   *Get selected channel (name=GetChannel)
   *Set selected channel (name=SetChannel)
   *Set all channels (name=SetAllChannels)
   *
   *Read all(selected?) registers (name=Read)
   *Write selected registers      (name=Write)
   *Compare written values of selected registers      (name=Compare)
   ** change the font colour if the written value
   ** does not equal the read back value?
   *
   *Load settings from XML file (server or upload?)
   *Write current settings to XML file (server or download?)
   **/
  bool makeComparison = cgi.queryCheckbox("Compare");

  if (makeComparison) {
    /**
     *should we just compare what we want to write with what is currently there,
     *or should we write, and compare that what we wrote matches what we said to write?
     */
  }

  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "performAction::reading options");
  std::string controlOption = "";
  std::string channelOption = "";
  //try {
  //  controlOption = cgi["VFAT2ControlOption"]->getValue();
  //} catch (const xgi::exception::Exception& e) {
  //  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Unable to find VFAT2ControlOption, possibly pressed the channel button " << e.what());
  //  XCEPT_RAISE(xgi::exception::Exception, e.what());
  //} catch (const std::exception& e) {
  //  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Unable to find VFAT2ControlOption, possibly pressed the channel button " << e.what());
  //  XCEPT_RAISE(xgi::exception::Exception, e.what());
  //}
  //
  //try {
  //  channelOption = cgi["VFAT2ChannelOption"]->getValue();
  //} catch (const xgi::exception::Exception& e) {
  //  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Unable to find VFAT2ChannelOption, possibly pressed the control button " << e.what());
  //  XCEPT_RAISE(xgi::exception::Exception, e.what());
  //} catch (const std::exception& e) {
  //  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Unable to find VFAT2ChannelOption, possibly pressed the control button " << e.what());
  //  XCEPT_RAISE(xgi::exception::Exception, e.what());
  //}

  controlOption = cgi["VFAT2ControlOption"]->getValue();
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "performAction::Control option " << controlOption);
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "performAction::Channel option " << channelOption);
  if (strcmp(controlOption.c_str(), "Read counters") == 0) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Read counters button pressed");
    // p_vfatDevice->readVFAT2Counters(p_vfatDevice->getVFAT2Params());
    p_vfatDevice->readVFAT2Counters();
  } else if (strcmp(controlOption.c_str(), "Set Channel 0/1") == 0) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Set channel 1 button pressed");

    if (cgi.queryCheckbox("Cal0"))
      p_vfatDevice->enableCalPulseToChannel(0);
    else
      p_vfatDevice->enableCalPulseToChannel(0, false);
    if (cgi.queryCheckbox("Cal1"))
      p_vfatDevice->enableCalPulseToChannel(1);
    else
      p_vfatDevice->enableCalPulseToChannel(1, false);
    if (cgi.queryCheckbox("Ch1Mask"))
      p_vfatDevice->maskChannel(1);
    else
      p_vfatDevice->maskChannel(1, false);
    if (cgi.queryCheckbox("SetCh1TrimDAC"))
      p_vfatDevice->setChannelTrimDAC(1, cgi["Ch1TrimDAC"]->getIntegerValue());

    p_vfatDevice->readVFAT2Channel(1);
    m_vfatParams = p_vfatDevice->getVFAT2Params();
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "set channel 0/1 - 0x"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[0].fullChannelReg) << std::dec << "::<"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[0].calPulse0     ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[0].calPulse      ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[0].mask          ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[0].trimDAC       ) << std::dec << ">"
                    << std::endl);
  } else if (strcmp(controlOption.c_str(), "Get This Channel") == 0) {
    // shouldn't need to query the device here, just read the properties from the
    // stored values, and ensure the web page displays that channel
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Get channel button pressed");
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    // uint8_t chanSettings = p_vfatDevice->getChannelSettings(chan);

    m_vfatParams = p_vfatDevice->getVFAT2Params();
    m_vfatParams.activeChannel = chan;
    // p_vfatDevice->setActiveChannelWeb(chan);
  } else if (strcmp(controlOption.c_str(), "Set This Channel") == 0) {
    // shouldn't need protection here
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Set this channel button pressed for channel "
                    << (unsigned)chan);
    // p_vfatDevice->getVFAT2Params().channels[chan-1].calPulse  = (cgi.queryCheckbox("ChCal"));
    // p_vfatDevice->getVFAT2Params().channels[chan-1].mask      = (cgi.queryCheckbox("ChMask"));

    if (cgi.queryCheckbox("ChCal"))
      p_vfatDevice->enableCalPulseToChannel(chan);
    else
      p_vfatDevice->enableCalPulseToChannel(chan, false);
    if (cgi.queryCheckbox("ChMask"))
      p_vfatDevice->maskChannel(chan);
    else
      p_vfatDevice->maskChannel(chan, false);
    if (cgi.queryCheckbox("SetTrimDAC"))
      // p_vfatDevice->getVFAT2Params().channels[chan-1].trimDAC = cgi["TrimDAC"]->getIntegerValue();
      p_vfatDevice->setChannelTrimDAC(chan, cgi["TrimDAC"]->getIntegerValue());

    // p_vfatDevice->getVFAT2Params().activeChannel = chan;
    // p_vfatDevice->readVFAT2Channel(p_vfatDevice->getVFAT2Params(), chan);
    p_vfatDevice->readVFAT2Channel(chan);
    m_vfatParams = p_vfatDevice->getVFAT2Params();
    m_vfatParams.activeChannel = chan;
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),
                    "chan = "<< (unsigned)chan << "; activeChannel = " <<(unsigned)m_vfatParams.activeChannel << std::endl);
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "set this channel " << (unsigned)chan << " - 0x"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].fullChannelReg) << std::dec << "::<"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].calPulse0     ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].calPulse      ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].mask          ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].trimDAC       ) << std::dec << ">"
                    << std::endl);
  } else if (strcmp(controlOption.c_str(), "Set All Channels") == 0) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Set all channels button pressed");
    // apply provided settings to all channels (2-128 or 1-128?)
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    int min_chan = 2;
    bool setMasked(false), setCalPulse(false);
    if (cgi.queryCheckbox("ChCal"))
      setCalPulse = true;
    for (int chan = min_chan; chan < 129; ++chan) {
      if (chan == 1)
        p_vfatDevice->enableCalPulseToChannel(chan-1, setCalPulse);
      p_vfatDevice->enableCalPulseToChannel(chan, setCalPulse);
    }
    if (cgi.queryCheckbox("ChMask"))
      setMasked = true;
    for (int chan = min_chan; chan < 129; ++chan)
      p_vfatDevice->maskChannel(chan, setMasked);
    if (cgi.queryCheckbox("SetTrimDAC")) {
      for (int chan = min_chan; chan < 129; ++chan)
        p_vfatDevice->setChannelTrimDAC(chan, cgi["TrimDAC"]->getIntegerValue());
    }
    // p_vfatDevice->readVFAT2Channels(p_vfatDevice->getVFAT2Params());
    p_vfatDevice->readVFAT2Channels();
    m_vfatParams = p_vfatDevice->getVFAT2Params();
    m_vfatParams.activeChannel = chan;
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "set all channels " << (unsigned)chan << " - 0x"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].fullChannelReg) << std::dec << "::<"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].calPulse0     ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].calPulse      ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].mask          ) << std::dec << ":"
                    << std::hex << static_cast<unsigned>(m_vfatParams.channels[chan-1].trimDAC       ) << std::dec << ">"
                    << std::endl);
  } else if (strcmp(controlOption.c_str(), "Read VFAT") == 0) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Read VFAT button pressed with following registers");

    std::vector<std::pair<std::string, uint8_t> >::const_iterator curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg) {
      // std::string msg = toolbox::toString((curReg->first).c_str());
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), curReg->first);
    }
    p_vfatDevice->readVFATRegs(regValsToSet);
    // readVFAT2Registers(p_vfatDevice->getVFAT2Params());
    p_vfatDevice->getAllSettings();
    m_vfatParams = p_vfatDevice->getVFAT2Params();
  } else if (strcmp(controlOption.c_str(), "Write VFAT") == 0) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Write VFAT button pressed with following registers");

    std::vector<std::pair<std::string, uint8_t> >::const_iterator curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg) {
      std::string msg =
        toolbox::toString("%s - 0x%02x", (curReg->first).c_str(), static_cast<unsigned>(curReg->second));
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), msg);
    }

    // do a single transaction
    // p_vfatDevice->writeVFATRegs(regValsToSet);

    curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg)
      p_vfatDevice->writeVFATReg(curReg->first, curReg->second);

    // readVFAT2Registers(p_vfatDevice->getVFAT2Params());
    p_vfatDevice->getAllSettings();
    m_vfatParams = p_vfatDevice->getVFAT2Params();
  }
  // m_vfatParams = p_vfatDevice->getVFAT2Params();
}

xoap::MessageReference gem::hw::vfat::VFAT2Manager::onMessage(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(), "-------------------SOAP Message Received-----------------");

  // reply to caller
  xoap::MessageReference reply         = xoap::createMessage();
  xoap::SOAPEnvelope     envelope      = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName         responseName  = envelope.createName("onMessageResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement  e             = envelope.getBody().addBodyElement(responseName);
  return reply;
}
