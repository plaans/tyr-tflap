#ifndef PLANNER_H
#define PLANNER_H

#include "../sas/sasTask.hpp"
#include "../heuristics/state.hpp"
#include "plan.hpp"
#include "selector.hpp"
#include "successors.hpp"
#include "plateau.hpp"
#include <time.h>

class TILAction {
public:
	double time;
	SASAction* action;

	TILAction(double t, SASAction* a) {
		time= t;
		action = a;
	}
	bool operator<(const TILAction &a) {
		return time < a.time;
	}
};

class Planner {
protected:
	SASTask* task;
	Plan* initialPlan;
	TState* initialState;
	bool forceAtEndConditions;
	bool filterRepeatedStates;
	bool generateTrace;
	Planner* parentPlanner;
	unsigned int expandedNodes;
	Successors* successors;
	std::vector<SASAction*>* tilActions;
	float initialH;
	Plan* solution;
	std::vector<Plan*> sucPlans;
	bool concurrentExpansion;
	QualitySelector qualitySelector;
	float timeout;
	clock_t startTime;

	void writeTrace(std::ofstream& f, Plan* p);
	Plan* createInitialPlan(TState* s);
	void addFrontierNodes(Plan* p);
	void calculateDeadlines();
	void updateState(TState* state, SASAction* a);
	bool timeExceed();

public:
	Planner(SASTask* task, Plan* initialPlan, TState* initialState, bool forceAtEndConditions, 
		bool filterRepeatedStates, bool generateTrace, std::vector<SASAction*>* tilActions, 
		Planner* parentPlanner, float timeout);
	virtual ~Planner();
	virtual Plan* plan() = 0;
	std::string planToPDDL(Plan* p);
	//virtual bool emptySearchSpace();
	virtual Plan* searchStep() = 0;
	unsigned int getExpandedNodes() { return expandedNodes; }
	Plan* improveSolution(uint16_t bestG, float bestGC, bool first);
};

#endif
