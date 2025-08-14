/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* March 2016                                           */
/********************************************************/
/* Plan selector   										*/
/********************************************************/

#include <iostream>
#include "selector.hpp"
using namespace std;

/*******************************************/
/* SearchQueue                             */
/*******************************************/

SearchQueue::SearchQueue(int index) {
	this->index = index;
	pq.reserve(INITIAL_PQ_CAPACITY);
	pq.push_back(nullptr);	// Position 0 empty
	planPosition.reserve(INITIAL_PQ_CAPACITY);
	bestH = FLOAT_INFINITY;
	improvedH = true;
}

// Adds a new plan to the list of open nodes
void SearchQueue::add(Plan* p) {
	unsigned int gap = pq.size();
	uint32_t parent;
	pq.push_back(nullptr);
	while (gap > 1 && p->compare(pq[gap >> 1], index) < 0) {
		parent = gap >> 1;
		planPosition[pq[parent]->id] = gap;
		pq[gap] = pq[parent];
		gap = parent;
	}
	pq[gap] = p;
	planPosition[p->id] = gap;
}

// Removes and returns the best plan in the queue of open nodes
Plan* SearchQueue::poll() {
	Plan* best = pq[1];
	if (pq.size() > 2) {
		pq[1] = pq.back();
		planPosition[pq[1]->id] = 1;
		pq.pop_back();
		heapify(1);
	}
	else if (pq.size() > 1) pq.pop_back();
	return best;
}

// Repairs the order in the priority queue
void SearchQueue::heapify(unsigned int gap) {
	Plan* aux = pq[gap];
	unsigned int child = gap << 1;
	while (child < pq.size()) {
		if (child != pq.size() - 1 && pq[child + 1]->compare(pq[child], index) < 0)
			child++;
		if (pq[child]->compare(aux, index) < 0) {
			planPosition[pq[child]->id] = gap;
			pq[gap] = pq[child];
			gap = child;
			child = gap << 1;
		}
		else break;
	}
	pq[gap] = aux;
	planPosition[aux->id] = gap;
}

void SearchQueue::remove(Plan* p) {
	uint32_t k = planPosition[p->id], parent;
	Plan* ult = pq.back();
	pq.pop_back();
	if (ult->compare(p, index) < 0) {
		while (k > 1 && ult->compare(pq[k >> 1], index) < 0) {
			parent = k >> 1;
			planPosition[pq[parent]->id] = k;
			pq[k] = pq[parent];
			k = parent;
		}
		pq[k] = ult;
		planPosition[ult->id] = k;
	}
	else {
		pq[k] = ult;
		planPosition[ult->id] = k;
		heapify(k);
	}
}

void SearchQueue::clear() {
	planPosition.clear();
	pq.clear();
	pq.push_back(nullptr);	// Position 0 empty
}


/*******************************************/
/* Selector                               */
/*******************************************/

Selector::Selector() {
	currentQueue = 0;
	overallBestPlan = nullptr;
	overallBest = FLOAT_INFINITY;
	iterationsWithoutImproving = 0;
}

void Selector::addQueue(int qtype) {
	queues.push_back(new SearchQueue(qtype));
}

bool Selector::add(Plan* p) {
	SearchQueue* q = queues[currentQueue];
	float ph = p->getH(q->getIndex());
	if (ph < q->bestH) {
		q->improvedH = true;
		q->bestH = ph;
	}
	for (unsigned int i = 0; i < queues.size(); i++) {
		queues[i]->add(p);
	}
	if (p->h < overallBest) {
		iterationsWithoutImproving = 0;
		overallBest = p->h;
		overallBestPlan = p;
		//cout << "[" << q->getIndex() << "]" << overallBest << endl;
		return true;
	}
	return false;
}

void Selector::exportTo(Selector* s) {
	SearchQueue* q = queues[0];
	unsigned int n = q->size();
	for (unsigned int i = 1; i <= n; i++) {
		s->add(q->getPlanAt(i));
	}
}

