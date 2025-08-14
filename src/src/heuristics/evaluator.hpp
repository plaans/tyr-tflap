#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <deque>
#include "../utils/utils.hpp"
#include "../sas/sasTask.hpp"
#include "../planner/plan.hpp"
#include "state.hpp"
#include "hLand.hpp"
#include "causalGraph.hpp"
#include "DTG.hpp"

class Evaluator {
private:
	SASTask* task;
	LandmarkHeuristic landmarks;
	std::vector<SASAction*>* tilActions;
	bool forceAtEndConditions;
	std::vector<TVarValue>* priorityGoals;

public:
	void initialize(TState* state, SASTask* task, std::vector<SASAction*>* a, bool forceAtEndConditions);
	void evaluate(Plan* p, TState* state, float makespan, bool helpfulActions);
	inline LandmarkHeuristic* getLandmarkHeuristic() {
		return &landmarks;
	}
	float evaluateWithoutContext(TState* state);
	bool informativeLandmarks();
	float evaluateCG(TState* state);
	std::vector<SASAction*>* getTILActions() { return tilActions; }
	void setPriorityGoals(std::vector<TVarValue>* priorityGoals) { this->priorityGoals = priorityGoals; }
};

#endif
