#ifndef PLATEAU_H
#define PLATEAU_H

#include "../sas/sasTask.hpp"
#include "../heuristics/state.hpp"
#include "plan.hpp"
#include "successors.hpp"
#include "selector.hpp"

class Plateau {
private:
	SASTask* task;
	Plan* initialPlan;
	Successors* successors;
	float hToImprove;
	PlateauSelector* selector;
	std::vector<Plan*> suc;
	int selectorIndex;
	Plan* bestPlan;
	std::vector<TVarValue> priorityGoals;

	void addOpenNodes(Plan* p);
	void calculatePriorityGoals();

public:
	Plateau(SASTask* sTask, Plan* initPlan, Successors* s, float h, int selectorIndex);
	~Plateau();
	bool searchStep(bool concurrent);
	inline Plan* getBestPlan() { return bestPlan; }
	void exportOpenNodes(Selector* s) { selector->exportTo(s); }
	inline bool empty() { return selector->size() == 0; }
	inline int getSelectorIndex() { return selectorIndex; }
};

#endif
