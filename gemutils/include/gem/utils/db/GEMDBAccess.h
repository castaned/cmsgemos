#ifndef GEM_UTILS_DB_GEMDBACCESS_H
#define GEM_UTILS_DB_GEMDBACCESS_H
#include "log4cplus/logger.h"

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Table.h"

namespace gem {
  namespace utils {
    namespace db { 
      
      class GEMDBAccess
      {

      public:

	GEMDBAccess();
	~GEMDBAccess();

	xoap::MessageReference getViewInfo(const std::string &vname) throw (xcept::Exception);
	std::string connect(xoap::MessageReference &response) throw (xcept::Exception);
	xoap::MessageReference SetViewInfo(const std::string &vname, const std::string &connectionID) throw (xcept::Exception); 
	void SetView(xoap::MessageReference &resp, xdata::Table &res) throw (xcept::Exception);


	xoap::MessageReference disconnectmsg(const std::string &connectionID) throw (xcept::Exception);



	//	void query(xdata::Table &results) throw (xgi::exception::Exception);
	
	//      private:
	//	xoap::MessageReference sendSOAPMessage(xoap::MessageReference &message) throw (xcept::Exception);
	// void insert(const std::string &connectionID,xdata::Table &newRows) throw (xcept::Exception);
	// void getDefinition(const std::string &connectionID,xdata::Table &results) throw (xcept::Exception);
	// void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	
      };
    }
  }
}      

#endif
