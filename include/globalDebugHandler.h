//Author: Chris McGinn (2020.01.28)

#ifndef GLOBALDEBUGHANDLER_H
#define GLOBALDEBUGHANDLER_H

//cpp
#include <string>

class globalDebugHandler
{
 public:
  globalDebugHandler();
  ~globalDebugHandler(){};

  bool GetDoGlobalDebug();

 private:
  const std::string envVarStr = "DOGLOBALDEBUGROOT";
  bool m_doGlobalDebug;
};

#endif
