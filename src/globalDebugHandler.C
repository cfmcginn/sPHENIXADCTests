//Author: Chris McGinn (2020.01.28)

//cpp
#include <iostream>

//ROOT
#include "TSystem.h"

//Local
#include "include/globalDebugHandler.h"
#include "include/stringUtil.h"

//public member functions
globalDebugHandler::globalDebugHandler()
{
  m_doGlobalDebug = false;
  std::string doGlobalDebugStr = "";
  if(gSystem->Getenv(envVarStr.c_str()) != nullptr) doGlobalDebugStr = gSystem->Getenv(envVarStr.c_str());
  if(doGlobalDebugStr.size() != 1){
    std::cout << "ERROR IN GLOBALDEBUGHANDLER: Environment variable \'" << envVarStr << "\' is not defined correctly in scope. Currently \'" << doGlobalDebugStr << "\'. Please set to 0 or 1. defaulting to false" << std::endl;
  }
  else if(!isStrSame(doGlobalDebugStr, "1") && !isStrSame(doGlobalDebugStr, "0")){
    std::cout << "ERROR IN GLOBALDEBUGHANDLER: Environment variable \'" << envVarStr << "\' is not defined correctly in scope. Currently \'" << doGlobalDebugStr << "\'. Please set to 0 or 1. defaulting to false" << std::endl;
  }
  else m_doGlobalDebug = std::stoi(doGlobalDebugStr);

  return;
}

bool globalDebugHandler::GetDoGlobalDebug(){return m_doGlobalDebug;}
