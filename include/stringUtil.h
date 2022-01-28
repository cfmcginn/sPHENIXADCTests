#ifndef STRINGUTIL_H
#define STRINGUTIL_H

//c+cpp
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

inline bool isStrSame(std::string inStr1, std::string inStr2)
{
  if(inStr1.size() != inStr2.size()) return false;
  if(inStr1.find(inStr2) == std::string::npos) return false;
  return true;
}

inline std::string removeAllWhiteSpace(std::string inStr)
{
  while(inStr.find(" ") != std::string::npos){
    inStr.replace(inStr.find(" "), 1, "");
  }

  return inStr;
}

inline std::string returnAllCapsString(std::string inStr)
{
  const std::string lowStr = "abcdefghijklmnopqrstuvwxyz";
  const std::string hiStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  for(unsigned int lowIter = 0; lowIter < lowStr.size(); ++lowIter){
    while(inStr.find(lowStr.substr(lowIter, 1)) != std::string::npos){
      inStr.replace(inStr.find(lowStr.substr(lowIter, 1)), 1, hiStr.substr(lowIter, 1));
    }
  }

  return inStr;
}

inline std::string returnAllLowercaseString(std::string inStr)
{
  const std::string lowStr = "abcdefghijklmnopqrstuvwxyz";
  const std::string hiStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  for(unsigned int hiIter = 0; hiIter < hiStr.size(); ++hiIter){
    while(inStr.find(hiStr.substr(hiIter, 1)) != std::string::npos){
      inStr.replace(inStr.find(hiStr.substr(hiIter, 1)), 1, lowStr.substr(hiIter, 1));
    }
  }

  return inStr;
}

inline bool isStrFromCharSet(const std::string inStr, const std::string charSet)
{
  for(unsigned int iter = 0; iter < inStr.size(); ++iter){
    if(charSet.find(inStr.substr(iter, 1)) == std::string::npos){
      return false;
    }
  }

  return true;
}

