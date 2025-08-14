#ifndef PLANNER_CONCURRENT_H
#define PLANNER_CONCURRENT_H

#include "planner.hpp"

class PlannerConcurrent: public Planner {
private:
	Plan* initialPlan;
	Selector *sel;
	Plateau *plateau;
	Plan *bestPlan;

	void addInitialPlansToSelectors();
	bool emptySearchSpace();
	bool expandBasePlan(Plan* base);
	void addSuccessors(Plan* base);
	void cancelPlateauSearch(bool improve);
	void checkPlateau();

public:
	PlannerConcurrent(SASTask* task, Plan* initialPlan, TState* initialState, bool forceAtEndConditions, 
		bool filterRepeatedStates, bool generateTrace, std::vector<SASAction*>* tilActions, 
		Planner* parentPlanner, float timeout);
	Plan* plan();
	Plan* searchStep();
};

#endif
