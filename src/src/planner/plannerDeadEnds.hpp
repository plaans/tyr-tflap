#ifndef PLANNER_DEADENDS_H
#define PLANNER_DEADENDS_H

#include "planner.hpp"

class PlannerDeadEnds: public Planner {
private:
	Plan* initialPlan;
	Selector *selA, *selB, *currentSelector, *otherSelector;
	bool currentSelectorA;
	//Plateau *plateauA, *plateauB, *currentPlateau;
	Plan *bestPlanA, *bestPlanB;

	void addInitialPlansToSelectors();
	bool emptySearchSpace();
	void setCurrentSelector();
	bool expandBasePlan(Plan* base);
	void addSuccessors(Plan* base);
	//void cancelPlateauSearch(bool improve);
	//void checkPlateau();

public:
	PlannerDeadEnds(SASTask* task, Plan* initialPlan, TState* initialState, bool forceAtEndConditions, 
		bool filterRepeatedStates, bool generateTrace, std::vector<SASAction*>* tilActions, 
		Planner* parentPlanner, float timeout);
	Plan* plan();
	Plan* searchStep();
};

#endif
