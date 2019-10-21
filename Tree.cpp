/*
 * Tree.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: Tadeze
 */

#include "Tree.hpp"

bool Tree::checkRange = false;
bool Tree::useVolumeForScore = false;
bool Tree::POS_WEIGHT_ONLY = false;
bool Tree::KEEP_NEG_BUT_USE_0 = false;
std::vector<double> Tree::LB;
std::vector<double> Tree::UB;
double **Tree::Qnt;

void Tree::iTree(std::vector<int> const &dIndex, const doubleframe *dt, int &maxheight){
	this->nodeSize = dIndex.size(); // Set size of the node
	if (dIndex.size() <= 1 || (maxheight > 0 && this->depth > maxheight))
		return;

	double min, max, temp;
	int attCnt = dt->ncol, attribute, rIdx;
	int *attPool = new int[attCnt];
	for(int i = 0; i < attCnt; ++i)
		attPool[i] = i;
	while(attCnt > 0){
		rIdx = randomI(0, attCnt - 1);
		attribute = attPool[rIdx];
		min = max = dt->data[dIndex[0]][attribute];
		for(unsigned int i = 1; i < dIndex.size(); ++i){
			temp = dt->data[dIndex[i]][attribute];
			if (min > temp) min = temp;
			if (max < temp) max = temp;
		}
		if(min < max) break; // found attribute
		--attCnt;
		attPool[rIdx] = attPool[attCnt];
	}
	delete []attPool;
	if(attCnt <= 0){ // no valid attribute found
		this->weight = this->nodeSize;
		return;
	}

	this->minAttVal = min;
	this->maxAttVal = max;
	this->splittingAtt = attribute;
	this->splittingPoint = randomD(min, max);

	//Split the node into two
	std::vector<int> lnodeData, rnodeData;
	for (unsigned int i = 0; i < dIndex.size(); i++) {
		temp = dt->data[dIndex[i]][this->splittingAtt];
		if (temp <= this->splittingPoint && temp != max)
			lnodeData.push_back(dIndex[i]);
		else
			rnodeData.push_back(dIndex[i]);
	}

	leftChild = new Tree();
	leftChild->parent = this;
	leftChild->depth = this->depth + 1;
	leftChild->volume = this->volume + log(this->splittingPoint - Tree::LB[attribute])
									 - log(Tree::UB[attribute] - Tree::LB[attribute]);
//	leftChild->volume = this->volume + log(this->splittingPoint - min) - log(max - min);
	temp = Tree::UB[attribute];
	Tree::UB[attribute] = this->splittingPoint;
	leftChild->iTree(lnodeData, dt, maxheight);
	Tree::UB[attribute] = temp;

	rightChild = new Tree();
	rightChild->parent = this;
	rightChild->depth = this->depth + 1;
	rightChild->volume = this->volume + log(Tree::UB[attribute] - this->splittingPoint)
									  - log(Tree::UB[attribute] - Tree::LB[attribute]);
//	rightChild->volume = this->volume + log(max - this->splittingPoint) - log(max - min);
	temp = Tree::LB[attribute];
	Tree::LB[attribute] = this->splittingPoint;
	rightChild->iTree(rnodeData, dt, maxheight);
	Tree::LB[attribute] = temp;
}

double Tree::getScoreAtDepth(double *inst, int depLim){
	double ret = 0;
	Tree *cur = this;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
		ret = cur->depth + avgPL(cur->nodeSize);
		if(cur->depth == depLim)
			return ret;
	}
	return ret;
}

double Tree::getPatternScoreAtDepth(double *inst, int depLim){
	double ret = 0;
	Tree *cur = this;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
//		if(cur->nodeSize == 0)
//			std::cout << "error0" << std::endl;
		ret = log(cur->nodeSize) - cur->volume;
		if(cur->depth == depLim)
			return ret;
	}
	return ret;
}


std::vector<double> Tree::getPatternScores(double *inst, int depLim){
	std::vector<double> ret;
	Tree *cur = this;
	int cnt = 0;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
		ret.push_back(log(cur->nodeSize) - cur->volume);
		++cnt;
		if(cnt == depLim)
			break;
	}
	return ret;
}

double Tree::getScoreFromWeights(double *inst){
	Tree *cur = this;
	double score = 0;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
		score += cur->weight;
	}
	return score;
}

