/*
 * main.cpp
 *
 @Created on: Mar 22, 2015
 * @Author: Tadeze
 * Main entry: accepts the the
 * @param argv
 *    Usage: iforest [options]
 *      -i FILE, --infile=FILE
 Specify path to input data file. (Required).
 -o FILE, --outfile=FILE
 Specify path to output results file. (Required).
 -m COLS, --metacol=COLS
 Specify columns to preserve as meta-data. (Separated by ',' Use '-' to specify ranges).
 -t N, --ntrees=N
 Specify number of trees to build.
 Default value is 100.
 -s S, --sampsize=S
 Specify subsampling rate for each tree. (Value of 0 indicates to use entire data set).
 Default value is 2048.
 -d MAX, --maxdepth=MAX
 Specify maximum depth of trees. (Value of 0 indicates no maximum).
 Default value is 0.
 -H, --header
 Toggle whether or not to expect a header input.
 Default value is true.
 -v, --verbose
 Toggle verbose ouput.
 Default value is false.
 -h, --help
 Print this help message and exit.
 */

#include "main.hpp"
#include<cfloat>
#include<ctime>
using namespace std;
//log file
//ofstream logfile("treepath.csv");

void deletedoubleframe(doubleframe *df) {
	for (int ii = 0; ii < df->nrow; ++ii) {
		delete[] df->data[ii];
	}
	delete[] df->data;
	delete df;
}

doubleframe *copyNormalInstances(const doubleframe *dtOrg, const ntstringframe* metadata) {
	int cntNormal = 0, cnt = 0;
	for(int i = 0; i < metadata->nrow; ++i){
		if(!strcmp(metadata->data[i][0], "nominal"))
			++cntNormal;
	}
	std::cout << "Number of normals: " << cntNormal << std::endl;
	doubleframe *dtOn = new doubleframe();
//	dtOn->colnames = dtOrg->colnames;
	dtOn->column_major = dtOrg->column_major;
	dtOn->ncol = dtOrg->ncol;
	dtOn->nrow = cntNormal;
//	dtOn->rownames = dtOrg->rownames;
	dtOn->data = new double *[dtOn->nrow];
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		dtOn->data[ii] = new double[dtOn->ncol];
	}
	for (int ii = 0; ii < metadata->nrow; ++ii) {
		if(!strcmp(metadata->data[ii][0], "nominal")){
			for (int jj = 0; jj < dtOn->ncol; ++jj) {
				dtOn->data[cnt][jj] = dtOrg->data[ii][jj];
			}
			++cnt;
		}
	}
	return dtOn;
}

doubleframe *copyAnomalyInstances(const doubleframe *dtOrg, const ntstringframe* metadata) {
	int cntAnomaly = 0, cnt = 0;
	for(int i = 0; i < metadata->nrow; ++i){
		if(!strcmp(metadata->data[i][0], "anomaly"))
			++cntAnomaly;
	}
	std::cout << "Number of anomaly: " << cntAnomaly << std::endl;
	doubleframe *dtOn = new doubleframe();
//	dtOn->colnames = dtOrg->colnames;
	dtOn->column_major = dtOrg->column_major;
	dtOn->ncol = dtOrg->ncol;
	dtOn->nrow = cntAnomaly;
//	dtOn->rownames = dtOrg->rownames;
	dtOn->data = new double *[dtOn->nrow];
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		dtOn->data[ii] = new double[dtOn->ncol];
	}
	for (int ii = 0; ii < metadata->nrow; ++ii) {
		if(!strcmp(metadata->data[ii][0], "anomaly")){
			for (int jj = 0; jj < dtOn->ncol; ++jj) {
				dtOn->data[cnt][jj] = dtOrg->data[ii][jj];
			}
			++cnt;
		}
	}
	return dtOn;
}

doubleframe *copySelectedRows(const doubleframe *dtOrg, std::vector<int> idx, int from, int to) {
	doubleframe *dtOn = new doubleframe();
//	dtOn->colnames = dtOrg->colnames;
	dtOn->column_major = dtOrg->column_major;
	dtOn->ncol = dtOrg->ncol;
	dtOn->nrow = to - from + 1;
//	dtOn->rownames = dtOrg->rownames;
	dtOn->data = new double *[dtOn->nrow];
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		dtOn->data[ii] = new double[dtOn->ncol];
	}
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		for (int jj = 0; jj < dtOn->ncol; ++jj) {
			dtOn->data[ii][jj] = dtOrg->data[idx[from + ii]][jj];
		}
	}
	return dtOn;
}