inline bool isStrAllAlpha(std::string inStr){return isStrFromCharSet(returnAllCapsString(inStr), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");}
inline bool isStrInt(std::string inStr){return isStrFromCharSet(inStr, "-0123456789");}
inline bool isStrFloatOrDouble(std::string inStr){return isStrFromCharSet(inStr, ".-0123456789");}

inline bool isStrTrueOrFalse(std::string inStr)
{
  inStr = returnAllCapsString(inStr);
  if(!isStrAllAlpha(inStr)) return false;

  if(inStr.size() == 4 && inStr.find("TRUE") != std::string::npos) return true;
  if(inStr.size() == 5 && inStr.find("FALSE") != std::string::npos) return true;

  return false;
}

inline bool strToTrueOrFalse(std::string inStr)
{
  inStr = returnAllCapsString(inStr);

  if(inStr.size() == 4 && inStr.find("TRUE") != std::string::npos) return true;
  else if(inStr.size() == 5 && inStr.find("FALSE") != std::string::npos) return false;

  std::cout << "Call to strToTrueOrFalse is invalid; \'" << inStr << "\' neither true or false str. return false but you really ought to fix this" << std::endl;
  return false;
}

inline int getRVal(const std::string inStr)
{
  int rVal = -1;
  if(inStr.find("ak1PF") != std::string::npos) rVal = 1;
  else if(inStr.find("ak2PF") != std::string::npos) rVal = 2;
  else if(inStr.find("ak3PF") != std::string::npos) rVal = 3;
  else if(inStr.find("ak4PF") != std::string::npos) rVal = 4;
  else if(inStr.find("ak5PF") != std::string::npos) rVal = 5;
  else if(inStr.find("ak6PF") != std::string::npos) rVal = 6;
  else if(inStr.find("ak8PF") != std::string::npos) rVal = 8;
  else if(inStr.find("ak10PF") != std::string::npos) rVal = 10;
  else if(inStr.find("akCs1P") != std::string::npos) rVal = 1;
  else if(inStr.find("akCs2P") != std::string::npos) rVal = 2;
  else if(inStr.find("akCs3P") != std::string::npos) rVal = 3;
  else if(inStr.find("akCs4P") != std::string::npos) rVal = 4;
  else if(inStr.find("akCs5P") != std::string::npos) rVal = 5;
  else if(inStr.find("akCs6P") != std::string::npos) rVal = 6;
  else if(inStr.find("akCs8P") != std::string::npos) rVal = 8;
  else if(inStr.find("akCs10P") != std::string::npos) rVal = 10;
  else if(inStr.find("akPu3P") != std::string::npos) rVal = 3;
  else if(inStr.find("akPu4P") != std::string::npos) rVal = 4;

  return rVal;
}

inline std::string getRValStr(const std::string inStr)
{
  std::string rVal = "999";
  if(inStr.find("ak1PF") != std::string::npos) rVal = "0.1";
  else if(inStr.find("ak2PF") != std::string::npos) rVal = "0.2";
  else if(inStr.find("ak3PF") != std::string::npos) rVal = "0.3";
  else if(inStr.find("ak4PF") != std::string::npos) rVal = "0.4";
  else if(inStr.find("ak5PF") != std::string::npos) rVal = "0.5";
  else if(inStr.find("ak6PF") != std::string::npos) rVal = "0.6";
  else if(inStr.find("ak8PF") != std::string::npos) rVal = "0.8";
  else if(inStr.find("ak10PF") != std::string::npos) rVal = "1.0";
  else if(inStr.find("akCs1P") != std::string::npos) rVal = "0.1";
  else if(inStr.find("akCs2P") != std::string::npos) rVal = "0.2";
  else if(inStr.find("akCs3P") != std::string::npos) rVal = "0.3";
  else if(inStr.find("akCs4P") != std::string::npos) rVal = "0.4";
  else if(inStr.find("akCs5P") != std::string::npos) rVal = "0.5";
  else if(inStr.find("akCs6P") != std::string::npos) rVal = "0.6";
  else if(inStr.find("akCs8P") != std::string::npos) rVal = "0.8";
  else if(inStr.find("akCs10P") != std::string::npos) rVal = "1.0";
  else if(inStr.find("akPu3P") != std::string::npos) rVal = "0.3";
  else if(inStr.find("akPu4P") != std::string::npos) rVal = "0.4";

  return rVal;
}

inline std::vector<std::string> commaSepStringToVect(std::string inStr)
{
  std::vector<std::string> retVect;

  inStr = inStr + ",";
  while(inStr.find(",,") != std::string::npos){inStr.replace(inStr.find(",,"), 2, ",");}
  if(inStr.size() != 0){
    while(inStr.substr(0,1).find(",") != std::string::npos){
      inStr.replace(0, 1, "");
      if(inStr.size() == 0) break;
    }
  }

  while(inStr.find(",") != std::string::npos){
    retVect.push_back(inStr.substr(0, inStr.find(",")));
    inStr.replace(0, inStr.find(",")+1, "");
  }
  
  return retVect;
}

inline bool vectContainsStr(std::string inStr, std::vector<std::string>* inVect)
{
  bool isInVect = false;
  if(inStr.size() != 0){
    for(unsigned int vI = 0; vI < inVect->size(); ++vI){
      if(isStrSame(inStr, (*inVect)[vI])){
	isInVect = true;
	break;
      }
    }
  }
  return isInVect;
}

inline int vectContainsStrPos(std::string inStr, std::vector<std::string>* inVect)
{
  int vectPos = -1;
  if(inStr.size() != 0){
    for(unsigned int vI = 0; vI < inVect->size(); ++vI){
      if(isStrSame(inStr, (*inVect)[vI])){
	vectPos = vI;
	break;
      }
    }
  }
  return vectPos;
}

inline bool vectContainsInt(int inInt, std::vector<int>* inVect)
{
  bool isInVect = false;
  for(unsigned int vI = 0; vI < inVect->size(); ++vI){
    if(inInt == ((*inVect)[vI])){
      isInVect = true;
      break;
    }
  }
  return isInVect;
}

inline std::vector<std::string> strToVect(std::string inStr, std::string sepChar = ",")
{
  std::vector<std::string> retVect;
  while(inStr.find(sepChar) != std::string::npos){
    retVect.push_back(inStr.substr(0, inStr.find(sepChar)));
    inStr.replace(0, inStr.find(sepChar)+sepChar.size(), "");
  }
  if(inStr.size() != 0) retVect.push_back(inStr);

  return retVect;
}

inline std::vector<float> strToVectF(std::string inStr)
{
  std::vector<float> retVect;
  std::vector<std::string> strVect = strToVect(inStr);
  for(auto const & iter : strVect){
    retVect.push_back(std::stod(iter));
  }
  return retVect;
}

inline std::vector<int> strToVectI(std::string inStr)
{
  std::vector<int> retVect;
  std::vector<std::string> strVect = strToVect(inStr);
  for(auto const & iter : strVect){
    retVect.push_back(std::stod(iter));
  }
  return retVect;
}

inline std::string getDateStr()
{
  std::time_t now = time(0);
  std::tm *ltm = std::localtime(&now);
  std::string dateStr = std::to_string(1+ltm->tm_mon);
  if(dateStr.size() == 1) dateStr = "0" + dateStr;
  std::string dayStr = std::to_string(ltm->tm_mday);
  if(dayStr.size() == 1) dayStr = "0" + dayStr;
  dateStr = std::to_string(1900 + ltm->tm_year) + dateStr + dayStr;

  return dateStr;
}

inline std::string rootFileNameProc(std::string inFileName, std::vector<std::string> modStrs)
{
  const std::string ext = ".root";
  const std::string defaultName = "stringutil_rootfilenameproc_default";
  std::string outFileName = inFileName;

  if(outFileName.size() == 0){
    std::cout << "STRINGUTIL ROOTFILENAMEPROC: Given inFileName is size 0, call defult name \'" << defaultName << "\'." << std::endl;
    outFileName = defaultName;
  }

  if(outFileName.size() >= ext.size()){
    if(isStrSame(outFileName, ext)){
      std::cout << "STRINGUTIL ROOTFILENAMEPROC: Given inFileName is size 0, call defult name \'" << defaultName << "\'." << std::endl;
      outFileName = defaultName;
    }
    else if(isStrSame(outFileName.substr(outFileName.size()-ext.size(), ext.size()), ext)) outFileName.replace(outFileName.size()-ext.size(), ext.size(), "");
  }

  for(unsigned int sI = 0; sI < modStrs.size(); ++sI){
    if(outFileName.size() < modStrs[sI].size() + 1) outFileName = outFileName + "_" + modStrs[sI];
    else if(!isStrSame(modStrs[sI], outFileName.substr(outFileName.size() - modStrs[sI].size(), modStrs[sI].size()))) outFileName = outFileName + "_" + modStrs[sI];
  }
  outFileName = outFileName + ".root";
  return outFileName;
}

inline std::string strLowerToUpper(std::string inStr)
{
  const std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const std::string lower = "abcdefghijklmnopqrstuvwxyz";

  for(unsigned int i = 0; i < inStr.size(); ++i){
    if(lower.find(inStr.substr(i, 1)) != std::string::npos){
      
      inStr.replace(i,1,upper.substr(lower.find(inStr.substr(i, 1)),1));
    }
  }
  return inStr;
}

#endif
