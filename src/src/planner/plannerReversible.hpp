#ifndef PLANNER_REVERSIBLE_H
#define PLANNER_REVERSIBLE_H

#include "planner.hpp"

class PlannerReversible: public Planner {
private:
	Plan* initialPlan;
	Selector *sel;
	Plateau *plateau;
	Plan *bestPlan;
	Plan* base;

	void addInitialPlansToSelectors();
	bool emptySearchSpace();
	bool expandBasePlan(Plan* base);
	void addSuccessors(Plan* base);
	void cancelPlateauSearch(bool improve);
	void checkPlateau();

public:
	PlannerReversible(SASTask* task, Plan* initialPlan, TState* initialState, bool forceAtEndConditions, 
		bool filterRepeatedStates, bool generateTrace, std::vector<SASAction*>* tilActions, 
		Planner* parentPlanner, float timeout);
	Plan* plan();
	Plan* searchStep();
};

#endif
