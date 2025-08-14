#ifndef SELECTOR_H
#define SELECTOR_H

#include <unordered_map>
#include <vector>
#include "plan.hpp"
#include "successors.hpp"

class SearchQueue {
private:
	const static unsigned int INITIAL_PQ_CAPACITY = 8192;
	int index;
	std::unordered_map<uint32_t, uint32_t> planPosition;
	std::vector<Plan*> pq;

	void heapify(unsigned int gap);

public:
	float bestH;			// For queue alternating
	bool improvedH;

	SearchQueue(int index);
	void add(Plan* p);
	Plan* poll();
	void remove(Plan* p);
	inline Plan* peek() { return pq[1]; }
	inline int size() { return pq.size() - 1; }
	inline Plan* getPlanAt(unsigned int i) { return pq[i]; }
	void clear();
	inline int getIndex() { return index; }
};

class Selector {
private:
	std::vector<SearchQueue*> queues;
	int currentQueue;
	Plan* overallBestPlan;	// Best hFF values found
	float overallBest;
	int iterationsWithoutImproving;

public:
	Selector();
	void addQueue(int qtype);
	Plan* poll();
	inline Plan* getPlanAt(unsigned int i) { return queues[0]->getPlanAt(i); }
	inline unsigned int size() { return queues[0]->size(); }
	bool add(Plan* p);
	void exportTo(Selector* s);
	inline bool inPlateau(int plateauStart) { return iterationsWithoutImproving >= plateauStart; }
	inline float getBestH() { return overallBest; }
	inline void setBestPlan(Plan* p) {
		iterationsWithoutImproving = 0;
		overallBestPlan = p;
		overallBest = p->h;
	}
	inline void setIterationsWithoutImproving(int n) { iterationsWithoutImproving = n; }
	inline Plan* getBestPlan() { return overallBestPlan; }
	void clear();
};

class PlateauSelector {
private:
	SearchQueue* q;		// Priority queue

public:
	PlateauSelector(int qtype);
	inline unsigned int size() { return q->size(); }
	Plan* poll();
	Plan* randomPoll();
	void add(Plan* p);
	void exportTo(Selector* s);
};

class QualitySelector {
private:
	SearchQueue* qFF;
	float bestQuality;
	uint16_t numActions;

public:
	void initialize(float bestQualityFound, uint16_t numAct, Successors* suc);
	void setBestPlanQuality(float bestQualityFound, uint16_t numAct);
	inline unsigned int size() { return qFF->size(); }
	Plan* poll();
	void add(Plan* p);
	inline bool improves(Plan* p) {
		float distanceToBest = bestQuality - p->gc;
		return (distanceToBest > EPSILON) || (distanceToBest >= 0 && p->g < numActions);
	}
};

#endif
