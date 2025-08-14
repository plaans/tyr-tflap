/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* March 2016                                           */
/********************************************************/
/* Non-temporal relaxed planning graph                  */
/********************************************************/

#include <iostream>
#include <time.h>
#include "hFF.hpp"
using namespace std;

//#define DEBUG_RPG_ON
#define PENALTY 8

RPGVarValue::RPGVarValue(TVariable var, TValue value) {
	this->var = var;
	this->value = value;
}

RPG::RPG(vector< vector<TValue> > &varValues, SASTask* task, bool forceAtEndConditions, std::vector<SASAction*>* tilActions) {
	//debug = tilActions == nullptr;
	this->task = task;
	this->forceAtEndConditions = forceAtEndConditions;
	initialize();
	for (unsigned int i = 0; i < varValues.size(); i++) {
		for (unsigned int j = 0; j < varValues[i].size(); j++) {
			lastLevel->emplace_back(i, varValues[i][j]);
			literalLevels[i][varValues[i][j]] = 0;
		}
	}
	if (tilActions != nullptr) {
		addTILactions(tilActions);
	}
	expand();
}

RPG::RPG(TState* state, SASTask* task, bool forceAtEndConditions, std::vector<SASAction*>* tilActions) {
	this->task = task;
	this->forceAtEndConditions = forceAtEndConditions;
	initialize();
	//cout << "STATE:" << endl;
	for (unsigned int i = 0; i < state->numSASVars; i++) {
		TValue v = state->state[i];
		lastLevel->emplace_back(i, v);
		literalLevels[i][v] = 0;
		//cout << "(" << task->variables[i].name << ", " << task->values[v].name << ") -> Level 0" << endl;
	}
	if (tilActions != nullptr) {
		addTILactions(tilActions);
	}
	expand();
}

void RPG::addTILactions(std::vector<SASAction*>* tilActions) {
	for (unsigned int i = 0; i < tilActions->size(); i++) {
		SASAction* a = tilActions->at(i);
		for (unsigned int j = 0; j < a->endEff.size(); j++) {
			TVariable v = a->endEff[j].var;
			TValue value = a->endEff[j].value;
			if (literalLevels[v][value] != 0) {
				lastLevel->emplace_back(v, value);
				literalLevels[v][value] = 0;
			}
		}
	}
}

void RPG::expand() {
	numLevels = 0;
	while (lastLevel->size() > 0) {
		newLevel->clear();
		for (unsigned int i = 0; i < lastLevel->size(); i++) {
			TVariable var = (*lastLevel)[i].var;
			TValue value = (*lastLevel)[i].value;
#ifdef DEBUG_RPG_ON
			cout << "(" << task->variables[var].name << "," << task->values[value].name << ")" << endl;
#endif
			vector<SASAction*> &actions = task->requirers[var][value];
#ifdef DEBUG_RPG_ON
			cout << actions.size() << " actions" << endl;
#endif
			for (unsigned int j = 0; j < actions.size(); j++) {
				SASAction* a = actions[j];
				if (actionLevels[a->index] == MAX_INT32 && isExecutable(a)) {
#ifdef DEBUG_RPG_ON
					cout << "[" << numLevels << "] " << a->name << endl;
#endif
					actionLevels[a->index] = numLevels;
					addEffects(a);
				}
			}
		}
		if (numLevels == 0) {
			for (unsigned int j = 0; j < task->actionsWithoutConditions.size(); j++) {
				SASAction* a = task->actionsWithoutConditions[j];
				actionLevels[a->index] = numLevels;
				addEffects(a);
			}
		}
		numLevels++;
		for (unsigned int i = 0; i < newLevel->size(); i++) {
			literalLevels[(*newLevel)[i].var][(*newLevel)[i].value] = numLevels;
		}
		vector<RPGVarValue>* aux = lastLevel;
		lastLevel = newLevel;
		newLevel = aux;
	}
	delete lastLevel;
	delete newLevel;
#ifdef DEBUG_RPG_ON
	cout << "There are " << numLevels << " levels" << endl;
#endif
}

bool RPG::isExecutable(SASAction* a) {
	for (unsigned int i = 0; i < a->startCond.size(); i++) {
		if (literalLevels[a->startCond[i].var][a->startCond[i].value] == MAX_INT32)
			return false;
	}
	for (unsigned int i = 0; i < a->overCond.size(); i++) {
		if (literalLevels[a->overCond[i].var][a->overCond[i].value] == MAX_INT32)
			return false;
	}
	if (forceAtEndConditions) {
		for (unsigned int i = 0; i < a->endCond.size(); i++) {
			if (literalLevels[a->endCond[i].var][a->endCond[i].value] == MAX_INT32)
				return false;
		}
	}
	return true;
}

