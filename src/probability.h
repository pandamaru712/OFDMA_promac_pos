#ifndef _probability_h
#define _probability_h

#include "nodeInfo.h"
#include "setting.h"
#include "nodeInfo.h"

int powint(double, double);
int makeCombination(staInfo*);
void outputRate(void);
int selectNode(apInfo*, staInfo*, bool*, bool*, bool*, bool*, bool*, int*, int*, int*, protocolDetail*);
void calculateProbability(staInfo*, apInfo*);
void initializeMatrix(void);
void solveLP(void);

#endif
