#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
//#include "probability.h"
#include "perModel.h"
#include "engine.h"
#include "matrix.h"
#include "tmwtypes.h"
#include "macro.h"
#include "setting.h"
#include "nodeInfo.h"
#include "Initialization.h"

extern int gNumOptimization;
extern double gTotalTimeOptimization;
extern int gNumHalfDuplex;
extern int gNumFullDuplex_J;
extern int gNumFullDuplex_K;
extern int gNumFullDuplex_J_K;
extern int gNumOFDMA;
extern int gNumOFDMAandFullDuplex;
extern Engine *gEp;
extern double *r;
extern double *pro;
extern double *dummyA;
extern double *A;
extern double u[NUM_STA*2];
//extern double dummyAeq[2][(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)];
extern double *Aeq;
extern double *beq;
extern double *lb;
extern double *ub;
extern nodeComb *gComb;
int gPosInfo[4][50];
extern simSpec gSpec;
extern int gNumComb;

int powint(double x, double y){
	double temp = pow(x, y);

	return (int)temp;
}

void makeCombination(staInfo sta[]){
	int numPos0 = 0;
	int numPos1 = 0;
	int numPos2 = 0;
	int numPos3 = 0;

	int numHalf, numFull, numOfdma, numOfdmaFull;

	int i, j ,k;
	int base;

	FILE *outputComb;
	char filenameComb[STR_MAX];

	bool fOutputPos = true;

	if(fOutputPos==true){
		sprintf(filenameComb, "%s", gSpec.filename);
		for(i=0; i<STR_MAX; i++){
			if(filenameComb[i]=='.'){
				filenameComb[i] = '\0';
				break;
			}
		}
		strcat(filenameComb, "_comb.txt");

		if((outputComb=fopen(filenameComb, "a"))==NULL){
			printf("Output file cannot be opened!\n");
			exit(33);
		}
	}

	for(i=0; i<NUM_STA; i++){
		if(sta[i].pos==0){
			numPos0++;
			gPosInfo[0][numPos0-1] = i+1;
		}else if(sta[i].pos==1){
			numPos1++;
			gPosInfo[1][numPos1-1] = i+1;
		}else if(sta[i].pos==2){
			numPos2++;
			gPosInfo[2][numPos2-1] = i+1;
		}else if(sta[i].pos==3){
			numPos3++;
			gPosInfo[3][numPos3-1] = i+1;
		}else{
			printf("STA position error\n");
		}
	}

	if(fOutputPos==true){
		fprintf(outputComb, "--- position 0 (%d STAs) ---\n", numPos0);
		for(i=0; i<numPos0; i++){
			fprintf(outputComb, "STA %d\n", gPosInfo[0][i]);
		}
		fprintf(outputComb, "\n--- position1 (%d STAs) ---\n", numPos1);
		for(i=0; i<numPos1; i++){
			fprintf(outputComb, "STA %d\n", gPosInfo[1][i]);
		}
		fprintf(outputComb, "\n--- position 2 (%d STAs) ---\n", numPos2);
		for(i=0; i<numPos2; i++){
			fprintf(outputComb, "STA %d\n", gPosInfo[2][i]);
		}
		fprintf(outputComb, "\n--- position 3 (%d STAs) ---\n", numPos3);
		for(i=0; i<numPos3; i++){
			fprintf(outputComb, "STA %d\n", gPosInfo[3][i]);
		}
	}


	numHalf = NUM_STA * 2;
	numFull = numPos0 * numPos2 * 2 + numPos1 * numPos3 * 2;
	numOfdma = NUM_STA * (NUM_STA - 1) / 2;
	numOfdmaFull = (numPos0 * (numPos0 - 1) * numPos2 + numPos1 * (numPos1 - 1) * numPos3 + numPos2 * (numPos2 - 1) * numPos0 + numPos3 * (numPos3 - 1) * numPos1) / 2;

	gNumComb = numHalf + numFull + numOfdma + numOfdmaFull;

	gComb = (nodeComb*)malloc(sizeof(nodeComb)*gNumComb);

	for(i=0; i<NUM_STA; i++){
			gComb[i].i = i+1;
			gComb[i].j = 0;
			gComb[i].k = 0;
			gComb[i].prtcl = half;
	}

	for(j=0; j<NUM_STA; j++){
			gComb[j+NUM_STA].i = 0;
			gComb[j+NUM_STA].j = j+1;
			gComb[j+NUM_STA].k = 0;
			gComb[j+NUM_STA].prtcl = half;
	}
	base = numHalf;

	for(j=0; j<numPos0; j++){
		for(k=0; k<numPos2; k++){
			gComb[base+numPos2*j+k].i = gPosInfo[0][j];
			gComb[base+numPos2*j+k].j = gPosInfo[2][k];
			gComb[base+numPos2*j+k].k = 0;
			gComb[base+numPos2*j+k].prtcl = full;
		}
	}
	base += numPos0 * numPos2;

	for(j=0; j<numPos2; j++){
		for(k=0; k<numPos0; k++){
			gComb[base+numPos0*j+k].i = gPosInfo[2][j];
			gComb[base+numPos0*j+k].j = gPosInfo[0][k];
			gComb[base+numPos0*j+k].k = 0;
			gComb[base+numPos0*j+k].prtcl = full;
		}
	}
	base += numPos0 * numPos2;

	for(j=0; j<numPos1; j++){
		for(k=0; k<numPos3; k++){
			gComb[base+numPos3*j+k].i = gPosInfo[1][j];
			gComb[base+numPos3*j+k].j = gPosInfo[3][k];
			gComb[base+numPos3*j+k].k = 0;
			gComb[base+numPos3*j+k].prtcl = full;
		}
	}
	base += numPos1 * numPos3;

	for(j=0; j<numPos3; j++){
		for(k=0; k<numPos1; k++){
			gComb[base+numPos1*j+k].i = gPosInfo[3][j];
			gComb[base+numPos1*j+k].j = gPosInfo[1][k];
			gComb[base+numPos1*j+k].k = 0;
			gComb[base+numPos1*j+k].prtcl = full;
		}
	}
	base += numPos1 * numPos3;

	for(i=0; i<NUM_STA; i++){
		for(j=i+1; j<NUM_STA; j++){
			gComb[base].i = 0;
			gComb[base].j = i+1;
			gComb[base].k = j+1;
			gComb[base].prtcl = ofdma;
			base++;
		}
	}

	if(base!=numHalf+numFull+numOfdma){
		printf("base error\n");
	}

	for(j=0; j<numPos0; j++){
		for(k=j+1; k<numPos0; k++){
			for(i=0; i<numPos2; i++){
				gComb[base].i = gPosInfo[2][i];
				gComb[base].j = gPosInfo[0][j];
				gComb[base].k = gPosInfo[0][k];
				gComb[base].prtcl = ofdmaFull;
				base++;
			}
		}
	}

	for(j=0; j<numPos1; j++){
		for(k=j+1; k<numPos1; k++){
			for(i=0; i<numPos3; i++){
				gComb[base].i = gPosInfo[3][i];
				gComb[base].j = gPosInfo[1][j];
				gComb[base].k = gPosInfo[1][k];
				gComb[base].prtcl = ofdmaFull;
				base++;
			}
		}
	}

	for(j=0; j<numPos2; j++){
		for(k=j+1; k<numPos2; k++){
			for(i=0; i<numPos0; i++){
				gComb[base].i = gPosInfo[0][i];
				gComb[base].j = gPosInfo[2][j];
				gComb[base].k = gPosInfo[2][k];
				gComb[base].prtcl = ofdmaFull;
				base++;
			}
		}
	}

	for(j=0; j<numPos3; j++){
		for(k=j+1; k<numPos3; k++){
			for(i=0; i<numPos1; i++){
				gComb[base].i = gPosInfo[1][i];
				gComb[base].j = gPosInfo[3][j];
				gComb[base].k = gPosInfo[3][k];
				gComb[base].prtcl = ofdmaFull;
				base++;
			}
		}
	}

	if(base!=gNumComb){
		printf("base is not match to gNumComb, %d, %d\n", base, gNumComb);
	}

	if(fOutputPos==true){
		fprintf(outputComb, "\n --- Combinations (%d combinations) ---\n", gNumComb);
		fprintf(outputComb, "*** Half-duplex (%d combinations) ***\n", numHalf);
		for(i=0; i<numHalf; i++){
			if(gComb[i].prtcl==half){
				fprintf(outputComb, "%d, %d, %d\n", gComb[i].i, gComb[i].j, gComb[i].k);
			}else{
				fprintf(outputComb, "%d, %d, %d, %d, error\n", gComb[i].i, gComb[i].j, gComb[i].k, gComb[i].prtcl);
			}
		}
		fprintf(outputComb, "\n*** Full-duplex (%d combinations) ***\n", numFull);
		for(; i<numHalf+numFull; i++){
			if(gComb[i].prtcl==full){
				fprintf(outputComb, "%d, %d, %d\n", gComb[i].i, gComb[i].j, gComb[i].k);
			}else{
				fprintf(outputComb, "%d, %d, %d, %d, error\n", gComb[i].i, gComb[i].j, gComb[i].k, gComb[i].prtcl);
			}
		}
		fprintf(outputComb, "\n*** OFDMA (%d combinatioins) ***\n", numOfdma);
		for(; i<numHalf+numFull+numOfdma; i++){
			if(gComb[i].prtcl == ofdma){
				fprintf(outputComb, "%d, %d, %d\n", gComb[i].i, gComb[i].j, gComb[i].k);
			}else{
				fprintf(outputComb, "%d, %d, %d, %d, error\n", gComb[i].i, gComb[i].j, gComb[i].k, gComb[i].prtcl);
			}
		}
		fprintf(outputComb, "\n *** OFDMA and Full-duplex (%d combinations)***\n", numOfdmaFull);
		for(; i<gNumComb; i++){
			if(gComb[i].prtcl == ofdmaFull){
				fprintf(outputComb, "%d, %d, %d\n", gComb[i].i, gComb[i].j, gComb[i].k);
			}else{
				fprintf(outputComb, "%d, %d, %d, %d, error\n", gComb[i].i, gComb[i].j, gComb[i].k, gComb[i].prtcl);
			}
		}
	}

	fclose(outputComb);
}

