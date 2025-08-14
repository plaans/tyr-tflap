#ifndef DTG_H
#define DTG_H

#include "../utils/utils.hpp"
#include "../utils/priorityQueue.hpp"
#include "../sas/sasTask.hpp"
#include "state.hpp"
#include "causalGraph.hpp"

class DTG;

class DTGSetInterface {
public:
	virtual ~DTGSetInterface() { };
	virtual DTG* getDTG(TVariable v) = 0;
};

class DTGTransition {
private:
	SASAction* a;
	std::vector<TVariable> vars;
	std::vector<TValue> values;

	void checkCondition(SASCondition* c,  TVariable v, CausalGraph* cg);
	void addCondition(SASCondition* c);

public:
	DTGTransition(TVariable v, SASAction* a, CausalGraph* cg);
	inline SASAction* getAction() { return a; }
	inline unsigned int getNumConditions() { return vars.size(); }
	inline TVariable getVar(unsigned int i) { return vars[i]; }
	inline TValue getValue(unsigned int i) { return values[i]; }
	std::string toString(SASTask* task);
};

class DTGTransitionSet {
private:
	TValue fromValue;
	TValue toValue;
	std::vector<DTGTransition> transitions;

public:
	DTGTransitionSet(TValue fromValue, TValue toValue);
	inline TValue getToValue() { return toValue; }
	void addTransition(TVariable v, SASAction* a, CausalGraph* cg);
	inline unsigned int getNumTransitions() { return transitions.size(); }
	inline DTGTransition* getTransition(unsigned int i) { return &(transitions[i]); }
	std::string toString(SASTask* task);
};

class DijkstraValue : public PriorityQueueItem {
public:
	TValue value;
	float level;
	DijkstraValue(TValue val, float lev) {
		value = val;
		level = lev;
	}
	virtual inline int compare(PriorityQueueItem* other) {
		float otherLevel = ((DijkstraValue*)other)->level;
		if (level < otherLevel) return -1;
		else if (level > otherLevel) return 1;
		else return 0;
	}
	std::string toString(SASTask* task) {
		return "(" + task->values[value].name + ") -> " + std::to_string(level);
	}
};

class DTG {
private:
	DTGSetInterface* dtgSet;
	TVariable var;
	SASTask* task;
	std::vector< std::vector<DTGTransitionSet> > transitionSets;	// For each value, a set of transitions from other values
	std::vector<TValue> possibleValues;
	std::unordered_map<TValue, float> cache;
	TState* state;
	unsigned int numValues;
	bool dijkstraComputed;

	void calculateTransitionsToValue(TValue toValue, CausalGraph* cg);
	TValue getFromValue(SASAction* a);
	bool mutex(TValue fromValue, SASAction* a);
	void addTransition(TValue fromValue, TValue toValue, SASAction* a, CausalGraph* cg);
	void computeDijkstra(TValue orig);
	float computeWeight(DTGTransitionSet* ts);

public:
	void initialize(DTGSetInterface* dtgSet, TVariable var, SASTask* task, CausalGraph* cg);
	~DTG();
	DTGTransitionSet* findTransitionSet(TValue fromValue, TValue toValue);
	void clearCache(TState* state);
	float evaluateConditionCostWithoutContext(TValue value);
	std::string toString();
};

class DTGSet : public DTGSetInterface {
private:
	SASTask* task;

	void clearCache(TState* state);
	float evaluateGoalCostWithoutContext(SASAction* g, TState* state);

public:
	DTG* dtg;		// Array of DTGs, one DTG per variable
	unsigned int numVariables;

	void initialize(SASTask* task, CausalGraph* cg);
	float evaluateCostWithoutContext(TState* state);
	inline DTG* getDTG(TVariable v) { return &(dtg[v]); }
};

#endif
