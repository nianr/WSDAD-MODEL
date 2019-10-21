/*
 * Tree.h
 *
 *  Created on: Mar 24, 2015
 *      Author: Tadeze
 */

#ifndef TREE_H_
#define TREE_H_
#include "utility.hpp"
#include "cincl.hpp"

class Tree {
	Tree *leftChild;
	Tree *rightChild;
	Tree *parent;
	int depth;
	int nodeSize;

	int splittingAtt;
	double splittingPoint;

	int newNodeSize; // new attribute for online IF
	double minAttVal, maxAttVal;
	double volume;// store volume of this node in log scale
	double weight;// weight of this node in the weighted representation of the tree
	double weightUpd;// weight to be updated in case of moving average option is selected
	double mass;// mass of this node i.e. sum of the probabilities of all instances under this node
	std::vector<int> instIdx;// hold the index of data fall into this tree node
	static std::vector<double> LB, UB;

	static double **Qnt;

public:
	static bool checkRange;
	static bool useVolumeForScore;
	static bool POS_WEIGHT_ONLY;
	static bool KEEP_NEG_BUT_USE_0;

	Tree() {
		leftChild = NULL;
		rightChild = NULL;
		parent = NULL;
		splittingAtt = -1;
		splittingPoint = 0;
		depth = 0;
		nodeSize = 1;

		newNodeSize = 1;// prior 1 in case of no data
		minAttVal = 0;
		maxAttVal = 0;
		volume = 0;
		// initial weight is set to negative to interpret higher score as anomalous
		weight = 1;// initial weight changed to 1 to make it a cost (previously -1)
		weightUpd = -1;
		mass = 0;
	}

	virtual ~Tree() {
		delete leftChild;
		delete rightChild;
	}

	void iTree(std::vector<int> const &dIndex, const doubleframe* dt,
			int &maxHeight);
	double pathLength(double *inst);
	double pathLength(const double *inst, bool **marginalize);
	// for online IF
	void renewNodeSize();
	void update(const double inst[]);
	void printDepthAndNodeSize(std::ofstream &out);
	void printPatternFreq(double inst[], int &tid, int &iid, std::ofstream &out);

	std::vector<double> getPatternScores(double *inst, int depLim = 10);
	int getHighestDepth(double *inst);
	double getL1NormofWeights();
	void getCurrentWeights(std::vector<double> &w);
	void reinitializeWeights();
	double getScoreAtDepth(double *inst, int depLim);
	double getPatternScoreAtDepth(double *inst, int depLim);
	double getScoreFromWeights(double *inst);
	void weightIndexedScore(std::vector<double> &scores);
	void updateWeightsLLH(std::vector<double> &scores, double *inst, int direction, double lrate,
			double reg = 0, int regType = 1);
	void updateWeights(std::vector<double> &scores, double *inst, int direction, int type, double change = 1,
			double reg = 0, int regType = 1);
	void updateWeightsRunAvg(std::vector<double> &scores, double *inst, int direction, double change = 1);
	void indexInstancesIntoNodes(std::vector<int> &idx, const doubleframe* df);
	double computeMass(std::vector<double> &probScores);
	static void initializeLBandUB(const doubleframe* _df, std::vector<int> &sampleIndex);
	static void initializeQuantiles(const doubleframe* dt);
	static int getQuantile(int f, double p);
};

#endif /* TREE_H_ */
