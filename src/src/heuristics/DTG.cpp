/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* May 2017                                             */
/********************************************************/
/* Domain transition graphs                             */
/********************************************************/

#include <iostream>
#include "DTG.hpp"
using namespace std;

//#define DEBUG_DTG_ON

/********************************************************/
/* CLASS: DTGTransition                                 */
/********************************************************/

DTGTransition::DTGTransition(TVariable v, SASAction* a, CausalGraph* cg) {
	this->a = a;
	for (unsigned int i = 0; i < a->startCond.size(); i++)
		checkCondition(&(a->startCond[i]), v, cg);
	for (unsigned int i = 0; i < a->overCond.size(); i++)
		checkCondition(&(a->overCond[i]), v, cg);
	for (unsigned int i = 0; i < a->endCond.size(); i++)
		checkCondition(&(a->endCond[i]), v, cg);
}

void DTGTransition::checkCondition(SASCondition* c,  TVariable v, CausalGraph* cg) {
	if (c->var != v && cg->checkDependence(v, c->var)) {
		addCondition(c);
	}
}

void DTGTransition::addCondition(SASCondition* c) {
	for (unsigned int i = 0; i < vars.size(); i++) {
		if (vars[i] == c->var) return;
	}
	vars.push_back(c->var);
	values.push_back(c->value);
}

std::string DTGTransition::toString(SASTask* task) {
	std::string res = a->name;
	if (vars.size() > 0) {
		res += " (" + task->variables[vars[0]].name + "=" + task->values[values[0]].name;
		for (unsigned int i = 1; i < vars.size(); i++) {
			res += "," + task->variables[vars[i]].name + "=" + task->values[values[i]].name;
		}
		res += ")";
	}
	return res;
}

/********************************************************/
/* CLASS: DTGTransitionSet                              */
/********************************************************/

DTGTransitionSet::DTGTransitionSet(TValue fromValue, TValue toValue) {
	this->fromValue = fromValue;
	this->toValue = toValue;
}

void DTGTransitionSet::addTransition(TVariable v, SASAction* a, CausalGraph* cg) {
	// Check that the same action is not repeated for the same transitions
	for (unsigned int i = 0; i < transitions.size(); i++) {
		if (transitions[i].getAction() == a) return;
	}
	transitions.emplace_back(v, a, cg);
}

std::string DTGTransitionSet::toString(SASTask* task) {
	std::string res = "+ " + task->values[fromValue].name + " -> " + task->values[toValue].name + "\n";
	for (unsigned int i = 0; i < transitions.size(); i++) {
		res += "  - " + transitions[i].toString(task) + "\n";
	}
	return res;
}

/********************************************************/
/* CLASS: DTG                                           */
/********************************************************/

void DTG::initialize(DTGSetInterface* dtgSet, TVariable var, SASTask* task, CausalGraph* cg) {
	this->dtgSet = dtgSet;
	this->task = task;
	this->var = var;
	numValues = task->values.size();
	transitionSets.resize(numValues);
	SASVariable &v = task->variables[var];
	for (unsigned int i = 0; i < v.possibleValues.size(); i++) {
		TValue value = (TValue)v.possibleValues[i];
		possibleValues.push_back(value);
		calculateTransitionsToValue(value, cg);
	}
	dijkstraComputed = false;
}

DTG::~DTG() {
}

std::string DTG::toString() {
	std::string res = "DTG of " + task->variables[var].name + "\n";
	for (unsigned int i = 0; i < transitionSets.size(); i++) {
		for (unsigned int j = 0; j < transitionSets[i].size(); j++) {
			res += transitionSets[i][j].toString(task) + "\n";
		}
	}
	return res;
}

void DTG::calculateTransitionsToValue(TValue toValue, CausalGraph* cg) {
	std::vector<SASAction*>& prod = task->producers[var][toValue];
	for (unsigned int i = 0; i < prod.size(); i++) {
		SASAction* a = prod[i];
		TValue fromValue = getFromValue(a);
		if (fromValue != MAX_UINT16) {
			addTransition(fromValue, toValue, a, cg);
		} else { 	// Add transition from all possible values
			for (unsigned int j = 0; j < task->variables[var].possibleValues.size(); j++) {
				fromValue = task->variables[var].possibleValues[j];
				if (!mutex(fromValue, a))
					addTransition(fromValue, toValue, a, cg);
			}
		}
	}
}

TValue DTG::getFromValue(SASAction* a) {
	TValue res = MAX_UINT16;
	for (unsigned int i = 0; i < a->startCond.size(); i++) {
		if (a->startCond[i].var == var)
			return a->startCond[i].value;
	}
	for (unsigned int i = 0; i < a->overCond.size(); i++) {
		if (a->overCond[i].var == var)
			return a->overCond[i].value;
	}
	for (unsigned int i = 0; i < a->endCond.size(); i++) {
		if (a->endCond[i].var == var)
			return a->endCond[i].value;
	}
	return res;
}

bool DTG::mutex(TValue fromValue, SASAction* a) {
	if (fromValue == SASTask::OBJECT_UNDEFINED) return false;
	for (unsigned int i = 0; i < a->startCond.size(); i++) {
		if (a->startCond[i].value == SASTask::OBJECT_UNDEFINED) continue;
		if (task->isMutex(var, fromValue, a->startCond[i].var, a->startCond[i].value))
			return true;
	}
	for (unsigned int i = 0; i < a->overCond.size(); i++) {
		if (a->overCond[i].value == SASTask::OBJECT_UNDEFINED) continue;
		if (task->isMutex(var, fromValue, a->overCond[i].var, a->overCond[i].value))
			return true;
	}
	for (unsigned int i = 0; i < a->endCond.size(); i++) {
		if (a->endCond[i].value == SASTask::OBJECT_UNDEFINED) continue;
		if (task->isMutex(var, fromValue, a->endCond[i].var, a->endCond[i].value))
			return true;
	}
	return false;
}