void RPG::addEffects(SASAction* a) {
	for (unsigned int i = 0; i < a->startEff.size(); i++) {
		addEffect(a->startEff[i].var, a->startEff[i].value);
	}
	for (unsigned int i = 0; i < a->endEff.size(); i++) {
		addEffect(a->endEff[i].var, a->endEff[i].value);
	}
}

void RPG::addEffect(TVariable var, TValue value) {
	if (literalLevels[var][value] == MAX_INT32) {
		for (unsigned int i = 0; i < newLevel->size(); i++)
			if ((*newLevel)[i].var == var && (*newLevel)[i].value == value)
				return;
		newLevel->emplace_back(var, value);
#ifdef DEBUG_RPG_ON
		cout << "* " << task->variables[var].name << " = " << task->values[value].name << endl;
#endif
	}
}

void RPG::initialize() {
	unsigned int numVars = task->variables.size();
	literalLevels.resize(numVars);
	for (unsigned int i = 0; i < numVars; i++) {
		literalLevels[i].resize(task->values.size(), MAX_INT32);
	}
	actionLevels.resize(task->actions.size(), MAX_INT32);
	lastLevel = new vector<RPGVarValue>();
	newLevel = new vector<RPGVarValue>();
}

void RPG::resetReachedValues() {
	for (unsigned int i = 0; i < reachedValues.size(); i++) {
		TVariable v = SASTask::getVariableIndex(reachedValues[i]);
		TValue value = SASTask::getValueIndex(reachedValues[i]);
		if (literalLevels[v][value] < 0)
			literalLevels[v][value] = -literalLevels[v][value];
	}
	reachedValues.clear();
}

uint16_t RPG::computeHeuristic(bool mutex, PriorityQueue* openConditions) {
	int gLevel;
	uint16_t bestCost;
	uint16_t h = 0;
	while (openConditions->size() > 0) {
		RPGCondition* g = (RPGCondition*)openConditions->poll();
		//if (debug) cout << "Condition: " << task->variables[g->var].name << " = " << task->values[g->value].name << " (level " << literalLevels[g->var][g->value] << ")" << endl;
#ifdef DEBUG_RPG_ON
		cout << "Condition: " << task->variables[g->var].name << " = " << task->values[g->value].name << " (level " << literalLevels[g->var][g->value] << ")" << endl;
#endif
		gLevel = literalLevels[g->var][g->value];
		if (gLevel <= 0) {
			delete g;
			continue;
		}
		if (gLevel == MAX_INT32) return MAX_UINT16;
		literalLevels[g->var][g->value] = -gLevel;
		reachedValues.push_back(SASTask::getVariableValueCode(g->var, g->value));
		vector<SASAction*> &prod = task->producers[g->var][g->value];
		SASAction* bestAction = nullptr;
		bestCost = MAX_UINT16;
		for (unsigned int i = 0; i < prod.size(); i++) {
			SASAction* a = prod[i];
#ifdef DEBUG_RPG_ON
			cout << a->name << ", dif. " << getDifficulty(a) << endl;
#endif			
			if (gLevel == actionLevels[a->index] + 1) {
				if (bestAction == nullptr) {
					bestAction = a;
					bestCost = mutex ? getDifficultyWithPermanentMutex(a) : getDifficulty(a);
					if (bestCost == 0) break;
				}
				else {
					uint16_t cost = mutex ? getDifficultyWithPermanentMutex(a) : getDifficulty(a);
					if (cost < bestCost) {
						bestAction = a;
						bestCost = cost;
						if (bestCost == 0) {
							break;
						}
					}
				}
			}
		}
		delete g;
		if (bestAction != nullptr) {
			//if (debug) cout << bestAction->name << endl;
#ifdef DEBUG_RPG_ON
			cout << "* Best action = " << bestAction->name << ", cost " << bestCost << endl;
#endif
			h++;
			addSubgoals(bestAction, openConditions);
		}
		else {
#ifdef DEBUG_RPG_ON
			cout << "* No producers" << endl;
#endif
			return MAX_UINT16;
		}
	}
#ifdef DEBUG_RPG_ON
	cout << "H = " << h << endl;
#endif
	return h;
}