doubleframe *copyRows(const doubleframe *dtOrg, int from, int to) {
	doubleframe *dtOn = new doubleframe();
//	dtOn->colnames = dtOrg->colnames;
	dtOn->column_major = dtOrg->column_major;
	dtOn->ncol = dtOrg->ncol;
	dtOn->nrow = to - from + 1;
//	dtOn->rownames = dtOrg->rownames;
	dtOn->data = new double *[dtOn->nrow];
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		dtOn->data[ii] = new double[dtOn->ncol];
	}
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		for (int jj = 0; jj < dtOn->ncol; ++jj) {
			dtOn->data[ii][jj] = dtOrg->data[from + ii][jj];
		}
	}
	return dtOn;
}

doubleframe *copyCols(const doubleframe *dtOrg, int from, int to) {
	doubleframe *dtOn = new doubleframe();
//	dtOn->colnames = dtOrg->colnames;
	dtOn->column_major = dtOrg->column_major;
	dtOn->ncol = to - from + 1;
	dtOn->nrow = dtOrg->nrow;
//	dtOn->rownames = dtOrg->rownames;
	dtOn->data = new double *[dtOn->nrow];
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		dtOn->data[ii] = new double[dtOn->ncol];
	}
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		for (int jj = 0; jj < dtOn->ncol; ++jj) {
			dtOn->data[ii][jj] = dtOrg->data[ii][from + jj];
		}
	}
	return dtOn;
}

std::vector<int> getRandomIdx(int nsample, int nrow) {
	std::vector<int> sampleIndex;
	if(nsample > nrow || nsample <= 0)
		nsample = nrow;
	int *rndIdx = new int[nrow];
	for(int i = 0; i < nrow; ++i)
		rndIdx[i] = i;
	for(int i = 0; i < nsample; ++i){
		int r = std::rand() % nrow;
		int t = rndIdx[i];
		rndIdx[i] = rndIdx[r];
		rndIdx[r] = t;
	}
	for(int i = 0; i < nsample; ++i){
		sampleIndex.push_back(rndIdx[i]);
	}
	delete []rndIdx;
	return sampleIndex;
}

doubleframe *createTrainingSet(doubleframe *dtTrainNorm, doubleframe *dtTrainAnom, int numNorm, int numAnom){
	doubleframe *dtOn = new doubleframe();

	dtOn->column_major = dtTrainNorm->column_major;
	dtOn->ncol = dtTrainNorm->ncol;
	dtOn->nrow = numNorm + numAnom;

	dtOn->data = new double *[dtOn->nrow];
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		dtOn->data[ii] = new double[dtOn->ncol];
	}
	std::vector<int> nidx = getRandomIdx(numNorm, dtTrainNorm->nrow);
	std::vector<int> aidx = getRandomIdx(numAnom, dtTrainAnom->nrow);
	int i = 0, j = 0;
	for (int ii = 0; ii < dtOn->nrow; ++ii) {
		int normal = std::rand() % (numNorm + numAnom);
		if(normal >= numAnom && i >= (int)nidx.size())
			normal = 0;
		if(normal < numAnom && j >= (int)aidx.size())
			normal = numAnom;
		for (int jj = 0; jj < dtOn->ncol; ++jj) {
			if(normal >= numAnom)
				dtOn->data[ii][jj] = dtTrainNorm->data[nidx[i]][jj];
			else
				dtOn->data[ii][jj] = dtTrainAnom->data[aidx[j]][jj];
		}
		if(normal >= numAnom) ++i;
		else ++j;
	}
	return dtOn;
}

std::vector<int> getRestIdx(std::vector<int> idx, int nrow){
	std::vector<int> restidx;
	bool track[nrow];
	for(int i = 0; i < nrow; ++i)
		track[i] = true;
	for(int i = 0; i < (int)idx.size(); ++i)
		track[idx[i]] = false;
	for(int i = 0; i < nrow; ++i){
		if(track[i] == true)
			restidx.push_back(i);
	}
	return restidx;
}

int sidx = 0;
struct sItem {
	double *p;
	ntstring *q;
};
int cmp(sItem *a, sItem *b) {
	if (a->p[sidx] > b->p[sidx])
		return 1;
	if (a->p[sidx] < b->p[sidx])
		return -1;
	return 0;
}

void printData(int fid, doubleframe* dt, ntstringframe* metadata) {
	char fname[100];
	sprintf(fname, "data%d.csv", fid + 1);
	std::ofstream out(fname);
	for (int i = 0; i < dt->nrow; ++i) {
		if(metadata)
			out << metadata->data[i][0];
		for (int j = 0; j < dt->ncol; ++j) {
			out << "," << dt->data[i][j];
		}
		out << std::endl;
	}
	out.close();
}
void sortByFeature(int fid, doubleframe* dt, ntstringframe* metadata) {
	sidx = fid;
	sItem *tp = new sItem[dt->nrow];
	for (int i = 0; i < dt->nrow; ++i) {
		tp[i].p = dt->data[i];
		tp[i].q = metadata->data[i];
	}
	qsort(tp, dt->nrow, sizeof(tp[0]),
			(int (*)(const void *, const void *))cmp);
	for(int i = 0; i < dt->nrow; ++i) {
		dt->data[i] = tp[i].p;
		metadata->data[i] = tp[i].q;
	}
	delete[] tp;
}

