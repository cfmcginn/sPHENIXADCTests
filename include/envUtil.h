#ifndef ENVUTIL_H
#define ENVUTIL_H

//cpp
#include <iostream>
#include <map>
#include <string>
#include <vector>

//ROOT
#include "TEnv.h"
#include "THashList.h"

//Local
#include "include/stringUtil.h"

inline bool checkEnvForParams(TEnv* inEnv_p, std::vector<std::string> inParams)
{
  THashList* hash_p = (THashList*)inEnv_p->GetTable();
  bool retVal = true;

  for(auto const & param : inParams){
    bool isFound = false;

    for(Int_t entry = 0; entry < hash_p->GetEntries(); ++entry){
      std::string name = hash_p->At(entry)->GetName();
      if(isStrSame(name, param)){
        isFound = true;
        break;
      }
    }

    if(!isFound){
      std::cout << "PLOTMAKING ERROR - Missing required parameter \'" << param << "\'. return false" << std::endl;
      retVal = false;
    }
  }

  return retVal;
}

inline bool compEnvParams(TEnv* inEnv1_p, TEnv* inEnv2_p, std::vector<std::string> inParams)
{
  bool retVal = true;

  for(auto const & param : inParams){
    std::string val1 = inEnv1_p->GetValue(param.c_str(), "");
    std::string val2 = inEnv2_p->GetValue(param.c_str(), "");

    if(val1.size() != 0){
      while(val1.substr(0,1).find(" ") != std::string::npos){
	val1.replace(0, 1, "");
	if(val1.size() == 0) break;
      }

      if(val1.size() != 0){
	while(val1.substr(val1.size()-1,1).find(" ") != std::string::npos){
	  val1.replace(val1.size()-1, 1, "");
	  if(val1.size() == 0) break;
	}
      }
    }
    
    if(val2.size() != 0){
      while(val2.substr(0,1).find(" ") != std::string::npos){
	val2.replace(0, 1, "");
	if(val2.size() == 0) break;
      }

      if(val2.size() != 0){
	while(val2.substr(val2.size()-1,1).find(" ") != std::string::npos){
	  val2.replace(val2.size()-1, 1, "");
	  if(val2.size() == 0) break;
	}
      }
    }
    
    if(isStrSame(val1, val2)) continue;

    std::cout << "Values for param \'" << param << "\' differ. \'" << val1 << "\' vs. \'" << val2 << "\'. return false" << std::endl;
    retVal = false;
  }

  return retVal;
}


inline std::map<std::string, std::string> GetMapFromEnv(TEnv* inEnv_p)
{
  std::map<std::string, std::string> retMap;
  THashList* hash_p = (THashList*)inEnv_p->GetTable();
  for(Int_t entry = 0; entry < hash_p->GetEntries(); ++entry){
    std::string name = hash_p->At(entry)->GetName();

    retMap[name] = inEnv_p->GetValue(name.c_str(), "");
  }

  return retMap;
}


#endif
