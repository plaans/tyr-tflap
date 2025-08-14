#ifndef MEMOIZATION_H
#define MEMOIZATION_H

#include <unordered_map>
#include "plan.hpp"
#include "linearizer.hpp"
#include "../heuristics/state.hpp"

class Memoization {
private:
	SASTask* task;
	TState* initialState;
	std::unordered_map<uint64_t, std::vector<Plan*>* > memo;
	Linearizer linearizer;

	bool sameState(TState* state, Plan* pc);

public:
	Memoization();
	void initialize(SASTask* task);
	bool isRepeatedState(Plan* p, TState* state);
	void clear();
};

#endif
