//Author: Chris McGinn (2021.02.17)
//Contact at chmc7718@colorado.edu or cffionn on skype for bugs

//c+cpp
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

//ROOT
#include "TCanvas.h"
#include "TDirectory.h"
#include "TEnv.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLatex.h"
#include "TLine.h"
#include "TMath.h"
#include "TPad.h"
#include "TStyle.h"
#include "TTree.h"

//Local
#include "include/bufPalette.h"
#include "include/checkMakeDir.h"
#include "include/cppWatch.h"
#include "include/envUtil.h"
#include "include/fitUtil.h"
#include "include/globalDebugHandler.h"
#include "include/plotUtilities.h"
#include "include/stringUtil.h"

int sphenixADCProcessing(std::string inConfigFileName)
{
  checkMakeDir check;
  if(!check.checkFileExt(inConfigFileName, ".config")) return 1;

  cppWatch preLoopTimer, loopTimer1, loopTimer2, loopTimer3, loopTimerTotal, postLoopTimer;
  preLoopTimer.clear();
  loopTimer1.clear();
  loopTimer2.clear();
  loopTimer3.clear();
  loopTimerTotal.clear();
  postLoopTimer.clear();

  bufPalette buf;
  
  preLoopTimer.start();
  
  const std::string dateStr = getDateStr();
  check.doCheckMakeDir("pdfDir/");
  check.doCheckMakeDir("pdfDir/" + dateStr);
 
  check.doCheckMakeDir("output/");
  check.doCheckMakeDir("output/" + dateStr);
 
  globalDebugHandler gDebug;
  const bool doGlobalDebug = gDebug.GetDoGlobalDebug();

  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
  
  TEnv* config_p = new TEnv(inConfigFileName.c_str());
  std::vector<std::string> necessaryParams = {"INFILENAME",
					      "OUTFILENAME",
					      "MINCHANNEL",
					      "MAXCHANNEL",
					      "GLOBALMAX",
					      "LINRESMIN",
					      "LINRESMAX",
					      "STARTEVT",
					      "SAVEEXT"};  

  if(!checkEnvForParams(config_p, necessaryParams)) return 1;

  std::vector<std::string> validExtsIn = {"dat", "txt"};
  std::vector<std::string> validExtsOut = {"pdf", "png", "gif"};
  
  const std::string sphenixFileName = config_p->GetValue("INFILENAME", "");
  std::string outFileName = config_p->GetValue("OUTFILENAME", "");

  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
  
  const std::string saveExt = config_p->GetValue("SAVEEXT", "");
  const Float_t globalMax = config_p->GetValue("GLOBALMAX", -10.0);
  const Float_t linResMin = config_p->GetValue("LINRESMIN", -10.0);
  const Float_t linResMax = config_p->GetValue("LINRESMAX", -10.0);

  const int minChannel = config_p->GetValue("MINCHANNEL", 0);
  const int maxChannel = config_p->GetValue("MAXCHANNEL", 63);

  const int startEvent = config_p->GetValue("STARTEVT", 0);
  
  if(minChannel < 0 || minChannel > 63 || maxChannel < 0 || maxChannel > 63 || maxChannel < minChannel){
    std::cout << "FIX MIN-MAX CHANNELS (0-63): " << minChannel << "-" << maxChannel << ". return 1" << std::endl;
    return 1;
  }
  
  if(!vectContainsStr(saveExt, &validExtsOut)) return 1;

  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
  
  std::string inExt = "";
  if(sphenixFileName.find(".") != std::string::npos) inExt = sphenixFileName.substr(sphenixFileName.rfind(".")+1, sphenixFileName.size());
  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << ", " << inExt << std::endl;
  
  if(!vectContainsStr(inExt, &validExtsIn)) return 1;

  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
  
  //Following is hard-coded for characterizing the peak
  const double riseTime = 1.5;
    
  std::ifstream inFile(sphenixFileName.c_str());
  std::string lineStr;

  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

  std::string boardID = "";
  int nLine = 0;
  int nSignals = 0;
  int nEvent = 0;
  int nSteps = 0;
  int nEventsPerStep = 0;
  int nADCPerStep = 0;
  int nSample = 0;
  
  //3 manual calls for the overhead info
  for(unsigned int i = 0; i < 10; ++i){
    std::getline(inFile, lineStr);
    std::cout << lineStr << std::endl;
    std::vector<std::string> tempVect = strToVect(lineStr, " ");

    if(tempVect[0].find("BOARDID") != std::string::npos) boardID = tempVect[1];
    else if(tempVect[0].find("NUMBEROFSTEPS") != std::string::npos) nSteps = std::stoi(tempVect[1]);
    else if(tempVect[0].find("EVENTSPERSTEP") != std::string::npos) nEventsPerStep = std::stoi(tempVect[1]);
    else if(tempVect[0].find("DACPERSTEP") != std::string::npos) nADCPerStep = std::stoi(tempVect[1]);
    else if(tempVect[0].find("NSAMPLES") != std::string::npos) nSample = std::stoi(tempVect[1]);
    
  }

  const int nEventTotal = nSteps*nEventsPerStep;
  const int nEventDisp = TMath::Max((Int_t)1, (Int_t)nEventTotal/20);

  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;  
  
  std::cout << "Processing input \'" << sphenixFileName << "\'" << std::endl;
  std::cout << " nSteps: " << nSteps << std::endl;
  std::cout << " nEventsPerStep: " << nEventsPerStep << std::endl;
  std::cout << " nADCPerStep: " << nADCPerStep << std::endl;
  std::cout << " nEventTotal: " << nEventTotal << std::endl;
  std::cout << " nSample: " << nSample << std::endl;

  //The following is hard-coded in sphenix_adc_test_jseb2.c
  //  const Int_t nSample = 24;
  //next to define the 2-D decoded data array (int adc_data[64][20])
  const Int_t nADCDataArr1 = 64;
  const Int_t nADCDataArr2 = 50;

  const Int_t nMaxSteps = 100;
  if(nSteps >  nMaxSteps){
    std::cout << "nSteps \'" << nSteps << "\' exceeds max nSteps \'" << nMaxSteps << "\'. return 1" << std::endl;
    return 1;
  }

  if(nSample >  nADCDataArr2){
    std::cout << "nSample \'" << nSample << "\' exceeds max nSample \'" << nADCDataArr2 << "\'. return 1" << std::endl;
    return 1;
  }

  if(outFileName.find("/") == std::string::npos){
    outFileName = "output/" + dateStr + "/" + outFileName;
  }
  if(outFileName.find(".root") == std::string::npos){
    std::cout << "OUTFILENAME \'" << outFileName << "\' is invalid, end in '.root'. return 1" << std::endl;
    return 1;
  }
  else outFileName.replace(outFileName.rfind(".root"), 5, "_" + dateStr + ".root");
  
  
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  std::vector<TDirectory*> dir_p;

  for(Int_t cI = minChannel; cI <= maxChannel; ++cI){
    std::string channelStr = std::to_string(cI);
    if(cI < 10) channelStr = "0" + channelStr;
    channelStr = "channel" + channelStr;

    dir_p.push_back( nullptr );

    outFile_p->cd();
    dir_p[cI - minChannel] = (TDirectoryFile*)outFile_p->mkdir(channelStr.c_str());    
  }

  const Int_t nPulse = 10;
  TH1F* adcPulse_p[nADCDataArr1][nMaxSteps][nPulse];
  TF1* adcPulse_Fit_p[nADCDataArr1][nMaxSteps][nPulse];
  TH1F* adcResponse_p[nADCDataArr1];
  TH1F* adcResponse_Distrib_p[nADCDataArr1][nMaxSteps];
  std::vector<std::vector<std::vector< Float_t> > > adcResponse_DistribVect;

  for(Int_t aI = 0; aI < nADCDataArr1; ++aI){
    for(Int_t sI = 0; sI < nMaxSteps; ++sI){
      for(Int_t pI = 0; pI < nPulse; ++pI){
	adcPulse_p[aI][sI][pI] = nullptr;
	adcPulse_Fit_p[aI][sI][pI] = nullptr;
      }
    }    
  }
    
  for(Int_t i = minChannel; i <= maxChannel; ++i){
    std::string channelStr = std::to_string(i);
    if(i < 10) channelStr = "0" + channelStr;
    channelStr = "Channel" + channelStr;

    outFile_p->cd();
    dir_p[i - minChannel]->cd();

    adcResponse_DistribVect.push_back({});

    adcResponse_p[i] = new TH1F(("adcResponse_" + channelStr + "_h").c_str(), ";Step;Signal", nSteps, -0.5, ((Float_t)nSteps) - 0.5);

    for(Int_t stepI = 0; stepI < nSteps; ++stepI){
      std::string stepStr = std::to_string(stepI);
      if(stepI < 10) stepStr = "0" + stepStr;
      stepStr = "Step" + stepStr;
      
      adcResponse_DistribVect[i-minChannel].push_back({});
            
      for(Int_t pI = 0; pI < nPulse; ++pI){
	std::string evtStr = std::to_string(pI);
	if(pI < 10) evtStr = "0" + evtStr;
	evtStr = "Event" + evtStr;
	
	adcPulse_p[i][stepI][pI] = new TH1F(("adcPulse_" + channelStr + "_" + stepStr + "_" + evtStr + "_h").c_str(), ";N_{Sample};", nSample, -0.5, ((Float_t)nSample) - 0.5);

	adcPulse_Fit_p[i][stepI][pI] = new TF1(("adcPulse_Fit_" + channelStr + "_" + stepStr + "_" + evtStr + "_h").c_str(), SignalShape_PowerLawDoubleExp, -0.5, ((Float_t)nSample) - 0.5, nParam_SignalShape_PowerLawDoubleExp());

      }
      
    }
  }

  TF1* fit_p = new TF1("fit_p", SignalShape_PowerLawDoubleExp, -0.5, ((Float_t)nSample) - 0.5, nParam_SignalShape_PowerLawDoubleExp());

  
  std::vector<unsigned int> readVect;
  unsigned int dataArray[nADCDataArr1][nADCDataArr2];
  bool prevLineStrZero = false;

  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;

  //Starting a loop - we left after processing metadata

  std::getline(inFile, lineStr);
  std::getline(inFile, lineStr);


  nSignals = 0;
  nLine = 0;

  preLoopTimer.stop();

  std::cout << "PRE LOOP TIMING: " << preLoopTimer.totalWall() << std::endl;

  loopTimerTotal.start();
  while(std::getline(inFile, lineStr)){
    loopTimer1.start();
    if(lineStr.size() != 0){
      while(lineStr.substr(0,1).find(" ") != std::string::npos){
	lineStr.replace(0,1,"");
	if(lineStr.size() == 0) break;
      }
    }
       
    if(lineStr.size() == 0){
      if(prevLineStrZero){
	loopTimer1.stop();
	continue;
      }
      else prevLineStrZero = true;
      
      int pos = nEvent/nEventsPerStep;
      int pos2 = nEvent%nEventsPerStep;

      if(nEvent%nEventDisp == 0) std::cout << nEvent << "/" << nEventTotal << std::endl;
    
      nSignals = 0;
      nLine = 0;
      ++nEvent;      
      if(nEvent < startEvent){
	loopTimer1.stop();
	continue;
      }
      else if(nEvent != startEvent){
      
	if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
	
	for(Int_t sI = 0; sI < nSample; ++sI){
	  for(Int_t i = 0; i < nADCDataArr1/2; ++i){
	    dataArray[i*2][sI] = readVect[(i*nSample) + sI] & 0xffff;
	    dataArray[i*2 + 1][sI] = (readVect[(i*nSample) + sI] >> 16) & 0xffff;
	  }
	}      
	
	for(Int_t cI = minChannel; cI <= maxChannel; ++cI){
	  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;	  
	  outFile_p->cd();
	  dir_p[cI-minChannel]->cd();
	  
	  std::string saveName = "channel" + std::to_string(cI) + "_step" + std::to_string(pos) + "_evt" + std::to_string(pos2) + "_h";
	  std::string saveNameFit = "channel" + std::to_string(cI) + "_step" + std::to_string(pos) + "_evt" + std::to_string(pos2) + "_f";
	  TH1F* tempHist_p = new TH1F(saveName.c_str(), ";n_{Sample};ADC", nSample, -0.5, ((Float_t)nSample) - 0.5);
	  
	  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;	  
	  
	  Double_t maxPos = -1;
	  Double_t maxVal = -1;
	  for(Int_t sI = 0; sI < nSample; ++sI){
	    if(dataArray[cI][sI] > maxVal){
	      maxPos = sI;
	      maxVal = dataArray[cI][sI];
	    }
	    
	    tempHist_p->SetBinContent(sI+1, (Float_t)dataArray[cI][sI]);
	    tempHist_p->SetBinError(sI+1, (Float_t)0.1*dataArray[cI][sI]);
	    
	    if(pos2 < nPulse){
	      adcPulse_p[cI][pos][pos2]->SetBinContent(sI+1, (Float_t)dataArray[cI][sI]);
	      adcPulse_p[cI][pos][pos2]->SetBinError(sI+1, ((Float_t)dataArray[cI][sI])*0.1);
	    }
	  }
	  
	  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
	  
	  tempHist_p->SetMarkerStyle(24);
	  tempHist_p->SetMarkerSize(1.5);
	  tempHist_p->SetMarkerColor(buf.getColor(1));
	  tempHist_p->SetLineColor(buf.getColor(1));
	  
	  if(pos2 < nPulse){
	    adcPulse_p[cI][pos][pos2]->SetMarkerStyle(20);
	    adcPulse_p[cI][pos][pos2]->SetMarkerSize(1.5);
	    adcPulse_p[cI][pos][pos2]->SetMarkerColor(buf.getColor(1));
	    adcPulse_p[cI][pos][pos2]->SetLineColor(buf.getColor(1));	  
	  }
	  
	  maxVal -= dataArray[cI][0];
	  
	  std::vector<double> paramDefaults = {maxVal * 0.7,
					     maxPos - riseTime,
					       5.0,
					       riseTime,
					       (Float_t)dataArray[cI][0],
					       0,
					       riseTime};
	  
	  std::vector<double> paramMin = {maxVal * -1.5,
					  maxPos - riseTime*3,
					  1,
					  riseTime*.2,
					  ((Float_t)dataArray[cI][0]) - TMath::Abs(maxVal),
					  0,
					  riseTime};
	  
	  std::vector<double> paramMax = {maxVal * 1.5,
					  maxPos + riseTime,
					  10.,
					  riseTime*10,
					  ((Float_t)dataArray[cI][0]) + TMath::Abs(maxVal),
					  0,
					  riseTime};
	  
	  
	  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << ", " << cI << ", " << pos << ", " << pos2 << std::endl;
	  
	  for(Int_t sI = 0; sI < nParam_SignalShape_PowerLawDoubleExp(); ++sI){
	    fit_p->SetParameter(sI, paramDefaults[sI]);
	    
	    if(sI < 2) fit_p->SetParLimits(sI, paramMin[sI], paramMax[sI]);
	  }
	  
	  tempHist_p->Fit(fit_p, "Q N 0", "", -0.5, ((Float_t)nSample) - 0.5);
	  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
	  
	  if(pos2 <= nPulse - 1){
	    for(Int_t sI = 0; sI < nParam_SignalShape_PowerLawDoubleExp(); ++sI){
	      adcPulse_Fit_p[cI][pos][pos2]->SetParameter(sI, fit_p->GetParameter(sI));
	    }	  
	  }
	  
	  if(pos2 == nPulse-1){
	    TCanvas* canv_p = new TCanvas("canv_p", "", 4000, 1600);
	    canv_p->SetTopMargin(0.01);
	    canv_p->SetBottomMargin(0.01);
	    canv_p->SetLeftMargin(0.01);
	    canv_p->SetRightMargin(0.01);

	    canv_p->SetFillColor(buf.getColor(0));
	    
	    canv_p->Divide(5,2);
	    if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
	    
	    TLatex* label_p = new TLatex();
	    label_p->SetNDC();
	    
	    for(Int_t pulseI = 0; pulseI < nPulse; ++pulseI){
	      canv_p->cd();
	      canv_p->cd(pulseI+1);

	      gPad->SetTopMargin(0.01);
	      gPad->SetRightMargin(0.01);
	      gPad->SetLeftMargin(0.2);
	      gPad->SetBottomMargin(0.14);
	      
	      adcPulse_p[cI][pos][pulseI]->SetMinimum(-2000);
	      //	    adcPulse_p[cI][pos][pulseI]->SetMaximum(1.1*adcPulse_p[cI][pos][pulseI]->GetMaximum());

	      
	      adcPulse_p[cI][pos][pulseI]->GetXaxis()->SetLabelColor(buf.getColor(1));
	      adcPulse_p[cI][pos][pulseI]->GetXaxis()->SetAxisColor(buf.getColor(1));
	      adcPulse_p[cI][pos][pulseI]->GetXaxis()->SetTitleColor(buf.getColor(1));

	      adcPulse_p[cI][pos][pulseI]->GetYaxis()->SetLabelColor(buf.getColor(1));
	      adcPulse_p[cI][pos][pulseI]->GetYaxis()->SetAxisColor(buf.getColor(1));
	      adcPulse_p[cI][pos][pulseI]->GetYaxis()->SetTitleColor(buf.getColor(1));

	      std::cout << "Y title: " << adcPulse_p[cI][pos][pulseI]->GetYaxis()->GetTitle() << std::endl;
	      std::cout << "xLabelSize: "<< adcPulse_p[cI][pos][pulseI]->GetXaxis()->GetLabelSize() << std::endl;
	      std::cout << "xTitleSize: "<< adcPulse_p[cI][pos][pulseI]->GetXaxis()->GetTitleSize() << std::endl;
	      std::cout << "yLabelSize: "<< adcPulse_p[cI][pos][pulseI]->GetYaxis()->GetLabelSize() << std::endl;
	      std::cout << "yTitleSize: "<< adcPulse_p[cI][pos][pulseI]->GetYaxis()->GetTitleSize() << std::endl;
	      std::cout << "yTitleOffset: "<< adcPulse_p[cI][pos][pulseI]->GetYaxis()->GetTitleOffset() << std::endl;


	      adcPulse_p[cI][pos][pulseI]->GetYaxis()->SetTitle("ADC Pulse");

	      adcPulse_p[cI][pos][pulseI]->GetXaxis()->SetTitleSize(0.055);
	      adcPulse_p[cI][pos][pulseI]->GetYaxis()->SetTitleSize(0.055);
	      adcPulse_p[cI][pos][pulseI]->GetYaxis()->SetTitleOffset(1.9);

	      adcPulse_p[cI][pos][pulseI]->GetXaxis()->SetLabelSize(0.05);
	      adcPulse_p[cI][pos][pulseI]->GetYaxis()->SetLabelSize(0.05);

	      adcPulse_p[cI][pos][pulseI]->GetXaxis()->SetTickSize(0.1);
	      adcPulse_p[cI][pos][pulseI]->GetXaxis()->SetTickLength(0.04);
	      
	      adcPulse_p[cI][pos][pulseI]->DrawCopy("HIST E1 P");
	      adcPulse_Fit_p[cI][pos][pulseI]->SetMarkerSize(1);
	      adcPulse_Fit_p[cI][pos][pulseI]->SetLineWidth(1.5);
	      adcPulse_Fit_p[cI][pos][pulseI]->SetMarkerStyle(1);
	      adcPulse_Fit_p[cI][pos][pulseI]->SetMarkerColor(buf.getColor(1));
	      adcPulse_Fit_p[cI][pos][pulseI]->SetLineColor(buf.getColor(1));
	      //	      adcPulse_Fit_p[cI][pos][pulseI]->SetLineStyle(1);
	      adcPulse_Fit_p[cI][pos][pulseI]->DrawCopy("SAME");

	      TLine* line_p = new TLine();
	      line_p->SetLineColor(buf.getColor(1));
	      line_p->SetLineWidth(2);
	      line_p->SetLineStyle(2);
	      line_p->DrawLine(-0.5, 0.0, 27.5, 0.0);	      
	      delete line_p;
	      
	      gPad->SetTicks();
	      gStyle->SetOptStat(0);

	      label_p->SetTextAlign(31);
	      label_p->SetTextColor(buf.getColor(1));
	      if(pulseI == 0){
		label_p->DrawLatex(0.92, 0.66, ("Board: " + boardID).c_str());
		label_p->DrawLatex(0.92, 0.6, ("Channel " + std::to_string(cI)).c_str());
		label_p->DrawLatex(0.92, 0.54, ("ADC Step " + std::to_string(pos)).c_str());
	      }
	      label_p->SetTextAlign(11);

	      label_p->DrawLatex(0.52, 0.92, ("Event " + std::to_string(pulseI)).c_str());

	      //Following ./macros/coresoftware/offline/packages/tpcdaq/TPCDaqDefs.cc          
	      Float_t tempPedestal = adcPulse_Fit_p[cI][pos][pulseI]->GetParameter(4);
	      const double peakpos1 = adcPulse_Fit_p[cI][pos][pulseI]->GetParameter(3);
	      const double peakpos2 = adcPulse_Fit_p[cI][pos][pulseI]->GetParameter(6);
	      double max_peakpos = adcPulse_Fit_p[cI][pos][pulseI]->GetParameter(1) + (peakpos1 > peakpos2 ? peakpos1 : peakpos2);
	      if(max_peakpos > nSample - 1) max_peakpos = nSample - 1;
	      
	      double peak_sample = -1;
	      if(adcPulse_Fit_p[cI][pos][pulseI]->GetParameter(0) > 0) peak_sample = adcPulse_Fit_p[cI][pos][pulseI]->GetMaximumX(adcPulse_Fit_p[cI][pos][pulseI]->GetParameter(1), max_peakpos);
	      else peak_sample = adcPulse_Fit_p[cI][pos][pulseI]->GetMinimumX(adcPulse_Fit_p[cI][pos][pulseI]->GetParameter(1), max_peakpos);
	      
	      double tempPeak = adcPulse_Fit_p[cI][pos][pulseI]->Eval(peak_sample) - tempPedestal;

	      label_p->DrawLatex(0.52, 0.86, ("Pedestal: " + prettyString(tempPedestal, 1, false)).c_str());
	      label_p->DrawLatex(0.52, 0.8, ("Peak: " + prettyString(tempPeak + tempPedestal, 1, false)).c_str());	      //	      label_p->DrawLatex(0.62, 0.8, ("Event " + std::to_string(pulseI)).c_str());
	    }
	    
	    if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
	    
	    std::string saveName = "pdfDir/" + dateStr + "/adcPulse_Channel" + std::to_string(cI) + "_Step" + std::to_string(pos) + "_" + dateStr + "." + saveExt;
	    quietSaveAs(canv_p, saveName);
	    
	    delete canv_p;
	    delete label_p;
	    
	    
	    if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;	  
	  }

	  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;	  
	  
	  outFile_p->cd();
	  dir_p[cI-minChannel]->cd();
	  
	  //Following ./macros/coresoftware/offline/packages/tpcdaq/TPCDaqDefs.cc
	  Float_t tempPedestal = fit_p->GetParameter(4);
	  const double peakpos1 = fit_p->GetParameter(3);
	  const double peakpos2 = fit_p->GetParameter(6);
	  double max_peakpos = fit_p->GetParameter(1) + (peakpos1 > peakpos2 ? peakpos1 : peakpos2);
	  if(max_peakpos > nSample - 1) max_peakpos = nSample - 1;
	  
	  double peak_sample = -1;
	  if(fit_p->GetParameter(0) > 0) peak_sample = fit_p->GetMaximumX(fit_p->GetParameter(1), max_peakpos);
	  else peak_sample = fit_p->GetMinimumX(fit_p->GetParameter(1), max_peakpos);
	  
	  double tempPeak = fit_p->Eval(peak_sample) - tempPedestal;
	  
	  adcResponse_DistribVect[cI - minChannel][pos].push_back(tempPeak);
	  
	  if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;	  
	  fit_p->Write(saveNameFit.c_str(), TObject::kOverwrite);
	  tempHist_p->Write(saveName.c_str(), TObject::kOverwrite);
	  
	  delete tempHist_p;
	}
	
	if(doGlobalDebug) std::cout << "FILE, LINE: " << __FILE__ << ", " << __LINE__ << std::endl;
	loopTimer1.stop();
	readVect.clear();
	continue;
      }
    }
    else prevLineStrZero = false;
    
    loopTimer1.stop();
    loopTimer2.start();

    if(nEvent < startEvent){
      loopTimer2.stop();
      continue;
    }
    
    while(lineStr.find("  ") != std::string::npos){
      lineStr.replace(lineStr.find("  "), 2, " ");
    }

    while(lineStr.find(" ") != std::string::npos){
      lineStr.replace(lineStr.find(" "), 1, ",");
    }
    
    std::vector<std::string> lineVect = strToVect(lineStr);
    
    if(nLine == 1){
      std::stringstream tempStream;
      unsigned long long decimalConvert = 0;
      tempStream << std::hex << lineVect[0];
      tempStream >> decimalConvert;
    }
    else if(nLine > 1 && lineVect.size() == 8){
      for(unsigned int i = 0; i < lineVect.size(); ++i){
	std::stringstream tempStream;
	tempStream << lineVect[i];
	unsigned int tempVal = 0;
	tempStream >> std::hex >> tempVal;
	
	readVect.push_back(tempVal);	
	++nSignals;
      }
    }   
    
    ++nLine;
    loopTimer2.stop();
    //    loopTimer3.start();
  }
  loopTimer3.stop();
  loopTimerTotal.stop();

  double denom = loopTimerTotal.totalCPU();
  
  std::cout << "LOOP TIMES: " << std::endl;
  std::cout << " LOOP 1: " << loopTimer1.totalCPU() << " (" << ((double)loopTimer1.totalCPU())/denom << ")" << std::endl;
  std::cout << " LOOP 2: " << loopTimer2.totalCPU() << " (" << ((double)loopTimer2.totalCPU())/denom << ")" << std::endl;
  std::cout << " LOOP 3: " << loopTimer3.totalCPU() << " (" << ((double)loopTimer3.totalCPU())/denom << ")" <<  std::endl;
  std::cout << " LOOP Total: " << loopTimerTotal.totalCPU() << std::endl;
  postLoopTimer.start();
  
  inFile.close();
    
  outFile_p->cd();

  for(Int_t i = minChannel; i <= maxChannel; ++i){
    outFile_p->cd();
    dir_p[i - minChannel]->cd();

    std::string channelStr = std::to_string(i);
    if(i < 10) channelStr = "0" + channelStr;
    channelStr = "Channel" + channelStr;
    
    for(Int_t sI = 0; sI < nSteps; ++sI){
      std::vector<float> tempVect = adcResponse_DistribVect[i - minChannel][sI];

      std::sort(std::begin(tempVect), std::end(tempVect));

      double delta = tempVect[tempVect.size()-1] - tempVect[0];

      adcResponse_Distrib_p[i][sI] = new TH1F(("adc" + channelStr + "_Step" + std::to_string(sI) + "_h").c_str(), (";ADC (Step=" + std::to_string(sI) + ");Counts").c_str(), 20, tempVect[0] - delta/10., tempVect[tempVect.size()-1] + delta/10.);

      for(unsigned int tI = 0; tI < tempVect.size(); ++tI){
	adcResponse_Distrib_p[i][sI]->Fill(tempVect[tI]);
      }      

      Float_t mean = adcResponse_Distrib_p[i][sI]->GetMean();
      Float_t meanErr = adcResponse_Distrib_p[i][sI]->GetMeanError();

      adcResponse_p[i]->SetBinContent(sI+1, mean);
      adcResponse_p[i]->SetBinError(sI+1, meanErr);
    }

    const Int_t nX = 8;
    const Int_t nY = 5;

    if(nX*nY >= nSteps){
      TCanvas* canv_p = new TCanvas("canv_p", "", nX*200 + nY*200);
      canv_p->SetTopMargin(0.01);
      canv_p->SetBottomMargin(0.01);
      canv_p->SetRightMargin(0.01);
      canv_p->SetLeftMargin(0.01);

      canv_p->Divide(nX, nY);

      for(Int_t sI = 0; sI < nSteps; ++sI){
	canv_p->cd();
	canv_p->cd(sI+1);

	adcResponse_Distrib_p[i][sI]->SetMarkerStyle(24);
	adcResponse_Distrib_p[i][sI]->SetMarkerSize(1.1);
	adcResponse_Distrib_p[i][sI]->SetLineColor(1);
	adcResponse_Distrib_p[i][sI]->SetMarkerColor(1);
	
	adcResponse_Distrib_p[i][sI]->SetMinimum(0.0);

 	adcResponse_Distrib_p[i][sI]->DrawCopy("HIST E1 P");
      }
      
      std::string saveName = "pdfDir/" + dateStr + "/adcResponse_" + channelStr + "_Distrib_" + dateStr + "." + saveExt;
      quietSaveAs(canv_p, saveName.c_str());
      delete canv_p;
    }			  
    else std::cout << "Dimensions nX*nY=" << nX << "*" << nY << "=" << nX*nY << " is less than needed " << nSteps << ". skipping..." << std::endl;
    
    outFile_p->cd();
    dir_p[i - minChannel]->cd();

    for(Int_t sI = 0; sI < nSteps; ++sI){
      adcResponse_Distrib_p[i][sI]->Write("", TObject::kOverwrite);
      delete adcResponse_Distrib_p[i][sI];
    }
    
    adcResponse_p[i]->Write("", TObject::kOverwrite);
  }


  TF1* lineFit_p = new TF1("lineFit_p", "[0] + x*[1]", -0.5, ((Float_t)nSteps) - 0.5);
  
  for(Int_t i = minChannel; i <= maxChannel; ++i){
    TCanvas* canv_p = new TCanvas("canv_p", "", 900, 900);
    canv_p->SetLeftMargin(0.14);
    canv_p->SetBottomMargin(0.14);
    canv_p->SetRightMargin(0.01);
    canv_p->SetTopMargin(0.01);
    
    adcResponse_p[i]->SetMinimum(0.0);
    adcResponse_p[i]->GetYaxis()->SetNdivisions(404);
    
    std::string xTitle = adcResponse_p[i]->GetXaxis()->GetTitle();
    std::string yTitle = adcResponse_p[i]->GetYaxis()->GetTitle();

    xTitle = "#bf{" + xTitle + "}";
    yTitle = "#bf{" + yTitle + "}";
    
    adcResponse_p[i]->GetXaxis()->SetTitle(xTitle.c_str());
    adcResponse_p[i]->GetYaxis()->SetTitle(yTitle.c_str());

    adcResponse_p[i]->SetMaximum(linResMax);
    adcResponse_p[i]->SetMinimum(linResMin);
    
    std::string nEventStr = std::to_string(nEvent - 1);
    if(nEvent - 1 < 10) nEventStr = "0" + nEventStr;
    std::string nChannelStr = std::to_string(i);
    if(i < 10) nChannelStr = "0" + nChannelStr;

    adcResponse_p[i]->SetMarkerStyle(21);
    adcResponse_p[i]->SetMarkerSize(1.5);
    adcResponse_p[i]->SetMarkerColor(1);
    adcResponse_p[i]->SetLineColor(1);

    TLatex* label_p = new TLatex();
    label_p->SetNDC();

    lineFit_p->SetParameter(0, 0.0);
    lineFit_p->SetParameter(1, 100.0);
    adcResponse_p[i]->Fit("lineFit_p", "Q", "", -0.5, ((Float_t)nSteps) - 0.5);
    adcResponse_p[i]->DrawCopy("HIST E1 P");

    TEnv* tempEnv_p = new TEnv();
    tempEnv_p->SetValue("CHANNEL", i);
    tempEnv_p->SetValue("B", lineFit_p->GetParameter(0));
    tempEnv_p->SetValue("DELTAB", lineFit_p->GetParError(0));
    tempEnv_p->SetValue("M", lineFit_p->GetParameter(1));
    tempEnv_p->SetValue("DELTAM", lineFit_p->GetParError(1));
  
    tempEnv_p->WriteFile(("output/" + dateStr + "/channel" + std::to_string(i) + ".config").c_str());
    
    delete tempEnv_p;
    
    label_p->DrawLatex(0.2, 0.8, ("Channel " + std::to_string(i)).c_str());
    
    
    gStyle->SetOptStat(0);
    quietSaveAs(canv_p, "pdfDir/" + dateStr + "/response_Channel" + nChannelStr + "_" + dateStr + "." + saveExt);
    
    delete canv_p;
    delete label_p;
    
    delete adcResponse_p[i];
  }  

  delete fit_p;
  delete lineFit_p;
  
  outFile_p->Close();
  delete outFile_p;

  postLoopTimer.stop();
  
  std::cout << "SPHENIXADCPROCESSING COMPLETE. return 0." << std::endl;
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/sphenixADCProcessing.exe <inConfigFileName>" << std::endl;
    std::cout << "TO DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=1 #from command line" << std::endl;
    std::cout << "TO TURN OFF DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=0 #from command line" << std::endl;
    std::cout << "return 1." << std::endl;
    return 1;
  }
 
  int retVal = 0;
  retVal += sphenixADCProcessing(argv[1]);
  return retVal;
}
