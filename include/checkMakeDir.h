#ifndef CHECKMAKEDIR_H
#define CHECKMAKEDIR_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class checkMakeDir{
 public:
  checkMakeDir(){};
  ~checkMakeDir(){};

  bool checkDir(const std::string inPath);
  bool checkFile(const std::string inFile);
  bool checkFileExt(const std::string inFile, const std::string ext);
  bool doCheckMakeDir(const std::string inPath);
  void invalidFileMessage(const std::string inFile);
  
 private:
  struct stat st;
};

#endif
