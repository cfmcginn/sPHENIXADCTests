//Author: Chris McGinn (2021.06.02)
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
#include "TBox.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TEnv.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TImage.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TPad.h"
#include "TStyle.h"
#include "TTree.h"

//Local
#include "include/bufPalette.h"
#include "include/checkMakeDir.h"
#include "include/envUtil.h"
#include "include/fitUtil.h"
#include "include/globalDebugHandler.h"
#include "include/plotUtilities.h"
#include "include/stringUtil.h"

int sphenixTokenPassing(std::string inConfigFileName)
{
  checkMakeDir check;
  if(!check.checkFileExt(inConfigFileName, ".config")) return 1;

  const std::string dateStr = getDateStr();
  check.doCheckMakeDir("pdfDir");
  check.doCheckMakeDir("pdfDir/" + dateStr);
  
  bufPalette buf;    
 
  globalDebugHandler gDebug;
  const bool doGlobalDebug = gDebug.GetDoGlobalDebug();

  TEnv* config_p = new TEnv(inConfigFileName.c_str());
  std::vector<std::string> necessaryParams = {"INFILENAME",
 					      "INSERTFAILCONDITION",
					      "INSERTEVENT",
					      "INSERTBOARD",
					      "INSERTCHANNEL",
					      "INSERTSAMPLE",
					      "INSERTVALUE"};
  if(!checkEnvForParams(config_p, necessaryParams)) return 1;

  std::vector<std::string> validExtsIn = {"dat", "txt"};

  std::string saveTag = config_p->GetValue("SAVETAG", "");
  
  const std::string sphenixFileName = config_p->GetValue("INFILENAME", "");    
  const std::string insertFailConditionStr = config_p->GetValue("INSERTFAILCONDITION", "");
  if(!isStrSame(insertFailConditionStr, "0")  && !isStrSame(insertFailConditionStr, "1")){
    std::cout << "REQUIRED PARAMETER 'INSERTFAILCONDITION' MUST BE '0' or '1'. return 1" << std::endl;
    return 1;
  }

  const bool insertFailCondition = std::stoi(insertFailConditionStr);
  
  const int insertEvent = config_p->GetValue("INSERTEVENT", -1);
  const int insertBoard = config_p->GetValue("INSERTBOARD", -1);
  const int insertChannel = config_p->GetValue("INSERTCHANNEL", -1);
  const int insertSample = config_p->GetValue("INSERTSAMPLE", -1);
  const int insertValue = config_p->GetValue("INSERTVALUE", 0);

  if(insertFailCondition){
    std::cout << "INSERTFAILCONDITION IS ON" << std::endl;
    std::cout << "THIS IS A TEST, FAKE SIGNAL IS INSERTED TO CHECK THAT WE CATCH IT" << std::endl;

    std::cout << " INSERT EVENT: " << insertEvent << std::endl;
    std::cout << " INSERT BOARD: " << insertBoard << std::endl;
    std::cout << " INSERT CHANNEL: " << insertChannel << std::endl;
    std::cout << " INSERT SAMPLE: " << insertSample << std::endl;
    std::cout << " INSERT VALUE: " << insertValue << std::endl;
  }
  
  if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;
  
  std::string inExt = "";
  if(sphenixFileName.find(".") != std::string::npos) inExt = sphenixFileName.substr(sphenixFileName.rfind(".")+1, sphenixFileName.size());  
  if(!vectContainsStr(inExt, &validExtsIn)) return 1;

  const int nSample = 12;
  const int nBoards = 4;
  unsigned int lastEvtCh0S0[nBoards];
  const Int_t nBits = 14;
  const Int_t maxBitVal = TMath::Power(2,nBits)-1;
  const Int_t maxBitValExpectation = 8575;

  const Float_t xValExpectedCoeff = 64 + 128 + 64*3 + 8576 - 6*128;
  
  Bool_t bitHitBool[nBoards][maxBitVal + 1];
  for(Int_t bI = 0; bI < nBoards; ++bI){
    for(Int_t bI2 = 0; bI2 < maxBitVal+1; ++bI2){
      bitHitBool[bI][bI2] = false;
    }    
  }
  if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;

  TFile* outFile_p = new TFile("test.root", "RECREATE");
  TH1F* hist_p[nBoards];
  TH1F* histRat_p[nBoards];
  for(Int_t bI = 0; bI < nBoards; ++bI){
    hist_p[bI] = new TH1F(("hist_" + std::to_string(bI) + "_h").c_str(), ";Integer Value from Digitizer;Counts", maxBitValExpectation + 1, -0.5, ((Double_t)maxBitValExpectation) + 0.5);
    histRat_p[bI] = new TH1F(("histRat_" + std::to_string(bI) + "_h").c_str(), ";Integer Value from Digitizer;Counts/#color[2]{Expected}", maxBitValExpectation + 1, -0.5, ((Double_t)maxBitValExpectation) + 0.5);
  }

    if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;

  std::ifstream inFile(sphenixFileName.c_str());
  std::string lineStr;
  std::vector<std::string> lineVect;
  
  //The following is hard-coded in sphenix_adc_test_jseb2.c
  //  const Int_t nSample = 24;
  //next to define the 2-D decoded data array (int adc_data[64][20])
  const Int_t nADCDataArr1 = 64;
  const Int_t nADCDataArr2 = 50;

  if(nSample >  nADCDataArr2){
    std::cout << "nSample \'" << nSample << "\' exceeds max nSample \'" << nADCDataArr2 << "\'. return 1" << std::endl;
    return 1;
  }
  if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;
  
  std::vector<unsigned int> readVect;
  std::vector<std::string> readVectStr;
  unsigned int dataArray[nBoards][nADCDataArr1][nADCDataArr2];

  bool testPass = true;
  bool valueOutsideBitRange = false;

  std::map<std::string, std::string> metaDataMap;
  //Pre-processing the data before we hit the data-taking loop
  //Metadata is defined by parameter name : parameter value, so the conditional we use is lineStr.find(":");
  if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;
  std::getline(inFile, lineStr);
  while(lineStr.find(":") != std::string::npos){
    if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;
    std::string inParameter = lineStr.substr(0, lineStr.find(":"));
    std::string inValue = lineStr.substr(lineStr.find(":")+1, lineStr.size());

    if(doGlobalDebug) std::cout << "LINE, PARAM, VALUE: " << __LINE__ << ", '" << inParameter << "', '" << inValue << "'" << std::endl;

    while(inValue.substr(0,1).find(" ") != std::string::npos){inValue = inValue.substr(1,inValue.size());}
    while(inValue.substr(inValue.size()-1,1).find(" ") != std::string::npos){inValue = inValue.substr(0,inValue.size()-1);}
    metaDataMap[inParameter] = inValue;

    std::getline(inFile, lineStr);
  }

  std::cout << "METADATA" << std::endl;
  for(auto const& value : metaDataMap){
    std::cout << " '" << value.first << "': '" << value.second << "'" << std::endl;
  }

  std::vector<std::string> reqMetadata = {"BOARDID",
					  "DATE",
					  "ADDITIONALTAG",
					  "NUMBEROFEVENT",
					  "NSAMPLES"};

  std::vector<std::string> reqMetadataAlt = {"BOARDID",
					     "DATE",
					     "ADDITIONALTAG",
					     "NUMBEROFEVENTS",
					     "NUMBEROFSAMPLES"};
  
  bool allReqMetadataFound = true;
  for(auto const& val: reqMetadata){
    if(metaDataMap.count(val) == 0){
      std::cout << "REQUIRED METADATA \'" << val << "\' is missing! return 1" << std::endl;
      allReqMetadataFound = false;
    }
  } 

  bool allReqMetadataAltFound = true;
  for(auto const& val: reqMetadataAlt){
    if(metaDataMap.count(val) == 0){
      std::cout << "REQUIRED METADATA \'" << val << "\' is missing! return 1" << std::endl;
      allReqMetadataAltFound = false;
    }
  } 
  
  if(!allReqMetadataFound && !allReqMetadataAltFound){
    inFile.close();
    return 1;
  }
  
  std::string boardIDStr = metaDataMap["BOARDID"]; 

  std::vector<std::string> boardIDs;
  while(boardIDStr.find("_") != std::string::npos){
    std::string tempStr = boardIDStr.substr(0, boardIDStr.find("_"));
    boardIDs.push_back(tempStr);
    boardIDStr.replace(0, boardIDStr.find("_")+1, "");
  }
  if(boardIDStr.size() != 0) boardIDs.push_back(boardIDStr);  
  
  std::string additionalTagStr = metaDataMap["ADDITIONALTAG"];

  std::string dateOfTestStr = metaDataMap["DATE"];
  dateOfTestStr.replace(5, 1, dateOfTestStr.substr(5,1) + ".");
  dateOfTestStr.replace(3, 1, dateOfTestStr.substr(3,1) + ".");
  
  std::string numberOfEventsStr = "";
  if(allReqMetadataFound) numberOfEventsStr = metaDataMap["NUMBEROFEVENT"];
  else numberOfEventsStr = metaDataMap["NUMBEROFEVENTS"];

  std::string numberOfSamplesStr = "";
  if(allReqMetadataFound) numberOfSamplesStr = metaDataMap["NSAMPLES"];
  else numberOfSamplesStr = metaDataMap["NUMBEROFSAMPLES"];
  
  //Starting a loop - we left after processing metadata
  int nEvent = -1;
  while(std::getline(inFile, lineStr)){    
    if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;

    if(lineStr.size() != 0){
      while(lineStr.substr(0,1).find(" ") != std::string::npos){
	lineStr.replace(0,1,"");
	if(lineStr.size() == 0) break;
      }
    }
    if(lineStr.size() == 0) continue;

    while(lineStr.find("  ") != std::string::npos){
      lineStr.replace(lineStr.find("  "), 2, " ");
    }

    /*
    std::vector<unsigned int> wrapVal0s = {511};
    std::vector<unsigned int> wrapVal1s = {0};

    for(unsigned int i = 0; i < 100; ++i){
      wrapVal0s.push_back(wrapVal0s[i] + 128);
      wrapVal1s.push_back(wrapVal1s[i] + 128);
    }
    */
    //    std::cout << wrapVal0s[0] << "-" << wrapVal0s[100] << std::endl;
    //    std::cout << wrapVal1s[0] << "-" << wrapVal1s[100] << std::endl;
    //    return 1;
    
    std::vector<std::string> lineVect = strToVect(lineStr, " ");
    
    if(lineVect.size() == 1){      
      ++nEvent;       
      
      if(nEvent != 0){
	if(nEvent != 1){
	  lastEvtCh0S0[0] = dataArray[0][0][0];
	  lastEvtCh0S0[1] = dataArray[1][0][0];
	  lastEvtCh0S0[2] = dataArray[2][0][0];
	  lastEvtCh0S0[3] = dataArray[3][0][0];
	}
	
	for(Int_t bI = 0; bI < nBoards; ++bI){
	  Int_t ioffset = (64*nSample+4+2)*bI;
 
	  for(Int_t sI = 0; sI < nSample; ++sI){
	    for(Int_t i = 0; i < nADCDataArr1/2; ++i){
	      unsigned int value1 = readVect[ioffset + ((i*nSample) + sI)*2 + 11] & 0xffff;
	      unsigned int value2 = readVect[ioffset + ((i*nSample) + sI)*2 + 10] & 0xffff;
	      std::vector<int> values = {(int)value1, (int)value2};

	      for(unsigned int vI = 0; vI < values.size(); ++vI){
		hist_p[bI]->Fill(values[vI]);

		if(values[vI] < 0 || values[vI] > maxBitVal){
		  std::cout << "WARNING - SOMEHOW YOU HAVE EXCEEDED 14 BITS: " << std::endl;
		  std::cout << " Value: " << std::dec << values[vI] << " outside inclusive range (0-" << std::dec << maxBitVal << ")" << std::endl;
		  std::cout << " Token passing test will fail" << std::endl;
		  testPass = false;
		  valueOutsideBitRange = true;
		}
		else bitHitBool[bI][values[vI]] = true;
	      }
	      
	      
	      dataArray[bI][i*2][sI] = value1;
	      dataArray[bI][i*2 + 1][sI] = value2;
	      //	    dataArray[i*2 + 1][sI] = (readVect[(i*nSample) + sI + 10] >> 16) & 0xffff;

	      
	      if(insertFailCondition && nEvent-1 == insertEvent && sI == insertSample && bI == insertBoard){
		if(i*2 + 1 == insertChannel) dataArray[bI][i*2 + 1][sI] = insertValue;
		else if(i*2 == insertChannel) dataArray[bI][i*2][sI] = insertValue;
	      }	      
	    }
	  }	  
	}	

	std::cout << "EVENT " << nEvent-1 << std::endl;
	for(Int_t bI = 0; bI < nBoards; ++bI){
	  std::cout << "BOARD " << std::dec << bI << std::endl;

	  //Each channel should increment 0x80, or 128 for a successful test
	  bool channelDeltaGood = true;
	  bool hitWrapAround = false;
	  int wrapAroundSamplePos = -1;
	  int wrapAroundChannelPos = -1;
	  
	  for(Int_t cI = 0; cI < nADCDataArr1; ++cI){	    
	    std::cout << " channel " << std::dec << cI << " ";
	    
	    bool sampleDeltaGood = true;
	    for(Int_t sI = 0; sI < nSample; ++sI){
	      if(sI == nSample-1) std::cout << std::hex << dataArray[bI][cI][sI];
	      else std::cout << std::hex << dataArray[bI][cI][sI] << " ";
	    }

	    std::cout << std::endl;
	    
	    //Samples should all increment +1
	    for(Int_t sI = 1; sI < nSample; ++sI){	     
	      int valSample1 = dataArray[bI][cI][sI];
	      int valSample0 = dataArray[bI][cI][sI-1];
	      int delta = valSample1 - valSample0;

	      if(hitWrapAround && sI == wrapAroundSamplePos){
		Int_t plus128 = (cI - wrapAroundChannelPos)*128;
		if(valSample0 - plus128 == 511 && valSample1 - plus128 == 0){
		  //Nothing
		}
		else{
		  std::cout << "ERROR HIT WRAP AROUND SAMPLE POSITION BUT DONT MATCH EXPECTATION" << std::endl;
		}		
	      }
	      else if(valSample0 == 511 && valSample1 == 0){
		hitWrapAround = true;
		wrapAroundSamplePos = sI;
		wrapAroundChannelPos = cI;
	      }
	      else if(delta != 1){		  
		std::cout << std::endl;
		std::cout << "ERROR SAMPLEDELTA IS NOT EQUAL TO 1" << std::endl;
		std::cout << " EVENT: " << std::dec << nEvent - 1 << std::endl;
		std::cout << " BOARD: " << std::dec << bI << std::endl;
		std::cout << " CHANNEL: " << std::dec << cI << std::endl;
		std::cout << " SAMPLES: " << std::dec << sI-1 << "-" << std::dec << sI << std::endl;
		std::cout << " DECIMAL VALUES (Delta): " << std::dec << valSample1 << "-" << std::dec << valSample0 << " (" << std::dec << delta << ")" << std::endl;
		std::cout << " HEX VALUES (Delta): " << std::hex << valSample1 << "-" << std::hex << valSample0 << " (" << std::hex << delta << ")" << std::endl;
		sampleDeltaGood = false;
		std::cout << std::endl;
	      }
	    }

	    
	    if(cI > 0){//Check channel delta, should increment +128 (0x80 hex)
	      int valSample1 = dataArray[bI][cI][0];
	      int valSample0 = dataArray[bI][cI-1][0];
	      int delta = valSample1 - valSample0;
	      
              if(delta != 128){
		std::cout << std::endl;
                std::cout << "ERROR CHANNELDELTA IS NOT EQUAL TO 128 (0x80): " << std::endl;
                std::cout << " EVENT: " << std::dec << nEvent - 1 << std::endl;
                std::cout << " BOARD: " << std::dec << bI << std::endl;
                std::cout << " CHANNELS: " << std::dec << cI << "-" << std::dec << cI-1 << std::endl;
                std::cout << " DECIMAL VALUES (Delta): " << std::dec << valSample1 << "-" << std::dec << valSample0 << " (" << std::dec << delta << ")" << std::endl;
                std::cout << " HEX VALUES (Delta): " << std::hex << valSample1 << "-" << std::hex << valSample0 << " (" << std::hex << delta << ")" << std::endl;
		channelDeltaGood = false;
		std::cout << std::endl;
              }
            }	  
	    
	    if(!sampleDeltaGood){
	      std::cout << "Since sampledelta not equal to 1, token pass test failed" << std::endl;
	      testPass = false;
	    }	    
	  }

	  std::cout << std::endl;
	  if(!channelDeltaGood){
	    std::cout << "Since channeldelta not equal to 128, token pass test failed" << std::endl;
	    std::cout << std::endl;
	    testPass = false;
	  }	    
	}

      	//Now check that all boards incremented from last event to this event by the same values
	bool passesSynchedEventDelta = true;
	if(nEvent != 1){
	  std::cout << "Event Delta (" << std::dec << nEvent - 1 << "-" << std::dec << nEvent - 2 << "): " <<  std::endl;
	  int initialDelta = -1;
	  for(Int_t bI = 0; bI < nBoards; ++bI){
	    int currentVal = dataArray[bI][0][0];
	    int prevVal = lastEvtCh0S0[bI];
	    int delta = currentVal - prevVal;

	    if(delta < 0){//Wrap around check
	      int altDelta = 512 - prevVal + currentVal;
	      delta = altDelta;//this is just wraparound
	    }
	    
	    if(bI == 0) initialDelta = delta;
	    
	    std::cout << " BOARD " << std::dec << bI << " EvtCurrent - EvtPrev (Delta): " << std::dec << currentVal << "-" << std::dec << prevVal <<  " (" << std::dec << delta << ")" << std::endl;
	    std::cout << "  HEX:" << std::hex << currentVal << "-" << std::hex << prevVal << " (" << std::hex << delta << ")" << std::endl;
	  }

	  for(Int_t bI = 1; bI < nBoards; ++bI){
	    int currentVal = dataArray[bI][0][0];
            int prevVal = lastEvtCh0S0[bI];
            int delta = currentVal - prevVal;

	    if(delta < 0){//Wrap around check
	      int altDelta = 512 - prevVal + currentVal;
	      delta = altDelta;//this is just wraparound
	    }
	    
	    if(delta != initialDelta){
	      std::cout << "ERROR EVENTDELTA NOT SYNCHRONIZED ACROSS BOARDS" << std::endl;
	      std::cout << " EVENTCURRENT - EVENTPREV: " << std::dec << nEvent - 1 << "-" << std::dec << nEvent - 2 << std::endl;
	      std::cout << " BOARDS: 0-" << std::dec << bI  << std::endl;
	      std::cout << " BOARD 0 DELTA: " << std::dec << initialDelta << std::endl;
	      std::cout << " BOARD " << std::dec << bI << " DELTA: " << std::dec << delta << std::endl;
	      passesSynchedEventDelta = false;
	    }
	  }	  
	}
	std::cout << std::endl;	
	if(!passesSynchedEventDelta){
	  std::cout << "Since interEvent Delta is not same across all boards, token pass test failed" << std::endl;
	  std::cout << std::endl;
	  testPass = false;
	} 
      }

      readVect.clear();      
      readVectStr.clear();      
    }
    else{
      for(unsigned int i = 0; i < lineVect.size(); ++i){
	std::stringstream tempStream;
	tempStream << lineVect[i];
	unsigned int tempVal = 0;
	tempStream >> std::hex >> tempVal;
	
	readVect.push_back(tempVal);	
	readVectStr.push_back(lineVect[i]);
      }      
    }
  }

  if(doGlobalDebug) std::cout << "LINE: " << __LINE__ << std::endl;

  inFile.close();

  outFile_p->cd();

  Int_t totalFills = (nEvent+1)*nSample*nADCDataArr1;

  std::cout << "Total Fills (nEvent*nSample*nChannel): " << std::dec << nEvent+1 << "*" << std::dec << nSample << "*" << std::dec << nADCDataArr1 << std::endl;
  std::cout << "Coefficient: " << xValExpectedCoeff << std::endl;
  Float_t expectation = ((Float_t)totalFills)/xValExpectedCoeff;
  std::cout << "Median fill expectation: " << expectation << std::endl;
  
  for(Int_t bI = 0; bI < nBoards; ++bI){
    Double_t padSplit = 0.36;
    const Double_t leftMargin = 0.18;
    const Double_t bottomMargin = 0.125/padSplit;
    const Double_t topMargin = 0.02;
    const Double_t rightMargin = 0.02;
    Double_t height = 1800.0;
    Double_t width = height*(1.0 - topMargin*(1.0 - padSplit) - bottomMargin*padSplit)/(1.0 - leftMargin - rightMargin);

    TCanvas* canv_p = new TCanvas(("canv_" + std::to_string(bI) + "_p").c_str(), "", width, height);
    canv_p->SetTopMargin(0.001);
    canv_p->SetTopMargin(0.001);
    canv_p->SetTopMargin(0.001);
    canv_p->SetTopMargin(0.001);

    TPad* pads_p[3];
    pads_p[0] = new TPad("pad0", "", 0.0, padSplit, 1.0, 1.0);
    pads_p[0]->SetTopMargin(topMargin);
    pads_p[0]->SetRightMargin(rightMargin);
    pads_p[0]->SetLeftMargin(leftMargin);
    pads_p[0]->SetBottomMargin(0.001);

    canv_p->cd();
    pads_p[0]->Draw("SAME");
    pads_p[0]->cd();
    canv_p->cd();

    pads_p[1] = new TPad("pad1", "", 0.0, 0.0, 1.0, padSplit);
    pads_p[1]->SetTopMargin(0.001);
    pads_p[1]->SetRightMargin(rightMargin);
    pads_p[1]->SetLeftMargin(leftMargin);
    pads_p[1]->SetBottomMargin(bottomMargin);

    canv_p->cd();
    pads_p[1]->Draw("SAME");
    pads_p[1]->cd();
    canv_p->cd();

    pads_p[0]->cd();

    hist_p[bI]->SetMarkerColor(1);
    hist_p[bI]->SetLineColor(1);
    hist_p[bI]->SetMarkerSize(1.5);
    hist_p[bI]->SetMarkerStyle(20);

    histRat_p[bI]->SetMarkerColor(1);
    histRat_p[bI]->SetLineColor(1);
    histRat_p[bI]->SetMarkerSize(1.5);
    histRat_p[bI]->SetMarkerStyle(20);

    hist_p[bI]->GetYaxis()->SetTitleSize(0.035/(1.0 - padSplit));
    hist_p[bI]->GetYaxis()->SetLabelSize(0.035/(1.0 - padSplit));


    hist_p[bI]->SetMinimum(0.0);
    hist_p[bI]->SetMaximum(1.25*expectation);

    hist_p[bI]->GetYaxis()->SetTitleOffset(1.5);

    hist_p[bI]->DrawCopy("HIST P");
    gPad->SetTicks();
    gStyle->SetOptStat(0);

    TLine* line_p = new TLine();
    line_p->SetLineStyle(2);
    line_p->SetLineWidth(5);
    line_p->DrawLine(128, 0, 128, hist_p[bI]->GetMaximum());
    line_p->DrawLine(128*2, 0, 128*2, hist_p[bI]->GetMaximum());
    line_p->DrawLine(128*3, 0, 128*3, hist_p[bI]->GetMaximum());


    line_p->DrawLine(maxBitValExpectation - 128, 0, maxBitValExpectation - 128, hist_p[bI]->GetMaximum());
    line_p->DrawLine(maxBitValExpectation - 128*2, 0, maxBitValExpectation - 128*2, hist_p[bI]->GetMaximum());
    line_p->DrawLine(maxBitValExpectation - 128*3, 0, maxBitValExpectation - 128*3, hist_p[bI]->GetMaximum());

    line_p->SetLineStyle(1);
    line_p->SetLineWidth(5);
    line_p->SetLineColor(kRed);

    
    line_p->DrawLine(128*3, expectation, maxBitValExpectation - 128*3, expectation);
    line_p->DrawLine(128*2, 3.*expectation/4., 128*3, 3.*expectation/4.);
    line_p->DrawLine(128*1, expectation/2., 128*2, expectation/2.);
    line_p->DrawLine(0, expectation/4., 128*1, expectation/4.);

    line_p->DrawLine(maxBitValExpectation - 128*3, 3.*expectation/4., maxBitValExpectation - 128*2, 3.*expectation/4.);
    line_p->DrawLine(maxBitValExpectation - 128*2, expectation/2., maxBitValExpectation - 128*1, expectation/2.);
    line_p->DrawLine(maxBitValExpectation - 128*1, expectation/4., maxBitValExpectation, expectation/4.);

    canv_p->cd();
    pads_p[0]->cd();
    
    TLatex* label_p = new TLatex();
    label_p->SetNDC();
    label_p->SetTextFont(42);
    label_p->SetTextSize(0.035/(1.0 - padSplit));
    label_p->SetTextColor(kBlack);
    
    std::vector<std::string> labels = {"Token-passing", "Board: " + boardIDs[bI], "Tested w/:"};

    for(unsigned int bI2 = 0; bI2 < nBoards; ++bI2){
      if(bI == bI2) continue;
      labels.push_back("     " + boardIDs[bI2]);
    }

    labels.push_back("Date of test: ");
    labels.push_back("     " + dateOfTestStr);
    
    for(unsigned int lI = 0; lI < labels.size(); ++lI){
      label_p->DrawLatex(0.65, 0.65 - lI*0.065, labels[lI].c_str());
    }

    canv_p->cd();
    pads_p[2] = new TPad("pad2", "", 0.26, 0.39, 0.32, 0.51);
    pads_p[2]->SetTopMargin(0.001);
    pads_p[2]->SetLeftMargin(0.001);
    pads_p[2]->SetBottomMargin(0.001);
    pads_p[2]->SetRightMargin(0.001);
    pads_p[2]->Draw("SAME");
    pads_p[2]->cd();
    TImage* image_p = nullptr;
    if(testPass) image_p = TImage::Open("input/clipArt/greenLight.png");
    else image_p = TImage::Open("input/clipArt/redLight.png");

    image_p->SetImageQuality(TImage::kImgBest);
    image_p->Draw();

    canv_p->cd();
    pads_p[0]->cd();

    if(testPass) label_p->DrawLatex(0.33, 0.17, ("#color[" + std::to_string(buf.getColor(2)) + "]{Passed}").c_str()); 
    else label_p->DrawLatex(0.33, 0.17, "#color[2]{Failed}");

    if(insertFailCondition){
      label_p->DrawLatex(0.33, 0.17 - 0.065, "WARNING: Fail condition artificially injected");
      label_p->DrawLatex(0.33, 0.17 - 0.065*2, "Indicated failure not from board performance");
    }
    
    TLegend* leg_p = new TLegend(0.25, 0.45, 0.4, 0.65);
    leg_p->SetFillStyle(0);
    leg_p->SetBorderSize(0);
    leg_p->SetTextSize(0.035/(1.0 - padSplit));

    hist_p[bI]->SetMarkerSize(2);   
    leg_p->AddEntry(hist_p[bI], "Board Test Data", "P");
    leg_p->AddEntry(line_p, "Expectation", "L");
    
    leg_p->Draw("SAME");
    
    canv_p->cd();
    pads_p[1]->cd();

    std::cout << " " << histRat_p[bI]->GetNbinsX() << "-" << maxBitValExpectation+1 << std::endl;
    for(Int_t bIX = 0; bIX < maxBitValExpectation+1; ++bIX){
      Float_t denom = expectation;
            
      if(bIX < 128 || bIX > maxBitValExpectation - 128) denom /= 4.;
      else if(bIX < 128*2 || bIX > maxBitValExpectation - 128*2) denom /= 2.;
      else if(bIX < 128*3 || bIX > maxBitValExpectation - 128*3) denom *= 3./4.;      
      
      histRat_p[bI]->SetBinContent(bIX+1, ((Float_t)hist_p[bI]->GetBinContent(bIX+1))/denom);
      histRat_p[bI]->SetBinError(bIX+1, 0.0);
    }

    histRat_p[bI]->GetYaxis()->SetTitleSize(0.035/(padSplit));
    histRat_p[bI]->GetYaxis()->SetLabelSize(0.035/(padSplit));
    histRat_p[bI]->GetXaxis()->SetTitleSize(0.035/(padSplit));
    histRat_p[bI]->GetXaxis()->SetLabelSize(0.035/(padSplit));

    Float_t ratMax = 1.25;
    Float_t ratMin = 0.75;
    histRat_p[bI]->SetMaximum(ratMax);
    histRat_p[bI]->SetMinimum(ratMin);


    //    histRat_p[bI]->GetXaxis()->SetNdivisions(505);
    histRat_p[bI]->GetYaxis()->SetNdivisions(505);
    histRat_p[bI]->GetYaxis()->SetTitleOffset(1.5*padSplit/(1.0 - padSplit));
    histRat_p[bI]->DrawCopy("HIST E1 P");
    gPad->SetTicks();
    
    line_p->DrawLine(0, 1.0, maxBitValExpectation+1, 1.0);

    line_p->SetLineColor(1);
    line_p->SetLineStyle(2);
    
    line_p->DrawLine(128, ratMin, 128, ratMax);
    line_p->DrawLine(128*2, ratMin, 128*2, ratMax);
    line_p->DrawLine(128*3, ratMin, 128*3, ratMax);

    line_p->DrawLine(maxBitValExpectation - 128, ratMin, maxBitValExpectation - 128, ratMax);
    line_p->DrawLine(maxBitValExpectation - 128*2, ratMin, maxBitValExpectation - 128*2, ratMax);
    line_p->DrawLine(maxBitValExpectation - 128*3, ratMin, maxBitValExpectation - 128*3, ratMax);
   
    line_p->SetLineColor(kRed);
    line_p->SetLineStyle(1);
    line_p->SetLineWidth(5);

    canv_p->cd();
    //Quickly draw a box to get rid of a cutoff number
    TBox* box_p = new TBox();
    box_p->SetFillColor(0);
    box_p->DrawBox(0.12, 0.35, 0.175, 0.38);    
    delete box_p;

    std::string extraString = "";
    if(insertFailCondition) extraString = "_INSERTFAILTEST";
    
    std::string saveName = "pdfDir/" + dateStr + "/tokenPassing_Set" + metaDataMap["BOARDID"] + "_Board" + boardIDs[bI] + extraString;
    if(saveTag.size() != 0) saveName = saveName + "_" + saveTag;
    saveName = saveName + ".png";
    
    quietSaveAs(canv_p, saveName);

    delete pads_p[0];
    delete pads_p[1];
    delete pads_p[2];
    delete canv_p;
    //    delete image_p;
    
    delete leg_p;
    delete label_p;
    
    hist_p[bI]->Write("", TObject::kOverwrite);
    histRat_p[bI]->Write("", TObject::kOverwrite);
    delete hist_p[bI];
    delete histRat_p[bI];
    delete line_p;
  }
  
  outFile_p->Close();
  delete outFile_p;
  
  
  if(valueOutsideBitRange){
    std::cout << "WARNING - SOMEHOW YOU HAVE EXCEEDED 14 BITS: " << std::endl;
    std::cout << " Values found outside inclusive range (0-" << std::dec << maxBitVal << ")" << std::endl;
    std::cout << " Token passing test will fail" << std::endl;
  }

  //  
  //  Int_t 
  
  
  
  //Bit check:
  std::cout << "Checking that all bits are filled..." << std::endl;
  for(Int_t bI = 0; bI < nBoards; ++bI){
    std::cout << " Board " << std::dec << bI << "..." << std::endl;

    std::vector<std::string> unhitBoolVect;
    bool isPrevFalse = false;
    
    for(Int_t bI2 = 0; bI2 < maxBitVal+1; ++bI2){
      if(!bitHitBool[bI][bI2]){
	//	std::cout << "Bit \'" << std::dec << bI2 << "\' not hit. Test fail." << std::endl;
	//	if(bI2 <= maxBitValExpectation) testPass = false;

	if(!isPrevFalse) unhitBoolVect.push_back(std::to_string(bI2));
	else{
	  if(unhitBoolVect[unhitBoolVect.size()-1].find(",") != std::string::npos){
	    std::string newStr = unhitBoolVect[unhitBoolVect.size()-1];
	    newStr = newStr.substr(0, newStr.find(","));
	    newStr = newStr + "-" + std::to_string(bI2);
	    unhitBoolVect[unhitBoolVect.size()-1] = newStr;
	  }
	  else if(unhitBoolVect[unhitBoolVect.size()-1].find("-") != std::string::npos){
	    std::string newStr = unhitBoolVect[unhitBoolVect.size()-1];
	    newStr = newStr.substr(0, newStr.find("-"));
	    newStr = newStr + "-" + std::to_string(bI2);
	    unhitBoolVect[unhitBoolVect.size()-1] = newStr;
	  }
	  else{
	    unhitBoolVect[unhitBoolVect.size()-1]  = unhitBoolVect[unhitBoolVect.size()-1] + "," + std::to_string(bI2);
	  }
	}
	isPrevFalse = true;
      }
      else isPrevFalse = false;
    }

    if(unhitBoolVect.size() != 0){
      std::cout << " Token Pass test failed bit-fill for this board" << std::endl;
    }
    
    for(unsigned int bI2 = 0; bI2 < unhitBoolVect.size(); ++bI2){
      std::cout << "  Missed integers for: " << unhitBoolVect[bI2] << std::endl;
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "nEvent: " << std::dec << nEvent + 1 << std::endl;
  
  std::cout << std::endl;
  std::cout << "TOKEN PASSING PASS/FAIL: " << std::dec << testPass << std::endl;
  std::cout << std::endl;
  
  std::cout << "SPHENIXTOKENPASSING COMPLETE. return 0." << std::endl;
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/sphenixTokenPassing.exe <inConfigFileName>" << std::endl;
    std::cout << "TO DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=1 #from command line" << std::endl;
    std::cout << "TO TURN OFF DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=0 #from command line" << std::endl;
    std::cout << "return 1." << std::endl;
    return 1;
  }
 
  int retVal = 0;
  retVal += sphenixTokenPassing(argv[1]);
  return retVal;
}
