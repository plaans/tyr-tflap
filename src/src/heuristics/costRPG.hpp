#ifndef COSTRPG_H
#define COSTRPG_H

#include <unordered_map>
#include "../utils/utils.hpp"
#include "../utils/priorityQueue.hpp"
#include "../sas/sasTask.hpp"
#include "state.hpp"
#include "temporalRPG.hpp"

class CostRPG {
private:
	SASTask* task;
	int numActions;
	std::unordered_map<TVarValue, float> firstGenerationCost;
	PriorityQueue qPNormal;
	float* actionCostLevel;
	
	void init(TState* state, float makespan);
	inline float getFirstGenerationCost(TVariable v, TValue value) {
		return getFirstGenerationCost(SASTask::getVariableValueCode(v, value));
	}
	inline float getFirstGenerationCost(TVarValue vv) {
		std::unordered_map<TVarValue, float>::const_iterator got = firstGenerationCost.find(vv);
		return got == firstGenerationCost.end() ? -1 : got->second;
	}
	
public:
	void build(SASTask* task, TState* state, float makespan);
	~CostRPG();
	inline float getActionLevel(SASAction* a) { 
		return actionCostLevel[a->index]; 
	}
	inline float getFluentLevel(TVariable v, TValue value) {
		return getFirstGenerationCost(SASTask::getVariableValueCode(v, value));
	}
};

#endif