// randomly shuffle dt along with metadata
void randomShuffle(doubleframe* dt, ntstringframe* metadata) {
	for (int ii = 0; ii < dt->nrow; ++ii) {
		int ridx = rand() % dt->nrow;
		if (metadata) {
			ntstring *t = metadata->data[ii];
			metadata->data[ii] = metadata->data[ridx];
			metadata->data[ridx] = t;
		}
		double *t = dt->data[ii];
		dt->data[ii] = dt->data[ridx];
		dt->data[ridx] = t;
	}
}

void printScoreToFile(vector<double> &scores, const ntstringframe* metadata,
		char fName[]) {
	ofstream outscore;
	outscore.open(fName);
	outscore << "groundtruth,score\n";
	for (int j = 0; j < (int) scores.size(); j++) {
		if (metadata) {
			outscore << metadata->data[j][0] << ",";
		}
		outscore << scores[j] << "\n";
	}
	outscore.close();
}

struct Obj{
    int idx;
    double score;
};
int ObjCmp(Obj *a, Obj *b){
    if(a->score > b->score) return 1;
    if(a->score < b->score) return -1;
    return 0;
}

int printNoFeedbackAnomCntToFile(const vector<double> &scores, const ntstringframe* metadata,
		ofstream &outscore, int numFeed) {
    Obj *idx = new Obj[scores.size()];
	for (int i = 0; i < (int) scores.size(); i++) {
        idx[i].idx = i;
        idx[i].score = scores[i];
    }
    qsort(idx, scores.size(), sizeof(idx[0]),
			(int (*)(const void *, const void *))ObjCmp);

    int numAnom = 0;
	for (int i = 0; i < numFeed; i++) {
		if(strcmp(metadata->data[idx[i].idx][0], "anomaly") == 0)
			numAnom++;
		outscore << "," << numAnom;
	}
	delete []idx;
	return numAnom;
}


void printScoreToFile(const vector<double> &scores, ntstringframe* csv,
		const ntstringframe* metadata, const doubleframe *dt, char fName[]) {
	ofstream outscore;
	outscore.open(fName);
	// print header
	for(int i = 0; i < metadata->ncol; ++i)
		outscore << metadata->colnames[i] << ",";
	for(int i = 0; i < csv->ncol; ++i)
        outscore << csv->colnames[i] << ",";

	outscore << "anomaly_score\n";

    Obj *idx = new Obj[scores.size()];
	for (int i = 0; i < (int) scores.size(); i++) {
        idx[i].idx = i;
        idx[i].score = scores[i];
    }
    qsort(idx, scores.size(), sizeof(idx[0]),
			(int (*)(const void *, const void *))ObjCmp);

	for (int i = 0; i < (int) scores.size(); i++) {
		for(int j = 0; j < metadata->ncol; ++j)
			outscore << metadata->data[idx[i].idx][j] << ",";
		for(int j = 0; j < dt->ncol; ++j)
			outscore << dt->data[idx[i].idx][j] << ",";
		outscore << scores[idx[i].idx] << "\n";
	}
	delete []idx;
	outscore.close();
}

void explanationFeedback(doubleframe* dt, ntstringframe* metadata,
		IsolationForest &iff, std::vector<double> scores, int iter, char type[], char out_name[], int numfeed){
	char fname[100];
	int **freq = new int *[dt->ncol];
	bool **marginalize = new bool *[dt->ncol];
	for(int i = 0; i < dt->ncol; i++){
		freq[i]	= new int[4];
		marginalize[i] = new bool[4];
		for(int j = 0; j < 4; j++)
			marginalize[i][j] = false;
	}

	for(int rep = 0; rep < numfeed; rep++){
		for(int i = 0; i < dt->ncol; i++)
			for(int j = 0; j < 4; j++)
				freq[i][j] = 0;

		for(int top = 0; top < 1; top++){
			double max = -1;
			int midx = -1;
			for(int i = 0; i < (int)scores.size(); i++){
				if(scores[i] > max && strcmp(metadata->data[i][0], "nominal") == 0){
					max = scores[i];
					midx = i;
				}
			}

			scores[midx] = -1;

			std::vector<int> expl;
			if(strcmp(type,"seq_marg") == 0)
				expl = iff.getSeqMarExplanation(dt->data[midx], dt->ncol, marginalize, 1);
			else if(strcmp(type,"seq_drop") == 0)
				expl = iff.getSeqDropExplanation(dt->data[midx], dt->ncol, marginalize, 1);
			else if(strcmp(type,"rev_seq_marg") == 0)
				expl = iff.getRevSeqMarExplanation(dt->data[midx], dt->ncol, marginalize);
			else if(strcmp(type,"rev_seq_drop") == 0)
				expl = iff.getRevSeqDropExplanation(dt->data[midx], dt->ncol, marginalize);

			// Disregard this instance if no explanation found
			if(expl.size() == 0) continue;

			int tf = expl[0];
			int q = Tree::getQuantile(tf, dt->data[midx][tf]);
			freq[tf][q]++;
		}

		int marg_feat = -1, q = -1;
		int max = 0;
		for(int i = 0; i < dt->ncol; i++){
			for(int j = 0; j < 4; j++){
				if(freq[i][j] > max){
					max = freq[i][j];
					marg_feat = i;
					q = j;
				}
			}
		}

		// stop if no feedback to give
		if(marg_feat < 0 || q < 0) break;

		marginalize[marg_feat][q] = true;
		scores = iff.AnomalyScore(dt, marginalize);
		std::cout <<"Feedback " << rep+1 << ": Removed " << marg_feat << " " << q << "\n" << std::flush;
		sprintf(fname, "%s_%s_iter%d_%d.csv", out_name, type, iter+1, rep+1);
		printScoreToFile(scores, metadata, fname);
	}

	std::cout << "\n";

	for(int i = 0; i < dt->ncol; i++){
		delete []freq[i];
		delete []marginalize[i];
	}
	delete []freq;
	delete []marginalize;
}

