#ifndef PLOTUTILITIES_H
#define PLOTUTILITIES_H

//cpp dependencies
#include <iostream>
#include <string>

//ROOT dependencies
#include "TBox.h"
#include "TCanvas.h"
#include "TError.h"
#include "TH1.h"
#include "TMath.h"

//Local
#include "include/stringUtil.h"

inline std::string prettyString(double inVal, const int prec, const bool doDot)
{
  const int maxPrec = 18;
  if(prec > maxPrec){
    std::cout << "PRETTYSTRING ERROR: CANNOT HANDLE REQUESTED PRECISION \'" << prec << "\', max is \'" << maxPrec << "\'. return empty string" << std::endl;
    return "";
  }

  std::string minStr = "";
  if(inVal < 0) minStr = "-";
  inVal = TMath::Abs(inVal);
  std::string retStr = "";
  
  inVal *= TMath::Power(10, prec);

  unsigned long long tempInVal = inVal;
  if(inVal - tempInVal > 0.5) ++tempInVal;
  retStr = std::to_string(tempInVal);
  while(retStr.size() < (unsigned int)prec){retStr = "0" + retStr;}
  if(isStrSame(retStr, "0")){
    std::string precStr = "";
    while(prec > (int)precStr.size()){precStr = precStr + "0";}
    retStr = retStr + precStr;
  }

  
  retStr = retStr.substr(0, retStr.size()-prec) + "." + retStr.substr(retStr.size()-prec, prec);
  
  if(retStr.substr(0,1).find(".") != std::string::npos) retStr = "0" + retStr;
  if(retStr.substr(retStr.size()-1,1).find(".") != std::string::npos) retStr.replace(retStr.size()-1, 1, "");

  if(doDot){
    if(retStr.find(".") != std::string::npos) retStr.replace(retStr.find("."), 1, "p");
    if(minStr.size() != 0) minStr = "Neg";
  }
    
  return minStr + retStr;
}

inline std::string prettyStringE(const double inVal, const int prec, const bool doDot)
{
  std::string retStr = prettyString(inVal, prec, false);
  int tenScale = retStr.find(".") - 1;
  while(retStr.find(".") != std::string::npos){
    retStr.replace(retStr.find("."), 1, "");
  }
  if(!doDot) retStr = retStr.substr(0,1) + "." + retStr.substr(1,retStr.size());
  else retStr = retStr.substr(0,1) + "p" + retStr.substr(1,retStr.size());

  while(retStr.find(".")+prec+1 < retStr.size()){
    retStr = retStr.substr(0, retStr.size()-1);
  }
  
  retStr = retStr + "E";
  if(tenScale >= 0) retStr = retStr + "+" + std::to_string(std::abs(tenScale));
  else retStr = retStr + "-" + std::to_string(std::abs(tenScale));
  
  return retStr;
}


inline void prettyCanv(TCanvas* canv_p)
{
  canv_p->SetRightMargin(0.01);
  canv_p->SetLeftMargin(1.5*canv_p->GetLeftMargin());
  canv_p->SetBottomMargin(canv_p->GetLeftMargin());
  canv_p->SetTopMargin(canv_p->GetLeftMargin()/2.);

  return;
}


inline void prettyTH1(TH1* hist_p, const double size, const int style, const int col)
{
  hist_p->SetMarkerSize(size);
  hist_p->SetMarkerStyle(style);
  hist_p->SetMarkerColor(col);
  hist_p->GetXaxis()->CenterTitle();
  hist_p->GetYaxis()->CenterTitle();

  return;
}


inline void drawWhiteBox(Double_t x1, Double_t x2, Double_t y1, Double_t y2)
{
  TBox* tempBox_p = new TBox();
  tempBox_p->SetFillColor(0);
  tempBox_p->DrawBox(x1, y1, x2, y2);
  delete tempBox_p;
}


inline void quietSaveAs(TCanvas* canv_p, const std::string saveName)
{
  Int_t oldLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;
  canv_p->SaveAs(saveName.c_str());
  gErrorIgnoreLevel = oldLevel;
  
  return;
}


inline double getNearestFactor10Up(double inVal, UInt_t steps = 0)
{
  double val = 0.00000000000001;
 
  for(UInt_t i = 0; i < 28; ++i){
    //    std::cout << val << std::endl;
    if(inVal >= val && inVal < val*10){
      val *= 10;
      break;
    }
    else val *= 10;
  }

  for(UInt_t i = 0; i < steps; ++i){
    val *= 10;
  }
  
  return val;
}

inline double getNearestFactor10Down(double inVal, UInt_t steps = 0)
{
  double val = 0.00000000000001;
 
  for(UInt_t i = 0; i < 28; ++i){
    //    std::cout << val << std::endl;
    if(inVal >= val && inVal < val*10){
      break;
    }
    else val *= 10;
  }

  for(UInt_t i = 0; i < steps; ++i){
    val /= 10;
  }
  
  return val;
}