void solveLP(){
	int i;//, j, k;
	int tate = NUM_STA * 2;
	int yoko = gNumComb;
	char buffer[EP_BUFFER_SIZE] = {'\0'};

	struct timeval start, end;
	gettimeofday(&start, NULL);
	gNumOptimization++;

	processPrintf("Start solveLP\n");
	optimizationPrintf("Setting matrixes.\n");

	mxArray *mx_p = NULL;
	mxArray *mx_fval = NULL;
	mxArray *mx_r = NULL;
	mxArray *mx_A = NULL;
	mxArray *mx_u = NULL;
	mxArray *mx_Aeq = NULL;
	mxArray *mx_beq = NULL;
	mxArray *mx_lb = NULL;
	mxArray *mx_ub = NULL;
	mx_p = mxCreateDoubleMatrix(1, yoko, mxREAL);
	mx_fval = mxCreateDoubleMatrix(1, 1, mxREAL);
	double *p, *fval;

	processPrintf("Setting MATLAB matrices\n");

	mx_r = mxCreateDoubleMatrix(1, yoko, mxREAL);
	memcpy((void *)mxGetPr(mx_r), (void *)r, sizeof(double)*gNumComb);
	mx_A = mxCreateDoubleMatrix(tate, yoko, mxREAL);
	memcpy((void *)mxGetPr(mx_A), (void *)A, sizeof(double)*NUM_STA*2*gNumComb);
	mx_u = mxCreateDoubleMatrix(1, tate, mxREAL);
	memcpy((void *)mxGetPr(mx_u), (void *)u, sizeof(double)*NUM_STA*2);
	mx_Aeq = mxCreateDoubleMatrix(1, yoko, mxREAL);
	memcpy((void *)mxGetPr(mx_Aeq), (void *)Aeq, sizeof(double)*gNumComb);
	mx_beq = mxCreateDoubleMatrix(1, 1, mxREAL);
	memcpy((void *)mxGetPr(mx_beq), (void *)beq, sizeof(double));
	mx_lb = mxCreateDoubleMatrix(1, yoko, mxREAL);
	memcpy((void *)mxGetPr(mx_lb), (void *)lb, sizeof(double)*gNumComb);
	mx_ub = mxCreateDoubleMatrix(1, yoko, mxREAL);
	memcpy((void *)mxGetPr(mx_ub), (void *)ub, sizeof(double)*gNumComb);

	processPrintf("Allocate MATLAB matrices\n");

	engPutVariable(gEp, "mx_r", mx_r);
	engPutVariable(gEp, "mx_A", mx_A);
	engPutVariable(gEp, "mx_u", mx_u);
	engPutVariable(gEp, "mx_Aeq", mx_Aeq);
	engPutVariable(gEp, "mx_beq", mx_beq);
	engPutVariable(gEp, "mx_lb", mx_lb);
	engPutVariable(gEp, "mx_ub", mx_ub);
	engEvalString(gEp, "mx_beq=100;");

	engOutputBuffer(gEp, buffer, EP_BUFFER_SIZE);
	/*engEvalString(gEp, "mx_r");
	printf("%s", buffer);
	engEvalString(gEp, "mx_A");
	printf("%s", buffer);
	engEvalString(gEp, "mx_u");
	printf("%s", buffer);
	engEvalString(gEp, "mx_Aeq");
	printf("%s", buffer);
	engEvalString(gEp, "mx_beq=100");
	printf("%s", buffer);
	engEvalString(gEp, "mx_lb");
	printf("%s", buffer);
	engEvalString(gEp, "mx_ub");
	printf("%s", buffer);*/
	processPrintf("End setting matlab matrices\n");
	optimizationPrintf("Optimization starts.\n");

	engEvalString(gEp, "[p, fval] = linprog(mx_r, mx_A, mx_u, mx_Aeq, mx_beq, mx_lb, mx_ub);");
	//printf("%s", buffer);
	processPrintf("End linprog\n");
	optimizationPrintf("%s", buffer);
	engEvalString(gEp, "p = p ./ 100;");
	//printf("%s", buffer);
	engEvalString(gEp, "fval = fval / (-100);");
	mx_p = engGetVariable(gEp, "p");
	p = mxGetPr(mx_p);
	mx_fval = engGetVariable(gEp, "fval");
	fval = mxGetPr(mx_fval);

	processPrintf("Transform p[i] and pro[i][j][k]\n");
	for(i=0; i<yoko; i++){
		//printf("(%d, %d, %d)", (i%powint(NUM_STA+1,2))/(NUM_STA+1), (i%powint(NUM_STA+1,2))%(NUM_STA+1), i/powint(NUM_STA+1,2));
		if(p[i]>=0.000001){
			pro[i] = p[i];
		}else{
			pro[i] = 0;
		}
		//probabilityPrintf("%f, ", p[i]);
	}
	processPrintf("End optimization\n");
	probabilityPrintf("\n\n");
	optimizationPrintf("Optimization terminated.\n");
	probabilityPrintf("***** Probability *****\n");

	for(i=0; i<gNumComb; i++){
		probabilityPrintf("%d, %d, %d, %f\n", gComb[i].i, gComb[i].j, gComb[i].k, pro[i]);
	}

	for(i=0; i<yoko; i++){
		if(p[i]>0.00001){
			probabilityPrintf("\n   p[%d] = %f\n", i, p[i]);
		}
	}
	probabilityPrintf("   fval = %f\n", *fval);
	probabilityPrintf("***** Probability *****\n\n ");

	mxDestroyArray(mx_r);
	mxDestroyArray(mx_A);
	mxDestroyArray(mx_u);
	mxDestroyArray(mx_Aeq);
	mxDestroyArray(mx_beq);
	mxDestroyArray(mx_lb);
	mxDestroyArray(mx_ub);
	mxDestroyArray(mx_p);
	mxDestroyArray(mx_fval);
	//engEvalString(gEp, "close;");

	gettimeofday(&end, NULL);
	//printf("%d\n", end.tv_usec);
	//printf("%f\n", (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000);
	gTotalTimeOptimization += (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000;
}

void calculateProbability(staInfo sta[], apInfo *ap){
	int i, j;
	double delay[NUM_STA+1] = {};
	int no;
	//probability
	//calculateDelay
	if(gSpec.proMode==1||gSpec.proMode==2||gSpec.proMode==4){
		calculateDelay(ap, sta, delay);
	}
	calculateRSSI(ap, sta, delay);

	for(i=0; i<NUM_STA; i++){
		for(j=0; j<gNumComb; j++){
			if(gComb[j].i==i+1){
				dummyA[gNumComb*i+j] = -1;
			}else{
				dummyA[gNumComb*i+j] = 0;
			}
		}
	}
	for(; i<NUM_STA*2; i++){
		for(j=0; j<gNumComb; j++){
			if(gComb[j].j==i-NUM_STA+1||gComb[j].k==i-NUM_STA+1){
				dummyA[gNumComb*i+j] = -1;
			}else{
				dummyA[gNumComb*i+j] = 0;
			}
		}
	}
	for(no=0; no<gNumComb*NUM_STA*2; no++){
		i = no / gNumComb;
		j = no % gNumComb;
		A[NUM_STA*2*j+i] = dummyA[gNumComb*i+j];
	}

	solveLP();
}

/*void initializeMatrix(){
	int tate = NUM_STA * 2;
	int yoko = pow((NUM_STA+1), 3);
	int i, j, no;

	for(i=0; i<NUM_STA; i++){
		for(j=0; j<yoko; j++){
			if(((j%(int)pow(NUM_STA+1,2))/(NUM_STA+1)==i+1)&&((j%(int)pow(NUM_STA+1,2))%(NUM_STA+1)!=i+1)){
				dummyA[i][j] = -1;
			}else{
				dummyA[i][j] = 0;
			}
			matrixPrintf("%f ", dummyA[i][j]);
		}
		matrixPrintf("\n");
	}
	for(i=NUM_STA; i<NUM_STA*2; i++){
		for(j=0; j<yoko; j++){
			if(((j%(int)pow(NUM_STA+1,2))%(NUM_STA+1)==(i-NUM_STA+1))&&((j%(int)pow(NUM_STA+1,2))/(NUM_STA+1)!=(i-NUM_STA+1))){
				dummyA[i][j] = -1;
			}else if(j/(int)pow(NUM_STA+1,2)==i-NUM_STA+1 && (j%(int)pow(NUM_STA+1,2))%(NUM_STA+2)!=0){
				dummyA[i][j] = -1;
			}else{
				dummyA[i][j] = 0;
			}
			matrixPrintf("%f ", dummyA[i][j]);
		}
		matrixPrintf("\n");
	}
	for(j=0; j<yoko; j++){
		for(i=0; i<tate; i++){
			no = tate * j + i + 1;
			if(no%yoko!=0){
				A[no/yoko][no%yoko-1] = dummyA[i][j];
			}else{
				A[no/yoko-1][yoko-1] = dummyA[i][j];
			}
		}
	}

	tate = 2;

	for(j=0; j<yoko; j++){
		if((j%(int)pow(NUM_STA+1,2))/(NUM_STA+1)==(j%(int)pow(NUM_STA+1,2))%(NUM_STA+1)){
			dummyAeq[1][j] = 1;
		}else{
			dummyAeq[0][j] = 1;
		}
	}
	for(j=0; j<yoko; j++){
		for(i=0; i<tate; i++){
			no = tate * j + i + 1;
			if(no%yoko!=0){
				Aeq[no/yoko][no%yoko-1] = dummyAeq[i][j];
			}else{
				Aeq[no/yoko-1][yoko-1] = dummyAeq[i][j];
			}
		}
	}

	if(gSpec.proMode==5){
		for(i=0; i<NUM_STA*2; i++){
			u[i] = 0;
		}
	}else if(gSpec.proMode!=3&&gSpec.proMode!=4){
		for(i=0; i<NUM_STA*2; i++){
			u[i] = -100/(2*NUM_STA);
		}
	}else{
		for(i=0; i<NUM_STA*2; i++){
			if(i<NUM_STA){
				u[i] = -100/(2*NUM_STA);
			}else if(i<NUM_STA*2-gSpec.delaySTA){
				u[i] = -100/(2*NUM_STA) + gSpec.giveU;
				matrixPrintf("%f\n", u[i]);
			}else{
				u[i] = -100/(2*NUM_STA) - gSpec.giveU*(NUM_STA-gSpec.delaySTA)/gSpec.delaySTA;
				matrixPrintf("%f\n", u[i]);
			}
		}
	}
}*/

int selectNode(apInfo *ap, staInfo sta[], bool *fUpCollOne, bool *fUpCollSecond, bool *fNoUpOne, bool *fNoUpSecond, bool *fNoDownlink, int *upNodeOne, int *upNodeSecond, int *downNode){
	double *proDown;
	proDown = (double*)malloc(sizeof(double)*(NUM_STA+1));// = {};
	double *proUp;
	proUp = (double*)malloc(sizeof(double)*(NUM_STA+1));// = {};
	double *proTempDown;
	proTempDown = (double*)malloc(sizeof(double)*(NUM_STA+1));// = {};   //確率判定のため
	//int upNode, downNode;   //0がなし．1--NUM_STAまで．配列とずれてるから注意．
	double downRand;
	int i, j, k;
	int minBackoffOne = INT_MAX;
	int minBackoffSecond = INT_MAX;
	int maxMinBackoff;
	int dummyNode = INT_MAX;
	//int nodeIdRandom;
	double *tempUp;
	tempUp = (double*)malloc(sizeof(double)*(NUM_STA+1));
	int numUpOne = 0;
	int numUpSecond = 0;

	processPrintf("Start selectNode\n");
	//配列の初期化
	initializeDoubleArray(proDown, NUM_STA+1, 0);
	initializeDoubleArray(proUp, NUM_STA+1, 0);
	initializeDoubleArray(proTempDown, NUM_STA+1, 0);
	initializeDoubleArray(tempUp, NUM_STA+1, 0);

	/*if(gSpec.proMode==6){
		nodeIdRandom = rand() % (NUM_STA+1);
		for(i=1; i<=NUM_STA; i++){
			if(nodeIdRandom==0){
				*downNode = 0;
				selectionPrintf("Dummy STA is selected as a destination node.\n");
				*fNoDownlink = true;
				break;
			}
			if(i==nodeIdRandom){
				*downNode = i;
				sta[i-1].fRx = true;
				selectionPrintf("STA %d is selected as a destination node.\n", i-1);
			}else{
				sta[i-1].fRx = false;
			}
		}
		goto HALF;
	}*/

	//下り通信を受信する端末の決定
	selectionPrintf("***** Probability that each node is selected as a destination node of AP. *****\n");
	for(i=0; i<NUM_STA+1; i++){
		if(i!=0){
			proTempDown[i] += proTempDown[i-1];
		}
		for(j=0; j<gNumComb; j++){
			if(gComb[j].i==i){
				proDown[i] += pro[j];
			}
		}
		proTempDown[i] += proDown[i];
		selectionPrintf("p_d[%d] is %f.\n", i, proTempDown[i]);
	}

	if(proTempDown[NUM_STA]<=0.999 || 1.001<=proTempDown[NUM_STA]){
		printf("Probability is wrong.%f\n", proTempDown[NUM_STA]);
	}

	downRand = (double)rand() / RAND_MAX;
	selectionPrintf("downRand is %f\n", downRand);
	for(i=0; i<=NUM_STA; i++){
		if(i==0){
			if(downRand<=proTempDown[i]){
				*downNode = i;
				selectionPrintf("Dummy STA is selected as a destination node.\n");
				*fNoDownlink = true;
				break;
			}
		}else if(proTempDown[i-1]<downRand && downRand<=proTempDown[i]){
			*downNode = i;
			sta[i-1].fRx = true;
			selectionPrintf("STA %d is selected as a destination node.\n", i-1);
			break;
		}
		if(i==NUM_STA){
			printf("Error. downRand = %f\n", downRand);
			exit(654);
			*downNode = 0;
			*fNoDownlink = true;
		}
	}

	//HALF:
	/*if(gSpec.proMode==6){
		do{
			nodeIdRandom = rand() % (NUM_STA+1);
		}while(nodeIdRandom==*downNode);
		for(i=1; i<=NUM_STA; i++){
			if(nodeIdRandom==0){
				*upNode = 0;
				selectionPrintf("Dummy STA is selected as a source node.\n");
				*fNoUplink = true;
				calculatePhyRate(ap, sta, upNode, downNode);
				break;
			}
			if(i==nodeIdRandom){
				*upNode = i;
				sta[i-1].fTx = true;
				selectionPrintf("STA %d is selected as a source node.\n", i-1);
				numTx++;
				calculatePhyRate(ap, sta, upNode, downNode);
			}else{
				sta[i-1].fTx = false;
			}
		}
		goto ENDHALF;
	}*/

	//上り通信端末の選択
	selectionPrintf("***** Probabiliy that each node is selected as a source node of AP. *****\n");
	selectionPrintf("Select STA j\n");
	for(j=0; j<NUM_STA+1; j++){
		for(k=0; k<gNumComb; k++){
			if(gComb[k].i==*downNode&&(gComb[k].j==j||gComb[k].k==j)){
				tempUp[j] += pro[k];
			}
		}
	}
	for(j=0; j<NUM_STA+1; j++){
		if(*downNode==j){
			proUp[j] = 0;
		}else{
			proUp[j] = tempUp[j]/proDown[*downNode];
			//if(proUp[j]<0.000001){
				selectionPrintf("%f, %f\n", tempUp[j], proDown[*downNode]);
			//}*/
		}
		selectionPrintf("p_u[%d] is %f\n", j, proUp[j]);
	}
	selectionPrintf("\n\n");

	for(i=0; i<NUM_STA+1; i++){
		if(proUp[i]!=0){
			if(i==0){
				dummyNode = rand() % ((int)(1/proUp[i])+1);
			}else{
				sta[i-1].cw = (int)(1/proUp[i]);
				sta[i-1].backoffCount = rand() % (sta[i-1].cw+1);
				selectionPrintf("%f, %d ", proUp[i], sta[i-1].backoffCount);
			}
		}
		if(i!=0){
			selectionPrintf("sta[%d].cw = %d\n", i-1,  sta[i-1].cw);
		}
	}
	//selectionPrintf("numUpOne = %d\n", numUpOne);
	//selectionPrintf("\n\n");

	bool emptyOne = true;

	for(i=0; i<gSpec.numSta; i++){
		if(sta[i].buffer[0].lengthMsdu!=0){
			emptyOne = false;
		}
		if(proUp[i+1]!=0){
			if((minBackoffOne>sta[i].backoffCount)&&(sta[i].buffer[0].lengthMsdu!=0)){
				minBackoffOne = sta[i].backoffCount;
			}
		}
	}
	selectionPrintf("minBackoffOne %d\n", minBackoffOne);
	if(minBackoffOne==INT_MAX&&emptyOne==true){
		printf("All STAs don't have a frame.(epmtyOne)\n");   //フレームが無いときだけじゃないかも ダミーが選ばれる場合も
	}
	if(dummyNode<minBackoffOne){
		minBackoffOne = dummyNode;
		*upNodeOne = 0;
		*fNoUpOne = true;
	}else{
		for(i=0; i<NUM_STA; i++){
			if(proUp[i+1]!=0 && sta[i].backoffCount==minBackoffOne && sta[i].fRx==false){
				sta[i].fTxOne = true;
				numUpOne++;
				*upNodeOne = i+1;
				*fNoUpOne = false;
			}else{
				sta[i].fTxOne = false;
			}
		}
	}

	//Select STA k
	initializeDoubleArray(tempUp, NUM_STA+1, 0);
	dummyNode = INT_MAX;
	selectionPrintf("Select STA k\n");
	for(k=0; k<NUM_STA+1; k++){
		for(j=0; j<gNumComb; j++){
			if(gComb[j].i==*downNode&&(gComb[j].j==k||gComb[j].k==k)){
				tempUp[k] += pro[j];
			}
		}
	}
	for(k=0; k<NUM_STA+1; k++){
		if(*downNode==k){
			proUp[k] = 0;
		}else{
			proUp[k] = tempUp[k]/proDown[*downNode];
			//if(proUp[j]<0.000001){
				selectionPrintf("%f, %f ", tempUp[k], proDown[*downNode]);
			//}*/
		}
		selectionPrintf("p_u[%d] is %f\n", k, proUp[k]);
	}
	selectionPrintf("\n\n");

	for(i=0; i<NUM_STA+1; i++){
		if(proUp[i]!=0){
			if(i==0){
				dummyNode = rand() % ((int)(1/proUp[i])+1);
			}else{
				sta[i-1].cw = (int)(1/proUp[i]);
				sta[i-1].backoffCount = rand() % (sta[i-1].cw+1);
				selectionPrintf("%f, %d ", proUp[i], sta[i-1].backoffCount);
			}
		}
		if(i!=0){
			selectionPrintf("sta[%d].cw = %d\n", i-1, sta[i-1].cw);
		}
	}
	//selectionPrintf("numUpSecond = %d\n", numUpSecond);
	//selectionPrintf("\n\n");

	bool emptySecond = true;

	for(i=0; i<gSpec.numSta; i++){
		if(sta[i].buffer[0].lengthMsdu!=0){
			emptySecond = false;
		}
		if(proUp[i+1]!=0){
			if((minBackoffSecond>sta[i].backoffCount)&&(sta[i].buffer[0].lengthMsdu!=0)){
				minBackoffSecond = sta[i].backoffCount;
			}
		}
	}
	selectionPrintf("minBackoffSecond %d\n", minBackoffSecond);
	if(minBackoffSecond==INT_MAX&&emptySecond==true){
		printf("All STAs don't have a frame.(emptySecond)\n");   //フレームが無いときだけじゃないかも ダミーが選ばれる場合も
	}
	if(dummyNode<minBackoffSecond){
		minBackoffSecond = dummyNode;
		*upNodeSecond = 0;
		*fNoUpSecond = true;
	}else{
		for(i=0; i<NUM_STA; i++){
			if(proUp[i+1]!=0 && sta[i].backoffCount==minBackoffSecond && sta[i].fRx==false){
				sta[i].fTxSecond = true;
				numUpSecond++;
				*upNodeSecond = i+1;
				*fNoUpSecond = false;
			}else{
				sta[i].fTxSecond = false;
			}
		}
	}

	/*if(*upNodeOne==0 && *upNodeSecond==0){
		calculatePhyRate(ap, sta, upNodeOne, upNodeSecond, downNode);
	}else{
		for(i=0; i<gSpec.numSta; i++){
			if(proUp[i+1]!=0 && sta[i].backoffCount == minBackoffOne && sta[i].fRx==false){
				sta[i].fTxOne = true;
				//sta[i].backoffCount = rand() % (sta[i].cw + 1);
				numUpOne++;
				*upNodeOne = i+1;
				*fNoUpOne = false;
				//calculatePhyRate(ap, sta, upNode, downNode);
				selectionPrintf("STA %d has minimum backoff count.\n", i);
			}else if(proUp[i+1]!=0 && sta[i].backoffCount == minBackoffSecond && sta[i].fRx==false){
				sta[i].fTxSecond = true;
				//sta[i].backoffCount = rand() % (sta[i].cw + 1);
				numUpSecond++;
				*upNodeSecond = i+1;
				*fNoUpSecond = false;
			}else{
				//sta[i].backoffCount -= minBackoff;
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
			}
		}*/
	calculatePhyRate(ap, sta, upNodeOne, upNodeSecond, downNode);
	selectionPrintf("numUpOne %d, numUpSecond %d\n", numUpOne, numUpSecond);

	//ENDHALF:
	/*if((numUpOne==0 && *fNoUpOne==false) || (numUpSecond==0 && *fNoUpSecond==false)){
		printf("undefined\n");
	}*/
	if((numUpOne==0 && *fNoUpOne==true)||(numUpOne==1 && *fNoUpOne==false)){
		*fUpCollOne = false;
	}
	if((numUpSecond==0 && *fNoUpSecond==true)||(numUpSecond==1 && *fNoUpSecond==false)){
		*fUpCollSecond = false;
	}
	if(numUpOne>1){
		*fUpCollOne = true;
		//printf("\ncollision\n");
	}
	if(numUpSecond>1){
		*fUpCollSecond = true;
	}
	selectionPrintf("(%d, %d, %d),", *downNode, *upNodeOne, *upNodeSecond);

	if(numUpOne==1&&numUpSecond==1){
		ratePrintf("\n(%d, %d, %d),\n", *downNode, *upNodeOne, *upNodeSecond);
	}

	if(*downNode!=0&&*upNodeOne==0&&*upNodeSecond==0){
		gNumHalfDuplex++;
	}else if(*downNode==0&&*upNodeOne!=0&&*upNodeSecond==0){
		gNumHalfDuplex++;
	}else if(*downNode==0&&*upNodeOne==0&&*upNodeSecond!=0){
		gNumHalfDuplex++;
	}else if(*downNode!=0&&*upNodeOne!=0&&*upNodeSecond==0){
		gNumFullDuplex_J++;
	}else if(*downNode!=0&&*upNodeOne==0&&*upNodeSecond!=0){
		gNumFullDuplex_K++;
	}else if(*downNode==0&&*upNodeOne!=0&&*upNodeSecond!=0){
		if(*upNodeOne==*upNodeSecond){
			gNumHalfDuplex++;
		}else{
			gNumOFDMA++;
		}
	}else if(*downNode!=0&&*upNodeOne!=0&&*upNodeSecond!=0){
		if(*upNodeOne==*upNodeSecond){
			gNumFullDuplex_J_K++;
		}else{
			gNumOFDMAandFullDuplex++;
		}
	}else{
		printf("Selection error\n");
	}

	free(proUp);
	free(proDown);
	free(proTempDown);
	free(tempUp);

	if(minBackoffOne==INT_MAX){
		minBackoffOne = 0;
	}
	if(minBackoffSecond==INT_MAX){
		minBackoffSecond = 0;
	}
	if(minBackoffOne>minBackoffSecond){
		maxMinBackoff = minBackoffOne;
	}else{
		maxMinBackoff = minBackoffSecond;
	}

	processPrintf("end selectNode\n");

	return maxMinBackoff;
}