uint16_t RPG::evaluate(bool mutex) {
	resetReachedValues();
	PriorityQueue openConditions(128);
	addSubgoals(task->getListOfGoals(), &openConditions);
	return computeHeuristic(mutex, &openConditions);
}

uint16_t RPG::evaluate(TVarValue goal, bool mutex) {
	resetReachedValues();
	PriorityQueue openConditions(128);
	addSubgoal(SASTask::getVariableIndex(goal), SASTask::getValueIndex(goal), &openConditions);
	return computeHeuristic(mutex, &openConditions);
}

uint16_t RPG::evaluate(std::vector<TVarValue>* goals, bool mutex) {
	resetReachedValues();
	PriorityQueue openConditions(128);
	for (unsigned int i = 0; i < goals->size(); i++) {
		TVarValue vv = goals->at(i);
		addSubgoal(SASTask::getVariableIndex(vv), SASTask::getValueIndex(vv), &openConditions);
	}
	return computeHeuristic(mutex, &openConditions);
}

void RPG::addUsefulAction(SASAction* a, std::vector<SASAction*>* usefulActions) {
	for (unsigned int i = 0; i < usefulActions->size(); i++) {
		if (usefulActions->at(i) == a) return;
	}
	usefulActions->push_back(a);
}

void RPG::addSubgoals(std::vector<TVarValue>* goals, PriorityQueue* openConditions) {
	TVariable var;
	TValue value;
	for (unsigned int i = 0; i < goals->size(); i++) {
		var = SASTask::getVariableIndex(goals->at(i));
		value = SASTask::getValueIndex(goals->at(i));
		addSubgoal(var, value, openConditions);
	}
}

void RPG::addSubgoal(TVariable var, TValue value, PriorityQueue* openConditions) {
	int level = literalLevels[var][value];
	if (level > 0) {
		openConditions->add(new RPGCondition(var, value, level));
#ifdef DEBUG_RPG_ON
		cout << "* Adding subgoal: " << task->variables[var].name << " = " << task->values[value].name << " (level " << level << ")" << endl;
#endif
	}
}

void RPG::addSubgoals(SASAction* a, PriorityQueue* openConditions) {
	TVariable var;
	TValue value;
	// Add the conditions of the action that do not hold in the frontier state as subgoals 
	for (unsigned int i = 0; i < a->startCond.size(); i++) {
		var = a->startCond[i].var;
		value = a->startCond[i].value;
		addSubgoal(var, value, openConditions);
	}
	for (unsigned int i = 0; i < a->overCond.size(); i++) {
		var = a->overCond[i].var;
		value = a->overCond[i].value;
		addSubgoal(var, value, openConditions);
	}
	if (forceAtEndConditions) {
		for (unsigned int i = 0; i < a->endCond.size(); i++) {
			var = a->endCond[i].var;
			value = a->endCond[i].value;
			addSubgoal(var, value, openConditions);
		}
	}
}

uint16_t RPG::getDifficulty(SASAction* a) {
	uint16_t cost = 0;
	for (unsigned int i = 0; i < a->startCond.size(); i++) {
		cost += getDifficulty(&(a->startCond[i]));
	}
	for (unsigned int i = 0; i < a->overCond.size(); i++) {
		cost += getDifficulty(&(a->overCond[i])); 
	}
	if (forceAtEndConditions) {
		for (unsigned int i = 0; i < a->endCond.size(); i++) {
			cost += getDifficulty(&(a->endCond[i]));
		}
	}
	//cout << " * Difficulty: " << cost << endl;
	return cost;
}

uint16_t RPG::getDifficultyWithPermanentMutex(SASAction* a) {
	for (unsigned int i = 0; i < relaxedPlan.size(); i++) {
		if (task->isPermanentMutex(a, relaxedPlan[i])) return MAX_UINT16;
	}
	uint16_t cost = 0;
	for (unsigned int i = 0; i < a->startCond.size(); i++) {
		cost += getDifficulty(&(a->startCond[i]));
	}
	for (unsigned int i = 0; i < a->overCond.size(); i++) {
		cost += getDifficulty(&(a->overCond[i]));
	}
	if (forceAtEndConditions) {
		for (unsigned int i = 0; i < a->endCond.size(); i++) {
			cost += getDifficulty(&(a->endCond[i]));
		}
	}
	return cost;
}

uint16_t RPG::getDifficulty(SASCondition* c) {
	int level = literalLevels[c->var][c->value];
	//cout << " * Dif. of (" << task->variables[c->var].name << ", " << task->values[c->value].name << "): " << level << endl;
	return level > 0 ? level : 0;
}