DTGTransitionSet* DTG::findTransitionSet(TValue fromValue, TValue toValue) {
	std::vector<DTGTransitionSet> &ts = transitionSets[fromValue];	// Transitions from other values to this one
	for (unsigned int i = 0; i < ts.size(); i++) {
		if (ts[i].getToValue() == toValue)
			return &(ts[i]);
	}
	return nullptr;
}

void DTG::addTransition(TValue fromValue, TValue toValue, SASAction* a, CausalGraph* cg) {
	if (fromValue == toValue) return;
	DTGTransitionSet *ts = findTransitionSet(fromValue, toValue);
	if (ts == nullptr) {			// Create new transition set
		transitionSets[fromValue].emplace_back(fromValue, toValue);
		ts = &(transitionSets[fromValue].back());
	}
	ts->addTransition(var, a, cg);
}

void DTG::clearCache(TState* state) {
	cache.clear();
	this->state = state;
	dijkstraComputed = false;
}

float DTG::evaluateConditionCostWithoutContext(TValue value) {
	if (!dijkstraComputed) {
		computeDijkstra(state->state[var]);
	}
	//cout << " + Cost for " << task->variables[var].name << "=" <<
	//		task->values[value].name << " -> " << cache.find(value)->second << endl;
	return cache.find(value)->second;
}

void DTG::computeDijkstra(TValue orig) {
	//cout << " - Computing Dijkstra from " << task->variables[var].name << "=" <<
	//		task->values[orig].name << endl;
	bool* visited = new bool[numValues];
	float* distance = new float[numValues];
	for (unsigned int i = 0; i < possibleValues.size(); i++) {
		TValue v = possibleValues[i];
		visited[v] = false;
		distance[v] = FLOAT_INFINITY;
	}
	distance[orig] = 0;
	PriorityQueue q;
	q.add(new DijkstraValue(orig, 0));
	while (q.size() > 0) {
		DijkstraValue* dv = (DijkstraValue*)q.poll();
		TValue v = dv->value;
		if (!visited[v]) {
			visited[v] = true;
			std::vector<DTGTransitionSet>* tss = &(transitionSets[dv->value]);
			for (unsigned int i = 0; i < tss->size(); i++) {
				DTGTransitionSet* ts = &(tss->at(i));
				TValue w = ts->getToValue();
				float weight = computeWeight(ts);
				if (distance[v] < FLOAT_INFINITY && weight < FLOAT_INFINITY)
					weight += distance[v];
				if (distance[w] > weight) {
					distance[w] = weight;
					q.add(new DijkstraValue(w, weight));
				}
			}
		}
		delete dv;
	}
	for (unsigned int i = 0; i < possibleValues.size(); i++) {
		TValue v = possibleValues[i];
		cache[v] = distance[v];
	}
	delete[] visited;
	delete[] distance;
	dijkstraComputed = true;
}

float DTG::computeWeight(DTGTransitionSet* ts) {
	float bestPathCost = FLOAT_INFINITY;
	unsigned int numTransitions = ts->getNumTransitions();
	for (unsigned int i = 0; i < numTransitions; i++) {
		DTGTransition* t = ts->getTransition(i);
		SASAction* a = t->getAction();
		float pathCost = a->fixedCost ? a->fixedCostValue : task->computeActionCost(a, state->numState, 0);
		if (pathCost >= bestPathCost) continue;
		unsigned int numConditions = t->getNumConditions();
		for (unsigned int j = 0; j < numConditions && pathCost < bestPathCost; j++) {
			float incCost = dtgSet->getDTG(t->getVar(j))->evaluateConditionCostWithoutContext(t->getValue(j));
			if (incCost == FLOAT_INFINITY) pathCost = FLOAT_INFINITY;
			else pathCost += incCost;
		}
		if (pathCost < bestPathCost) {
			bestPathCost = pathCost;
		}
	}
	return bestPathCost;
}

/********************************************************/
/* CLASS: DTGSet                                        */
/********************************************************/

void DTGSet::initialize(SASTask* task, CausalGraph* cg) {
	this->task = task;
	numVariables = task->variables.size();
	dtg = new DTG[numVariables];
	for (unsigned int i = 0; i < numVariables; i++) {
		dtg[i].initialize(this, (TVariable)i, task, cg);
	}
}

float DTGSet::evaluateCostWithoutContext(TState* state) {
	float h = FLOAT_INFINITY;
	clearCache(state);
	for (unsigned int i = 0; i < task->goals.size(); i++) {
		//cout << "-------------------------" << endl;
		float gh = evaluateGoalCostWithoutContext(&(task->goals[i]), state);
		if (gh < h) h = gh;
	}
	return h;
}

void DTGSet::clearCache(TState* state) {
	for (unsigned int i = 0; i < numVariables; i++)
		dtg[i].clearCache(state);
}

float DTGSet::evaluateGoalCostWithoutContext(SASAction* g, TState* state) {
	float h = 0;
	for (unsigned int i = 0; i < g->startCond.size(); i++) {
		SASCondition* c = &(g->startCond[i]);
#ifdef DEBUG_DTG_ON
		cout << "Evaluating condition " << task->variables[c->var].name << "=" <<
				task->values[c->value].name << endl;
#endif
		float incH = dtg[c->var].evaluateConditionCostWithoutContext(c->value);
		if (incH == FLOAT_INFINITY) {
			return FLOAT_INFINITY;
		}
		h += incH;
	}
	return h;
}