void Tree::weightIndexedScore(std::vector<double> &scores){
	if(this->depth > 0){
		for(int i = 0; i < (int)this->instIdx.size(); i++)
			scores[instIdx[i]] += this->weight;
	}
	if(this->leftChild == NULL || this->rightChild == NULL)
		return;
	this->leftChild->weightIndexedScore(scores);
	this->rightChild->weightIndexedScore(scores);
}

void Tree::updateWeightsLLH(std::vector<double> &scores, double *inst, int direction, double lrate, double reg, int regType){
	double prevWeight, delta;
	Tree *cur = this;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;

		double regPen = 0;
		if(reg != 0){
			regPen = 1;
			if(cur->weight - 1 < 0)
				regPen = -1;
			if(regType == 2)
				regPen = cur->weight - 1;
		}
		prevWeight = cur->weight;
		// for leaf with multiple nodes change weights accordingly
		if(cur->leftChild == NULL || cur->rightChild == NULL)
			cur->weight += direction * cur->nodeSize * lrate * ((1 - cur->mass) + reg*regPen);
		else
			cur->weight += direction * lrate * ((1 - cur->mass) + reg * regPen);

		if(Tree::POS_WEIGHT_ONLY == true && cur->weight < 0) cur->weight = 0;// prevent weight to be negative

		delta = cur->weight - prevWeight;

		if(Tree::KEEP_NEG_BUT_USE_0){
			if(cur->weight < 0){
				delta = 0 - prevWeight;
				if(delta > 0)
					delta = 0;
			}else if(prevWeight < 0){
				delta = cur->weight - 0;
			}
		}
		if(fabs(delta) > 0){
			for(int i = 0; i < (int)cur->instIdx.size(); i++)
				scores[cur->instIdx[i]] += delta;
		}
	}
}


void Tree::updateWeights(std::vector<double> &scores, double *inst, int direction, int type, double change, double reg, int regType){
	double prevWeight, delta;
	Tree *cur = this;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;

		double regPen = 0;
		if(reg != 0){
			regPen = 1;
			if(cur->weight - 1 < 0)
				regPen = -1;
			if(regType == 2)
				regPen = cur->weight - 1;
		}
		prevWeight = cur->weight;
		// for leaf with multiple nodes change weights accordingly
		if(cur->leftChild == NULL || cur->rightChild == NULL)
			cur->weight += direction * cur->nodeSize * (change + reg*regPen);
		else
			cur->weight += direction * (change + reg * regPen);

		if(Tree::POS_WEIGHT_ONLY == true && cur->weight < 0) cur->weight = 0;// prevent weight to be negative

		delta = cur->weight - prevWeight;

		if(Tree::KEEP_NEG_BUT_USE_0){
			if(cur->weight < 0){
				delta = 0 - prevWeight;
				if(delta > 0)
					delta = 0;
			}else if(prevWeight < 0){
				delta = cur->weight - 0;
			}
		}
		if(fabs(delta) > 0){
			for(int i = 0; i < (int)cur->instIdx.size(); i++)
				scores[cur->instIdx[i]] += delta;
		}
		// if exponential change
		if(type == 1)
			change *= 2;
	}
}

void Tree::updateWeightsRunAvg(std::vector<double> &scores, double *inst, int direction, double change){
	double prevWeight, delta;
	Tree *cur = this;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;

		// for leaf with multiple nodes change weights accordingly
		if(cur->leftChild == NULL || cur->rightChild == NULL)
			cur->weightUpd += direction * cur->nodeSize * change;
		else
			cur->weightUpd += direction * change;

		prevWeight = cur->weight;
		cur->weight = (cur->weight + cur->weightUpd) / 2;

		delta = cur->weight - prevWeight;
		for(int i = 0; i < (int)cur->instIdx.size(); i++)
			scores[cur->instIdx[i]] += delta;

	}
}

void Tree::indexInstancesIntoNodes(std::vector<int> &idx, const doubleframe* df){
	if(this->leftChild == NULL || this->rightChild == NULL)
		return;
	for(int i = 0; i < (int)idx.size(); i++){
		if(df->data[idx[i]][this->splittingAtt] <= this->splittingPoint)
			this->leftChild->instIdx.push_back(idx[i]);
		else
			this->rightChild->instIdx.push_back(idx[i]);
	}
	this->leftChild->indexInstancesIntoNodes(this->leftChild->instIdx, df);
	this->rightChild->indexInstancesIntoNodes(this->rightChild->instIdx, df);
}