inline void getNXNYPanels(int inTotal, int* nXVal, int* nYVal)
{
  if(inTotal == 1){
    (*nXVal) = 1;
    (*nYVal) = 1;
  }
  else if(inTotal == 2){
    (*nXVal) = 2;
    (*nYVal) = 1;
  }
  else if(inTotal == 3){
    (*nXVal) = 3;
    (*nYVal) = 1;
  }
  else if(inTotal == 4){
    (*nXVal) = 2;
    (*nYVal) = 2;
  }
  else if(inTotal == 5){
    (*nXVal) = 3;
    (*nYVal) = 2;
  }
  else if(inTotal == 6){
    (*nXVal) = 3;
    (*nYVal) = 2;
  }
  else if(inTotal == 7){
    (*nXVal) = 4;
    (*nYVal) = 2;
  }
  else if(inTotal == 8){
    (*nXVal) = 4;
    (*nYVal) = 2;
  }
  else if(inTotal == 9){
    (*nXVal) = 3;
    (*nYVal) = 3;
  }
  else if(inTotal == 10){
    (*nXVal) = 4;
    (*nYVal) = 3;
  }
  else if(inTotal == 11){
    (*nXVal) = 4;
    (*nYVal) = 3;
  }
  else if(inTotal == 12){
    (*nXVal) = 4;
    (*nYVal) = 3;
  }
  else if(inTotal == 13){
    (*nXVal) = 4;
    (*nYVal) = 4;
  }
  else if(inTotal == 14){
    (*nXVal) = 4;
    (*nYVal) = 4;
  }
  else if(inTotal == 15){
    (*nXVal) = 4;
    (*nYVal) = 4;
  }
  else if(inTotal == 16){
    (*nXVal) = 4;
    (*nYVal) = 4;
  }
  else if(inTotal == 17){
    (*nXVal) = 5;
    (*nYVal) = 4;
  }
  else if(inTotal == 18){
    (*nXVal) = 5;
    (*nYVal) = 4;
  }
  else if(inTotal == 19){
    (*nXVal) = 5;
    (*nYVal) = 4;
  }
  else if(inTotal == 20){
    (*nXVal) = 5;
    (*nYVal) = 4;
  }
  else if(inTotal == 21){
    (*nXVal) = 5;
    (*nYVal) = 5;
  }
  else if(inTotal == 22){
    (*nXVal) = 5;
    (*nYVal) = 5;
  }
  else if(inTotal == 23){
    (*nXVal) = 5;
    (*nYVal) = 5;
  }
  else if(inTotal == 24){
    (*nXVal) = 5;
    (*nYVal) = 5;
  }
  else if(inTotal == 25){
    (*nXVal) = 5;
    (*nYVal) = 5;
  }
  else if(inTotal == 26){
    (*nXVal) = 6;
    (*nYVal) = 5;
  }
  else if(inTotal == 27){
    (*nXVal) = 6;
    (*nYVal) = 5;
  }
  else if(inTotal == 28){
    (*nXVal) = 6;
    (*nYVal) = 5;
  }
  else if(inTotal == 29){
    (*nXVal) = 6;
    (*nYVal) = 5;
  }
  else if(inTotal == 30){
    (*nXVal) = 6;
    (*nYVal) = 5;
  }
  else if(inTotal == 31){
    (*nXVal) = 6;
    (*nYVal) = 6;
  }
  else if(inTotal == 32){
    (*nXVal) = 6;
    (*nYVal) = 6;
  }
  else if(inTotal == 33){
    (*nXVal) = 6;
    (*nYVal) = 6;
  }
  else if(inTotal == 34){
    (*nXVal) = 6;
    (*nYVal) = 6;
  }
  else if(inTotal == 35){
    (*nXVal) = 6;
    (*nYVal) = 6;
  }
  else if(inTotal == 37){
    (*nXVal) = 7;
    (*nYVal) = 6;
  }
  else if(inTotal == 38){
    (*nXVal) = 7;
    (*nYVal) = 6;
  }
  else if(inTotal == 39){
    (*nXVal) = 7;
    (*nYVal) = 6;
  }
  else if(inTotal == 40){
    (*nXVal) = 7;
    (*nYVal) = 6;
  }
  else if(inTotal == 41){
    (*nXVal) = 7;
    (*nYVal) = 6;
  }
  else if(inTotal == 42){
    (*nXVal) = 7;
    (*nYVal) = 6;
  }
  else{
    std::cout << "plotUtilities::getNXNYPanels ERROR - Requested dimensions for \'" << inTotal << "\' panels is less than max 42. return w/ no dimensions" << std::endl;
  }
  
  return;
}

inline void drawWhiteBoxNDC(TCanvas* canv_p, Double_t x1, Double_t y1, Double_t x2, Double_t y2, int color = 0)
{
  canv_p->cd();
  TBox* box_p = new TBox();
  box_p->SetFillColor(color);
  box_p->DrawBox(x1, y1, x2, y2);
  delete box_p;

  return;
}


#endif
