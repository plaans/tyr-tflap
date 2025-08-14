#ifndef PLANNER_SETTING_H
#define PLANNER_SETTING_H

#include "../sas/sasTask.hpp"
#include "plan.hpp"
#include "planner.hpp"
#include <time.h>

class PlannerSetting {
private:
	SASTask* task;
	bool generateTrace;
	bool forceAtEndConditions;
	bool filterRepeatedStates;
	Plan* initialPlan;
	TState* initialState;
	std::vector<SASAction*> tilActions;
	Planner* planner;
	float timeout;
	clock_t initialTime;

	void createInitialPlan();
	SASAction* createInitialAction();
	SASAction* createFictitiousAction(float actionDuration, std::vector<unsigned int> &varList,
			float timePoint, std::string name, bool isTIL);
	Plan* createTILactions(Plan* parentPlan);
	bool checkForceAtEndConditions();	// Check if it's required to leave at-end conditions not supported for some actions
	bool checkRepeatedStates();
	void checkPlannerType();

public:
	PlannerSetting(SASTask* sTask, bool generateTrace, float timeout);
	Plan* plan();
	Plan* improveSolution(uint16_t bestG, float bestGC, bool first);
	unsigned int getExpandedNodes();
	std::string planToPDDL(Plan* p);
};

#endif