double getQthPercentileScore(const std::vector<double> &scores, double q){
	double *x = new double[scores.size()];
	for(int i = 0; i < (int)scores.size(); i++)
		x[i] = scores[i];
	qsort(x, sizeof(x[0]), scores.size(), (int (*)(const void *, const void *))dblcmp);
	int k = (int)ceil(scores.size()*q);
	double quant = x[scores.size()-k];
	delete []x;
	return quant;
}

// Normalize scores to make an anomaly distribution
void normalizeScore(std::vector<double> &scores, std::vector<double> &scoresNorm){
	double Z = 0, MIN = scores[0];
	for(int i = 1; i < (int)scores.size(); i++){
		if(scores[i] < MIN) MIN = scores[i];
	}
	for(int i = 0; i < (int)scores.size(); i++){
		scoresNorm[i] = exp(MIN - scores[i]);
		Z += scoresNorm[i];
	}
	for(int i = 0; i < (int)scores.size(); i++)
		scoresNorm[i] = scoresNorm[i] / Z;
}

double getLinearLoss(int x, const ntstringframe* metadata, std::vector<double> &scores){
	int y = (strcmp(metadata->data[x][0], "anomaly") == 0) ? 1 : -1;
	double loss = y * scores[x];
	return loss;
}
double getLinearLoss(std::vector<int> x, const ntstringframe* metadata, std::vector<double> &scores){
	double loss = 0;
	for(int i = 0; i < (int)x.size(); i++){
		loss += getLinearLoss(x[i], metadata, scores);
	}
	loss /= x.size();
	return loss;
}


double getLoglikelihoodLoss(int x, const ntstringframe* metadata, std::vector<double> &scoresNorm){
	int y = (strcmp(metadata->data[x][0], "anomaly") == 0) ? 1 : -1;
	double loss = (-y) * log(scoresNorm[x]+1e-15);
	return loss;
}

double getLoglikelihoodLoss(std::vector<int> x, const ntstringframe* metadata, std::vector<double> &scoresNorm){
	double loss = 0;
	for(int i = 0; i < (int)x.size(); i++){
		loss += getLoglikelihoodLoss(x[i], metadata, scoresNorm);
	}
	loss /= x.size();
	return loss;
}


double getLogisticLoss(int x, const ntstringframe* metadata, std::vector<double> &scores){
	int y = (strcmp(metadata->data[x][0], "anomaly") == 0) ? 1 : -1;
	double loss = y * scores[x];
	if(loss > 100)
		return loss;
	else
		return log(1 + exp(loss));
}

double getLogisticLoss(std::vector<int> x, const ntstringframe* metadata, std::vector<double> &scores){
	double loss = 0;
	for(int i = 0; i < (int)x.size(); i++){
		loss += getLogisticLoss(x[i], metadata, scores);
	}
	loss /= x.size();
	return loss;
}

std::vector<int> rndVec[100];// need to allocate memory if feedback greater than 100
void initRndVec(){
	for(int s = 1; s <= 100; s++){
		for(int i = 0; i < s; i++){
			rndVec[s-1].push_back(rand() % s);
		}
	}
}
void shuffle(std::vector<int> &feedbackIdx){
	for(int j = 0; j < (int)feedbackIdx.size(); j++){
		int rnd = rndVec[feedbackIdx.size()-1][j];
		int tmp = feedbackIdx[j];
		feedbackIdx[j] = feedbackIdx[rnd];
		feedbackIdx[rnd] = tmp;
	}
}

