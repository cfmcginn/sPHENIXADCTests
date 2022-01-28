//2021.02.26; Author Chris McGinn; Contact at chmc7718@colorado.edu or 'cffionn' on skype

#ifndef FITUTIL_H
#define FITUTIL_H

#include <cmath>

//Copied directly w/ modest stylistic edits from:
//https://github.com/sPHENIX-Collaboration/coresoftware/blob/master/offline/packages/tpcdaq/TPCDaqDefs.cc#L297
//https://github.com/sPHENIX-Collaboration/coresoftware/pull/415

inline int nParam_SignalShape_PowerLawDoubleExp(){return 7;} //hardcoded but below goes up to par[6]

inline double SignalShape_PowerLawDoubleExp(double *x, double *par)
{
  double pedestal = par[4];
  if(x[0] < par[1]) return pedestal;

  double signal = par[0]*std::pow((x[0] - par[1]), par[2])*(((1. - par[5]) / pow(par[3], par[2]) * exp(par[2])) * exp(-(x[0] - par[1]) * (par[2] / par[3])) + (par[5] / pow(par[6], par[2]) * exp(par[2])) * exp(-(x[0] - par[1]) * (par[2] / par[6])) );
  return pedestal + signal;
}

#endif