// Removes and returns the best plan in the queue of open nodes
Plan* Selector::poll() {
	SearchQueue* q = queues[currentQueue];
	if (!q->improvedH) {
		if (++currentQueue >= (int)queues.size()) currentQueue = 0;
		q = queues[currentQueue];
	}
	Plan* next = q->poll();
	for (unsigned int i = 0; i < queues.size(); i++) {
		if ((int)i != currentQueue) {
			queues[i]->remove(next);
		}
	}
	q->improvedH = false;
	iterationsWithoutImproving++;
	return next;
}

void Selector::clear() {
	for (unsigned int i = 0; i < queues.size(); i++) {
		queues[i]->clear();
	}
}

/*
void Selector::initialize(bool hLandQueue, bool greedy) {
	currentQueue = true;
	qFF = new SearchQueue(greedy ? PQ_FF : PQ_A_FF);
	if (hLandQueue) {
		qLand = new SearchQueue(greedy ? PQ_LAND : PQ_A_LAND);
	} else {
		qLand = nullptr;
	}
	bestH = FLOAT_INFINITY;
	improvedH = true;
	bestPlan = overallBestPlan = nullptr;
	overallBest = FLOAT_INFINITY;
	type = greedy ? 'G' : 'A';
	iterationsWithoutImproving = 0;
	failedPlateauSearchSteps = 0;
}

void Selector::restart(Plan* rootPlan) {
	qFF->clear();
	qFF->add(rootPlan);
	if (qLand != nullptr) {
		qLand->clear();
		qLand->add(rootPlan);
	}
	bestH = FLOAT_INFINITY;
	improvedH = true;
	bestPlan = overallBestPlan = nullptr;
	overallBest = FLOAT_INFINITY;
	iterationsWithoutImproving = 0;
	failedPlateauSearchSteps = 0;
}



Plan* Selector::randomPoll() {
	int n = 1 + (rand() % (qFF->size()));
	Plan* next = qFF->getPlanAt(n);
	qFF->remove(next);
	if (qLand != nullptr) qLand->remove(next);
	return next;
}

void Selector::changeQueue() {
	if (currentQueue) {
		if (qLand != nullptr) currentQueue = false;
	} else {
		currentQueue = true;
	}
}

bool Selector::add(Plan* p) {
	if (currentQueue) {
		if (p->h < bestH) {
			improvedH = true;
			bestH = p->h;
		}
	} else {
		if (p->hLand < bestH) {
			improvedH = true;
			bestH = p->hLand;
		}
	}
	qFF->add(p);
	if (qLand != nullptr) qLand->add(p);
	if (p->h < overallBest) {
		iterationsWithoutImproving = 0;
		failedPlateauSearchSteps = 0;
		overallBest = p->h;
		overallBestPlan = p;
		cout << type << overallBest << endl;
		return true;
	}
	return false;
}
*/

/*******************************************/
/* Plateau Selector                        */
/*******************************************/

PlateauSelector::PlateauSelector(int qtype) {
	q = new SearchQueue(qtype);
}

// Removes and returns the best plan in the queue of open nodes
Plan* PlateauSelector::poll() {
	return q->poll();
}

Plan* PlateauSelector::randomPoll() {
	int n = 1 + (rand() % (q->size()));
	Plan* next = q->getPlanAt(n);
	q->remove(next);
	return next;
}

void PlateauSelector::add(Plan* p) {
	q->add(p);
}

void PlateauSelector::exportTo(Selector* s) {
	unsigned int n = q->size();
	for (unsigned int i = 1; i <= n; i++) {
		s->add(q->getPlanAt(i));
	}
}

/*******************************************/
/* Quality Selector                        */
/*******************************************/

void QualitySelector::initialize(float bestQualityFound, uint16_t numAct, Successors* suc) {
	setBestPlanQuality(bestQualityFound, numAct);
	qFF = new SearchQueue(SEARCH_G_HLAND_HFF + SEARCH_PLATEAU);
}

void QualitySelector::setBestPlanQuality(float bestQualityFound, uint16_t numAct) {
	bestQuality = bestQualityFound;
	numActions = numAct;
}

Plan* QualitySelector::poll() {
	Plan* next = qFF->poll();
	while (!improves(next)) {
		if (qFF->size() == 0) return nullptr;
		next = qFF->poll();
	}
	return next;
}

void QualitySelector::add(Plan* p) {
	if (improves(p)) {
		qFF->add(p);
	}
}
