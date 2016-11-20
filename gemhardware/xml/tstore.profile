<?xml version='1.0'?>
<!-- Order of specification will determine the sequence of installation. all modules are loaded prior instantiation of plugins -->
<xp:Profile xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" xmlns:xp="http://xdaq.web.cern.ch/xdaq/xsd/2005/XMLProfile-11">
  <!-- Compulsory  Plugins -->
	<xp:Application class="executive::Application" id="0" group="startup" service="executive" network="local">
	  <properties xmlns="urn:xdaq-application:Executive" xsi:type="soapenc:Struct">
		  <logUrl xsi:type="xsd:string">console</logUrl>
                	<logLevel xsi:type="xsd:string">INFO</logLevel>
                </properties>
	</xp:Application>
	<xp:Module>${XDAQ_ROOT}/lib/libb2innub.so</xp:Module>
	<xp:Module>${XDAQ_ROOT}/lib/libexecutive.so</xp:Module>
	
	<xp:Application class="pt::http::PeerTransportHTTP" id="1" group="startup" network="local">
	  <properties xmlns="urn:xdaq-application:PeerTransportHTTP" xsi:type="soapenc:Struct">
                   <aliasName xsi:type="xsd:string">tmp</aliasName>
                        <aliasPath xsi:type="xsd:string">/tmp</aliasPath>
			<documentRoot xsi:type="xsd:string">${XDAQ_DOCUMENT_ROOT}</documentRoot>
                 </properties>
	</xp:Application>
	<xp:Module>${XDAQ_ROOT}/lib/libpthttp.so</xp:Module>
	
	<xp:Application class="pt::fifo::PeerTransportFifo" id="8" group="startup" network="local"/>
	<xp:Module>${XDAQ_ROOT}/lib/libptfifo.so</xp:Module>
	
	<!-- XRelay -->
	<xp:Application class="xrelay::Application" id="4"  service="xrelay" group="startup" network="local"/>
	<xp:Module>${XDAQ_ROOT}/lib/libxrelay.so</xp:Module>
	
	<!-- HyperDAQ -->
	<xp:Application class="hyperdaq::Application" id="3" group="startup" service="hyperdaq" network="local"/>
	<xp:Module>${XDAQ_ROOT}/lib/libhyperdaq.so</xp:Module>
	
	<xp:Application class="tstore::TStore" id="120" instance="0" network="local" group="database">
	  <properties xmlns="urn:xdaq-application:TStore" xsi:type="soapenc:Struct">
	  <configurationRootDirectory xsi:type="xsd:string">${TSTORE_XML}</configurationRootDirectory>
	  </properties>
	</xp:Application>
	
	<xp:Module>${XDAQ_ROOT}/lib/libxalanMsg.so</xp:Module>
	<xp:Module>${XDAQ_ROOT}/lib/libxalan-c.so</xp:Module>
	<xp:Module>${XDAQ_ROOT}/lib/libxoapfilter.so</xp:Module>
	<xp:Module>${XDAQ_ROOT}/lib/libtstoreutils.so</xp:Module>
	<xp:Module>${XDAQ_ROOT}/lib/libtstoreclient.so</xp:Module>
	<xp:Module>${XDAQ_ROOT}/lib/libtstore.so</xp:Module>
</xp:Profile>