double getDifference(std::vector<double> &w0, std::vector<double> &w1, bool &first, IsolationForest &iff){
	if(first){
		w0.clear();
		iff.getCurrentWeights(w0);
		first = false;
	}else{
		w1.clear();
		iff.getCurrentWeights(w1);
		first = true;
	}
	double diff = 0;
	for(int k = 0; k < (int)w0.size(); k++){
		diff += fabs(w0[k] - w1[k]);
	}
	return diff;
}

template<class T,int N>
int mains(int argc, char* argv[], T (&re)[N]) {
	std::time_t st = std::time(nullptr);
	srand(0); //randomize for random number generator.
	initRndVec();
	// parse argument from command line
	parsed_args* pargs = parse_args(argc, argv);
	ntstring input_name = pargs->input_name;
	ntstring output_name = pargs->output_name;
	d(int)* metacol = pargs->metacol;
	int ntree = pargs->ntrees;
	int nsample = pargs->sampsize;
	int maxheight = pargs->maxdepth;
	bool header = pargs->header;
//	bool verbose = pargs->verbose;
	bool rsample = nsample != 0;

	bool PRINT = true;
	// Feedback Parameters
	int numIter = pargs->numIteration;
	int numFeedback = pargs->numFeedback;
	int numGradUpd = pargs->numGradUpd;
	int lossType = pargs->lossType;
	int updateType = pargs->updateType;
	double REG = pargs->REG_PARAM;
	double LRATE = pargs->learningRate;
	bool variableLearningRate = false;
	if(LRATE == 0)
		variableLearningRate = true;
	if(pargs->posWeight == 1)
		Tree::POS_WEIGHT_ONLY = true;
	int reInitWeights = pargs->reInitWeights;
	Tree::KEEP_NEG_BUT_USE_0 = (reInitWeights == 1);
	int regularizerType = pargs->regularizerType;
	int MAXGRADITER = 100;

	char typeLoss[100], typeUpdate[100];
	if(lossType == 0)
		strcpy(typeLoss, "linear");
	else if(lossType == 1)
		strcpy(typeLoss, "loglikelihood");
	else if(lossType == 2)
		strcpy(typeLoss, "logistic");

	if(updateType == 0)
		strcpy(typeUpdate, "online");
	else if(updateType == 1)
		strcpy(typeUpdate, "stochastic");
	else if(updateType == 2)
		strcpy(typeUpdate, "batch");

	ntstringframe* csv = read_csv(input_name, header, false, false);
	ntstringframe* metadata = split_frame(ntstring, csv, metacol, true);
	doubleframe* dt = conv_frame(double, ntstring, csv); //read data to the global variable
	nsample = nsample == 0 ? dt->nrow : nsample;

	// std::cout << "# Trees          = " << ntree << std::endl;
	// std::cout << "# Samples        = " << nsample << std::endl;
	// std::cout << "MaxHeight        = " << maxheight << std::endl;
	// std::cout << "Orig Dimension   = " << dt->nrow << "," << dt->ncol << std::endl;
	// std::cout << "# Iterations     = " << numIter << std::endl;
	// std::cout << "# Feedbacks      = " << numFeedback << std::endl;
	// std::cout << "Loss   type      = " << typeLoss << std::endl;
	// std::cout << "Update type      = " << typeUpdate << std::endl;
	// std::cout << "Num Grad Upd     = " << numGradUpd << std::endl;
	// std::cout << "Reg. Constant    = " << REG << std::endl;
	// std::cout << "Learning Rate    = " << LRATE << std::endl;
	// std::cout << "Variable LRate   = " << variableLearningRate << std::endl;
	// std::cout << "Positive W only  = " << Tree::POS_WEIGHT_ONLY << std::endl;
	// std::cout << "ReInitWgts       = " << reInitWeights << std::endl;
	// std::cout << "Regularizer type = L" << regularizerType << std::endl;

//	char treeFile[1000];
//	char fname[1000];
	char statFile[1000], statNoFeed[1000];
	double costBefore[1000], costAfter[1000], avgcostBefore[1000], avgcostAfter[1000];//need dynamic memory allocation for feedback > 1000
//	sprintf(treeFile, "%s_tree_%s.txt", output_name, type);
	sprintf(statFile,   "%s_summary_feed_%d_losstype_%s_updatetype_%s_ngrad_%d_reg_%g_lrate_%g_pwgt_%d_inwgt_%d_rtype_L%d.csv",
			output_name, numFeedback, typeLoss, typeUpdate, numGradUpd, REG, LRATE, Tree::POS_WEIGHT_ONLY, reInitWeights, regularizerType);
	sprintf(statNoFeed, "%s_summary_feed_%d_losstype_%s_updatetype_%s_ngrad_%d_reg_%g_lrate_%g_pwgt_%d_inwgt_%d_rtype_L%d.csv",
			output_name,           0, typeLoss, typeUpdate, numGradUpd, REG, LRATE, Tree::POS_WEIGHT_ONLY, reInitWeights, regularizerType);
//	ofstream tree(treeFile);
	ofstream stats(statFile), statsNoFeed(statNoFeed);
	stats << "iter";
	statsNoFeed << "iter";
	for(int i = 0; i < numFeedback; i++){
		stats << ",feed" << i+1;
		statsNoFeed << ",feed" << i+1;
	}
	stats << "\n";
	statsNoFeed << "\n";

	char costFile[1000];
	sprintf(costFile, "%s_cost_feed_%d_losstype_%s_updatetype_%s_ngrad_%d_reg_%g_lrate_%g_pwgt_%d_inwgt_%d_rtype_L%d.csv",
			output_name, numFeedback, typeLoss, typeUpdate, numGradUpd, REG, LRATE, Tree::POS_WEIGHT_ONLY, reInitWeights, regularizerType);
	ofstream costs(costFile);
	costs << "iter";
	for(int i = 0; i < numFeedback; i++){
		costs << ",feed" << i+1;
	}
	costs << "\n";

	double sum = 0, sumBase = 0;
	for(int iter = 0; iter < 1; iter++){
		stats << iter+1;
		statsNoFeed << iter+1;
		int numAnomFound = 0;
		// std::cout << "iter " << iter << ", # Anomaly: ";
		IsolationForest iff(ntree, dt, nsample, maxheight, rsample);
		iff.indexInstancesIntoNodes(dt);
		std::vector<double> scores(dt->nrow, 0.0), scoresNorm(dt->nrow, 0.0);
		iff.weightIndexedScore(scores);
		normalizeScore(scores, scoresNorm);
		iff.computeMass(scoresNorm);
		std::vector<bool> gotFeedback(dt->nrow, false);
		std::vector<int> feedbackIdx;
		if(scores.size()<numFeedback) numFeedback = scores.size();
		int breakout = 0;
		for(int feed = 0; feed < numFeedback; feed++){
			if(variableLearningRate)	LRATE = 1/sqrt(feed + 1);
			if(feed == 0){
				int baseAnom = printNoFeedbackAnomCntToFile(scores, metadata, statsNoFeed, numFeedback);
				// std::cout << "Baseline -> " << baseAnom << std::endl;
				sumBase += baseAnom;
			}
//			if(feed == 0 || feed == (numFeedback-1)){
//				iff.printStat(tree);
//			}
//			if(feed == 0 || feed == (numFeedback-1)){
//				sprintf(fname, "%s_iter_%d_feed_%d_losstype_%s_updatetype_%s_ngrad_%d_reg_%g_lrate_%g_pwgt_%d_inwgt_%d_rtype_L%d.csv",
//						output_name, iter + 1, numFeedback, typeLoss, typeUpdate, numGradUpd, REG, LRATE, Tree::POS_WEIGHT_ONLY, reInitWeights, regularizerType);
//				printScoreToFile(scores, csv, metadata, dt, fname);
//			}

			double min = DBL_MAX;
			int minInd = -1;
			for(int i = 0; i < (int)scores.size(); i++){
				if(gotFeedback[i] == false && min > scores[i]){
					min = scores[i];
					minInd = i;
				}
			}
			gotFeedback[minInd] = true;
			feedbackIdx.push_back(minInd);
			if(strcmp(metadata->data[minInd][0], "anomaly") == 0)
				numAnomFound++;
			stats << "," << numAnomFound;

			std::vector<double> w0, w1;
			bool first = false;
			if(numGradUpd == 0)
				iff.getCurrentWeights(w0);
			int updCnt = numGradUpd;
			if(updCnt == 0)
				updCnt = MAXGRADITER;

			if(lossType == 0){//linear loss
				costBefore[feed] = getLinearLoss(minInd, metadata, scores);
				avgcostBefore[feed] = getLinearLoss(feedbackIdx, metadata, scores);

				if(updateType == 0){// online update
					int y = (strcmp(metadata->data[minInd][0], "anomaly") == 0) ? 1 : -1;
					if(y == -1) breakout++;
					else {
						breakout = 0;
					}
					for(int i = 0; i < updCnt; i++){
						iff.updateWeights(scores, dt->data[minInd], -y, 0, LRATE, LRATE*REG, regularizerType);
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
					}
				}
				else if(updateType == 1){// stochastic update
					for(int i = 0; i < updCnt; i++){
						for(int j = 0; j < (int)feedbackIdx.size(); j++){
							int y = (strcmp(metadata->data[feedbackIdx[j]][0], "anomaly") == 0) ? 1 : -1;
							iff.updateWeights(scores, dt->data[feedbackIdx[j]], -y, 0, LRATE, LRATE*REG, regularizerType);
						}
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
						shuffle(feedbackIdx);
					}
				}
				else if(updateType == 2){// batch update
					for(int i = 0; i < updCnt; i++){
						int y = (strcmp(metadata->data[feedbackIdx[0]][0], "anomaly") == 0) ? 1 : -1;
						if(y == -1) breakout++;
						else {
							breakout = 0;
						}
						iff.updateWeights(scores, dt->data[feedbackIdx[0]], -y, 0, LRATE, LRATE*REG, regularizerType);
						for(int j = 1; j < (int)feedbackIdx.size(); j++){
							y = (strcmp(metadata->data[feedbackIdx[j]][0], "anomaly") == 0) ? 1 : -1;
							if(y == -1) breakout++;
							else {
								breakout = 0;
							}
							iff.updateWeights(scores, dt->data[feedbackIdx[j]], -y, 0, LRATE, 0, regularizerType);
						}
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
					}
				}

				costAfter[feed] = getLinearLoss(minInd, metadata, scores);
				avgcostAfter[feed] = getLinearLoss(feedbackIdx, metadata, scores);
			}
			else if(lossType == 1){//log likelihood loss
				costBefore[feed] = getLoglikelihoodLoss(minInd, metadata, scoresNorm);
				avgcostBefore[feed] = getLoglikelihoodLoss(feedbackIdx, metadata, scoresNorm);

				if(updateType == 0){// online update
					int y = (strcmp(metadata->data[minInd][0], "anomaly") == 0) ? 1 : -1;
					if(y == -1) breakout++;
					else {
						breakout = 0;
					}
					for(int i = 0; i < updCnt; i++){
						iff.updateWeightsLLH(scores, dt->data[minInd], -y, LRATE, REG, regularizerType);
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
						normalizeScore(scores, scoresNorm);
						iff.computeMass(scoresNorm);
					}
				}
				else if(updateType == 1){// stochastic update
					for(int i = 0; i < updCnt; i++){
						for(int j = 0; j < (int)feedbackIdx.size(); j++){
							int y = (strcmp(metadata->data[feedbackIdx[j]][0], "anomaly") == 0) ? 1 : -1;
							iff.updateWeightsLLH(scores, dt->data[feedbackIdx[j]], -y, LRATE, REG, regularizerType);
						}
						normalizeScore(scores, scoresNorm);
						iff.computeMass(scoresNorm);
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
						shuffle(feedbackIdx);
					}
				}
				else if(updateType == 2){// batch update
					for(int i = 0; i < updCnt; i++){
						int y = (strcmp(metadata->data[feedbackIdx[0]][0], "anomaly") == 0) ? 1 : -1;
						if(y == -1) breakout++;
						else {
							breakout = 0;
						}
						iff.updateWeightsLLH(scores, dt->data[feedbackIdx[0]], -y, LRATE, REG, regularizerType);
						for(int j = 1; j < (int)feedbackIdx.size(); j++){
							y = (strcmp(metadata->data[feedbackIdx[j]][0], "anomaly") == 0) ? 1 : -1;
							if(y == -1) breakout++;
							else {
								breakout = 0;
							}
							iff.updateWeightsLLH(scores, dt->data[feedbackIdx[j]], -y, LRATE, 0, regularizerType);
						}
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
						normalizeScore(scores, scoresNorm);
						iff.computeMass(scoresNorm);
					}
				}

				costAfter[feed] = getLoglikelihoodLoss(minInd, metadata, scoresNorm);
				avgcostAfter[feed] = getLoglikelihoodLoss(feedbackIdx, metadata, scoresNorm);
			}
			else if(lossType == 2){//logistic loss
				costBefore[feed] = getLogisticLoss(minInd, metadata, scores);
				avgcostBefore[feed] = getLogisticLoss(feedbackIdx, metadata, scores);

				if(updateType == 0){// online update
					int y = (strcmp(metadata->data[minInd][0], "anomaly") == 0) ? 1 : -1;
					if(y == -1) breakout++;
					else {
						breakout = 0;
					}
					for(int i = 0; i < updCnt; i++){
						double change = 1 / (1 + exp(-y * scores[minInd]));
						if(change > 1e-5){
							iff.updateWeights(scores, dt->data[minInd], -y, 0, LRATE*change, LRATE*REG, regularizerType);
						}
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
					}
				}
				else if(updateType == 1){// stochastic update
					for(int i = 0; i < updCnt; i++){
						for(int j = 0; j < (int)feedbackIdx.size(); j++){
							int y = (strcmp(metadata->data[feedbackIdx[j]][0], "anomaly") == 0) ? 1 : -1;
							double change = 1 / (1 + exp(-y * scores[feedbackIdx[j]]));
							if(change > 1e-6){
								iff.updateWeights(scores, dt->data[feedbackIdx[j]], -y, 0, LRATE*change, LRATE*REG, regularizerType);
							}
						}
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
						shuffle(feedbackIdx);
					}
				}
				else if(updateType == 2){// batch update
					std::vector<double> tscores;
					for(int ii = 0; ii < (int)scores.size(); ii++)
						tscores.push_back(scores[ii]);
					for(int i = 0; i < updCnt; i++){
						int y = (strcmp(metadata->data[feedbackIdx[0]][0], "anomaly") == 0) ? 1 : -1;
						if(y == -1) breakout++;
						else {
							breakout = 0;
						}
						double change = 1 / (1 + exp(-y * tscores[feedbackIdx[0]]));
						iff.updateWeights(scores, dt->data[feedbackIdx[0]], -y, 0, LRATE*change, LRATE*REG, regularizerType);
						for(int j = 1; j < (int)feedbackIdx.size(); j++){
							y = (strcmp(metadata->data[feedbackIdx[j]][0], "anomaly") == 0) ? 1 : -1;
							if(y == -1) breakout++;
							else {
								breakout = 0;
							}
							double change = 1 / (1 + exp(-y * tscores[feedbackIdx[j]]));
							if(change > 1e-6){
								iff.updateWeights(scores, dt->data[feedbackIdx[j]], -y, 0, LRATE*change, 0, regularizerType);
							}
						}
						if(numGradUpd == 0 && (i == (MAXGRADITER-1) || getDifference(w0, w1, first, iff) < 1e-2)){
							if(PRINT) std::cout << "," << i+1 << std::flush;
							break;
						}
						tscores.clear();
						for(int ii = 0; ii < (int)scores.size(); ii++)
							tscores.push_back(scores[ii]);
					}
				}

				costAfter[feed] = getLogisticLoss(minInd, metadata, scores);
				avgcostAfter[feed] = getLogisticLoss(feedbackIdx, metadata, scores);
				// for stochastic and batch update reinitialize weights
			}
			if(breakout >= 10){
				re[0] += numAnomFound;
				re[1] += feed;
				return 0;
			}
		}
		re[0] += numAnomFound;
		re[1] += numFeedback;
		return 1;

		stats << "\n";
		statsNoFeed << "\n";
		std::cout << " Feedback -> " << numAnomFound << std::endl;
		sum += numAnomFound;

		costs << iter+1;
		for(int feed = 0; feed < numFeedback; feed++)
			costs << "," << costBefore[feed];
		costs << "\n";
		costs << iter+1;
		for(int feed = 0; feed < numFeedback; feed++)
			costs << "," << costAfter[feed];
		costs << "\n";
		costs << iter+1;
		for(int feed = 0; feed < numFeedback; feed++)
			costs << "," << avgcostBefore[feed];
		costs << "\n";
		costs << iter+1;
		for(int feed = 0; feed < numFeedback; feed++)
			costs << "," << avgcostAfter[feed];
		costs << "\n";
	}
	std::cout << "Avg: Baseline -> " << sumBase/numIter << " Feedback -> " << sum/numIter<< std::endl;
//	tree.close();
	stats.close();
	statsNoFeed.close();
	costs.close();
	std::cout << "Time elapsed: " << std::time(nullptr) - st << " seconds\n";

	return 0;
}
template<class T>
int Length(T& arr)
{
    //cout << sizeof(arr[0]) << endl;
    //cout << sizeof(arr) << endl;
    return sizeof(arr) / sizeof(arr[0]);
}