double Tree::computeMass(std::vector<double> &probScores){
	if(this->leftChild == NULL || this->rightChild == NULL){
		this->mass = 0;
		for(int i = 0; i < (int)this->instIdx.size(); i++){
			this->mass += probScores[this->instIdx[i]];
		}
	}else{
		this->mass = this->leftChild->computeMass(probScores) + this->rightChild->computeMass(probScores);
	}
	return this->mass;
}

int Tree::getHighestDepth(double *inst){
	Tree *cur = this;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
	}
	return cur->depth;
}

double Tree::getL1NormofWeights(){
	if(this->leftChild == NULL || this->rightChild == NULL){
		return fabs(this->weight);
	}
	return  this->leftChild->getL1NormofWeights() +
			this->rightChild->getL1NormofWeights();
}

void Tree::getCurrentWeights(std::vector<double> &w){
	w.push_back(this->weight);
	if(this->leftChild != NULL && this->rightChild != NULL){
		this->leftChild->getCurrentWeights(w);
		this->rightChild->getCurrentWeights(w);
	}
}

void Tree::reinitializeWeights(){
	if(this->leftChild == NULL || this->rightChild == NULL){
		this->weight = this->nodeSize;
	}
	else{
		this->weight = 1;
		this->leftChild->reinitializeWeights();
		this->rightChild->reinitializeWeights();
	}
}

/*
 * takes an instance as vector of double
 */
double Tree::pathLength(double *inst) {
	Tree *cur = this;
	double min, max, instAttVal, temp;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if(cur->nodeSize <= 1) break;
		instAttVal = inst[cur->splittingAtt];
		if(Tree::checkRange == true){
			min = cur->minAttVal;
			max = cur->maxAttVal;
			if(instAttVal < min){
				temp = randomD(instAttVal, max);
				if(temp < min){
					if(Tree::useVolumeForScore == true)
						return ((cur->depth + 1) *
							   -(cur->volume + log(temp - instAttVal) - log(max-instAttVal)));
					else
						return (cur->depth + 1);
				}
			}
			if(instAttVal > max){
				temp = randomD(min, instAttVal);
				if(temp > max){
					if(Tree::useVolumeForScore == true)
						return ((cur->depth + 1) *
							   -(cur->volume + log(instAttVal-temp) - log(instAttVal-min)));
					else
						return (cur->depth + 1);
				}
			}
		}

		if (instAttVal <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
	}
	if(Tree::useVolumeForScore == true){
//		if(cur->nodeSize <= 1)
//			return (-cur->nodeSize/cur->volume);
//		return (-1/(cur->volume - log(cur->nodeSize)));
		double d = (cur->depth + avgPL(cur->nodeSize));
		double vol = cur->volume;
		if(cur->nodeSize > 1)
			vol -= log(cur->nodeSize);
		return (d * -vol);
//		return (-cur->nodeSize/cur->volume);
	}
	return (cur->depth + avgPL(cur->nodeSize));
}


double Tree::pathLength(const double *inst, bool **marginalize) {
	Tree *cur = this;
	if(cur->leftChild == NULL && cur->rightChild == NULL)
		return (cur->depth + avgPL(cur->nodeSize));

	int q = Tree::getQuantile(cur->splittingAtt, cur->splittingPoint);

	if(marginalize[cur->splittingAtt][q] == true){
		double ldepth = cur->leftChild->pathLength(inst, marginalize);
		double rdepth = cur->rightChild->pathLength(inst, marginalize);
		return 	(cur->leftChild->nodeSize / (double)cur->nodeSize) * ldepth +
				(cur->rightChild->nodeSize / (double)cur->nodeSize) * rdepth;
	}else{
		double instAttVal = inst[cur->splittingAtt];
		if (instAttVal <= cur->splittingPoint)
			return cur->leftChild->pathLength(inst, marginalize);
		else
			return cur->rightChild->pathLength(inst, marginalize);
	}
}

