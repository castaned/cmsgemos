#include <gem/utils/db/GEMDBAccess.h>
#include <gem/utils/db/TStoreRequest.h>
#include "toolbox/TimeInterval.h"
#include "tstore/client/AttachmentUtils.h"
#include "tstore/client/Client.h"
#include "xcept/tools.h"
#include "xgi/Utils.h"
#include "xgi/Method.h"
#include "xdaq/WebApplication.h"
#include "cgicc/HTMLClasses.h"
//#include "tstoretest/RandomData.h"

gem::utils::db::GEMDBAccess::GEMDBAccess()
{
}

gem::utils::db::GEMDBAccess::~GEMDBAccess()
{
}

// void gem::utils::db::GEMDBAccess::query(xdata::Table &results) throw (xgi::exception::Exception) {
//   try {
//     xdata::Table results;
//     std::string connectionID=connect();

//     SetView(connectionID,results);
//     disconnect(connectionID);
// }


xoap::MessageReference  gem::utils::db::GEMDBAccess::getViewInfo(const std::string &ViewName) throw  (xcept::Exception) {
  
  TStoreRequest request("connect");
  
  //add the view ID
  request.addTStoreParameter("id","urn:tstore-view-SQL:"+ViewName);
  
  //this parameter is mandatory. "basic" is the only value allowed at the moment
  request.addTStoreParameter("authentication","basic");
 
  //login credentials in format username/password
  request.addTStoreParameter("credentials","CMS_GEM_APPUSER_R/GEM_Reader_2015");  // Development GEMDB
  
  //connection will time out after 20 minutes
  toolbox::TimeInterval timeout(1200,0); 
  request.addTStoreParameter("timeout",timeout.toString("xs:duration"));
  
  xoap::MessageReference message=request.toSOAP();
  
  return message;
}
  




std::string gem::utils::db::GEMDBAccess::connect(xoap::MessageReference &response) throw (xcept::Exception) {
  return tstoreclient::connectionID(response);
}


xoap::MessageReference gem::utils::db::GEMDBAccess::disconnectmsg(const std::string &connectionID) throw (xcept::Exception) {
  TStoreRequest request("disconnect");
  
  //add the connection ID
  request.addTStoreParameter("connectionID",connectionID);
  
  xoap::MessageReference message=request.toSOAP();
  
  return message;
}


 
 xoap::MessageReference gem::utils::db::GEMDBAccess::SetViewInfo(const std::string &viewname,const std::string &connectionID) throw (xcept::Exception) {

   //for a query, we need to send some parameters which are specific to SQLView.
   //these use the namespace tstore-view-SQL. 
	
   //In general, you might have the view name in a variable, so you won't know the view class. In this
   //case you can find out the view class using the TStore client library:
   std::string viewClass=tstoreclient::classNameForView("urn:tstore-view-SQL:"+viewname);
   
  //If we give the name of the view class when constructing the TStoreRequest, 
  //it will automatically use that namespace for
  //any view specific parameters we add.
   TStoreRequest request("query",viewClass);
   
   //add the connection ID
   request.addTStoreParameter("connectionID",connectionID);
   
   //for an SQLView, the name parameter refers to the name of a query section in the configuration
   request.addViewSpecificParameter("name",viewname);
   
   xoap::MessageReference message=request.toSOAP();

   return message;
   //  xoap::MessageReference response=sendSOAPMessage(message);


 }


void gem::utils::db::GEMDBAccess::SetView(xoap::MessageReference &response, xdata::Table &results) throw (xcept::Exception) {

   // use the TStore client library to extract the first attachment of type "table"
   //from the SOAP response
   if (!tstoreclient::getFirstAttachmentOfType(response,results)) {
     XCEPT_RAISE (xcept::Exception, "Server returned no data");
   }
}



// void gem::utils::db::GEMDBAccess::setParameter(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception) {
//   try
//     {
//       cgicc::Cgicc cgi(in);
//       myParameter_ = cgi["value"]->getIntegerValue();
//       this->Default(in,out);
//       //      *out<<"  Parameter  "<<myParameter_<<std::endl;

//     }
//   catch (const std::exception & e)
//     {
//       XCEPT_RAISE(xgi::exception::Exception, e.what());
//     }	
// }



// void insert(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception) {
//   try {
//     xdata::Table results;
	  
//     std::string connectionID=connect();
	  
//     xdata::Table newRows;
	  
//     //get the definition of the table, so we know what data types should be in each row
//     getDefinition(connectionID,newRows);
	  
//     //add some random rows into the table definition
//     //the RandomData class is part of the TStoreTest application
//     RandomData randomData;
//     randomData.insertRandomRowsIntoTable(newRows);
	  
//     //insert the new rows
//     insert(connectionID,newRows);
	  
//     disconnect(connectionID);
	  
//   } catch (xcept::Exception &e) {
//     LOG4CPLUS_ERROR(this->getApplicationLogger(),xcept::stdformat_exception_history(e));
//   }
// }


// void getDefinition(const std::string &connectionID,xdata::Table &results) throw (xcept::Exception) {
//   //the definition message is essentially the same as a query message.
//   //instead of retrieving a table full of results, we retrieve an empty table
//   //with the appropriate column names and types.
  
//   //we need to send some parameters which are specific to SQLView.
//   //these use the namespace tstore-view-SQL. 
  
//   //In general, you might have the view name in a variable, so you won't know the view class. In this
//   //case you can find out the view class using the TStore client library:
//   std::string viewClass=tstoreclient::classNameForView("urn:tstore-view-SQL:TStoreInsert");
  
//   TStoreRequest request("definition",viewClass);
  
//   //add the connection ID
//   request.addTStoreParameter("connectionID",connectionID);
  
//   //for an SQLView, the name parameter refers to the name of a insert section in the configuration
//   //since the definition is used for an insert.
//   //We'll use the "test" one.
//   request.addViewSpecificParameter("name","test");
  
//   xoap::MessageReference message=request.toSOAP();
//   xoap::MessageReference response=sendSOAPMessage(message);
  
//   //use the TStore client library to extract the first attachment of type "table"
//   //from the SOAP response
//   if (!tstoreclient::getFirstAttachmentOfType(response,results)) {
//     XCEPT_RAISE (xcept::Exception, "Server returned no data");
//   }
// }


// void insert(const std::string &connectionID,xdata::Table &newRows) throw (xcept::Exception) {
  
//   //for a query, we need to send some parameters which are specific to SQLView.
//   //these use the namespace tstore-view-SQL. 
  
//   //In general, you might have the view name in a variable, so you won't know the view class. In this
//   //case you can find out the view class using the TStore client library:
//   std::string viewClass=tstoreclient::classNameForView("urn:tstore-view-SQL:TStoreInsert");
	
//   //If we give the name of the view class when constructing the TStoreRequest, 
//   //it will automatically use that namespace for
//   //any view specific parameters we add.
//   TStoreRequest request("insert",viewClass);
	
//   //add the connection ID
//   request.addTStoreParameter("connectionID",connectionID);
	
//   //for an SQLView, the name parameter refers to the name of a query section in the configuration
//   //We'll use the "test" one.
//   request.addViewSpecificParameter("name","test");
	
//   xoap::MessageReference message=request.toSOAP();	
//   //add our new rows as an attachment to the SOAP message
//   //the last parameter is the ID of the attachment. The SQLView does not mind what it is, as there should only be one attachment per message.
//   tstoreclient::addAttachment(message,newRows,"test");
	
//   xoap::MessageReference response=sendSOAPMessage(message);
// }