int main(int argc, char* argv[]){
	clock_t startTime,endTime;
	startTime = clock();
	int re[] = {0,0};
	int feedback = std::atoi(argv[14]);
	int feed;
	int j=0;
	int iter=0;
	char ch;
	ifstream OpenFile("datasets/anomaly/abalone/fullsamples/queue.txt");
	OpenFile.get(ch);
	j = atoi(&ch);
	std::sprintf(argv[2], "datasets/anomaly/abalone/fullsamples/abalone_m%d.csv", j);
	std::sprintf(argv[4], "outtest/abalone_m%d", j);
	// for(int m=0;m<15;m++){ 
	// 	cout<<argv[m]<<endl;
	// }
	while(re[1] != 150){
		mains(argc, argv, re);
        std::cout<<"subgrahps"<<iter<<"# Found anomalies: "<<re[0]<<" Total feedback: "<<re[1]<<std::endl;
		feed = feedback - re[1];
		std::sprintf(argv[14], "%d", feed);
		if(! OpenFile.get(ch)) {
			cout<<"No enough subgrahps, exit\n";
			break;
		}
		j = atoi(&ch);
		std::sprintf(argv[2], "datasets/anomaly/abalone/fullsamples/abalone_m%d.csv", j);
		std::sprintf(argv[4], "outtest/abalone_m%d", j);
		iter++;
	}
    std::cout<<"final# Found anomalies: "<<re[0]<<" Total feedback: "<<re[1]<<std::endl;
	endTime = clock();
    std::cout << "Running time: "<<float(endTime-startTime)/CLOCKS_PER_SEC << std::endl;
	return 0;
}