// for online IF
void Tree::renewNodeSize(){
	nodeSize = newNodeSize;
	newNodeSize = 1;
	if(leftChild == NULL && rightChild == NULL)
		return;
	leftChild->renewNodeSize();
	rightChild->renewNodeSize();
}

void Tree::update(const double inst[]){
	++newNodeSize;
	if(leftChild == NULL && rightChild == NULL)
		return;
	if(inst[splittingAtt] <= splittingPoint)
		leftChild->update(inst);
	else
		rightChild->update(inst);
}

void Tree::printDepthAndNodeSize(std::ofstream &out){
	for(int i = 0; i < depth; ++i)
		out << "-";
	out << "(" << depth
		<< ", " << nodeSize << ", " << weight
//		<< ", " << exp(volume)
//		<< ", " << splittingAtt
//		<< ", " << minAttVal
		<< ", " << splittingPoint
//		<< ", " << maxAttVal
		<< ", " << mass
		<< ")" << std::endl;
	if(leftChild != NULL)
		leftChild->printDepthAndNodeSize(out);
	if(rightChild != NULL)
		rightChild->printDepthAndNodeSize(out);

	//	if(this->leftChild == NULL && this->rightChild == NULL){
	//		out << this->depth << "," << this->nodeSize << std::endl;
	//	}else{
	//		this->leftChild->printDepthAndNodeSize(out);
	//		this->rightChild->printDepthAndNodeSize(out);
	//	}

//	if(this->leftChild == NULL && this->rightChild == NULL){
//		out << this->depth << "," << this->nodeSize << std::endl;
//	}else{
//		this->leftChild->printDepthAndNodeSize(out);
//		this->rightChild->printDepthAndNodeSize(out);
//	}
}

void Tree::printPatternFreq(double inst[], int &tid, int &iid, std::ofstream &out){
	Tree *cur = this;
//	out << (iid+1) << ", "
//		<< (tid+1) << ", "
//		<< cur->depth << ", "
//		<< cur->nodeSize << ", "
//		<< exp(cur->volume) << ", "
//		<< cur->nodeSize/exp(cur->volume) << std::endl;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
		out << (iid+1) << ", "
			<< (tid+1) << ", "
			<< cur->depth << ", "
			<< cur->nodeSize << ", "
			<< exp(cur->volume) << ", "
			<< log(cur->nodeSize) - cur->volume << std::endl;
	}
}


void Tree::initializeLBandUB(const doubleframe* _df, std::vector<int> &sampleIndex){
	// initialize LBs and UBs
	Tree::LB.clear();
	Tree::UB.clear();
	double min, max, temp;
	for(int j = 0; j < _df->ncol; ++j){
		temp = _df->data[sampleIndex[0]][j];
		min = max = temp;
		for(unsigned int i = 1; i < sampleIndex.size(); ++i){
			temp = _df->data[sampleIndex[i]][j];
			if(min > temp) min = temp;
			if(max < temp) max = temp;
		}
		Tree::LB.push_back(min);
		Tree::UB.push_back(max);
	}
}

// Todo: delete memory allocated for Qnt
void Tree::initializeQuantiles(const doubleframe* dt){
	// initialize Qnt
	Qnt = new double *[dt->ncol];
	for(int i = 0; i < dt->ncol; i++)
		Qnt[i] = new double[3];

	int q25 = (int)ceil(dt->nrow / 4.0),
		q50 = (int)ceil(dt->nrow / 2.0),
		q75 = (int)ceil(dt->nrow * 3 / 4.0);
	double *temp = new double[dt->nrow];
	for(int i = 0; i < dt->ncol; i++){
		for(int j = 0; j < dt->nrow; j++)
			temp[j] = dt->data[j][i];
		qsort(temp, dt->nrow, sizeof(temp[0]), (int (*)(const void *, const void *))dblcmp);
		Qnt[i][0] = temp[q25];
		Qnt[i][1] = temp[q50];
		Qnt[i][2] = temp[q75];
	}
	delete []temp;
}

int Tree::getQuantile(int f, double p){
	int q = 3;
	if(p < Qnt[f][0])
		q = 0;
	else if(Qnt[f][0] <= p && p < Qnt[f][1])
		q = 1;
	else if(Qnt[f][1] <= p && p < Qnt[f][2])
		q = 2;
	return q;
}
