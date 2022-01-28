#ifndef BUFPALETTE_H
#define BUFPALETTE_H

#include <vector>

#include "TColor.h"

class bufPalette{
 public:
  bufPalette();
  ~bufPalette(){};

  std::vector<Int_t> bufColors;

  Int_t getColor(unsigned int colPos);
};

bufPalette::bufPalette()
{
  TColor* bufCol = new TColor();
  bufColors.push_back(bufCol->GetColor(50, 49, 66)); //BLUE
  bufColors.push_back(bufCol->GetColor(208,184,124)); //GOLD
  bufColors.push_back(bufCol->GetColor(2,137,0)); //GREEN2
  delete bufCol;
  return;
}


Int_t bufPalette::getColor(unsigned int colPos){return bufColors.at(colPos);}


#endif
