#include "costRPG.hpp"
#include <algorithm>
#include <iostream>
using namespace std;

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* May 2017                                             */
/********************************************************/
/* Metric-based relaxed planning graph                  */
/********************************************************/

CostRPG::~CostRPG() {
	delete[] actionCostLevel;
}

void CostRPG::build(SASTask* task, TState* state, float makespan) {
	this->task = task;
	numActions = task->actions.size();
	actionCostLevel = new float[numActions];
	for (int i = 0; i < numActions; i++) {
		actionCostLevel[i] = -1;
	}
	init(state, makespan);
	float auxLevel, effLevel;
	while (qPNormal.size() > 0) {
		FluentLevel* fl = (FluentLevel*) qPNormal.poll();
		std::vector<SASAction*> &req = task->requirers[fl->variable][fl->value];
		//cout << "EXTR.: " << fl->toString(task) << ", " << req.size() << " requirers" << endl;
		for (unsigned int i = 0; i < req.size(); i++) {
			SASAction* a = req[i];
			if (actionCostLevel[a->index] < 0) {
				float actionLevel = 0;
				for (unsigned int j = 0; j < a->startCond.size(); j++) {
					auxLevel = getFirstGenerationCost(a->startCond[j].var, a->startCond[j].value);
					if (auxLevel < 0 || auxLevel > fl->level) {
						actionLevel = -1;
						break; // Non applicable
					}
					else {
						actionLevel += auxLevel;
					}
				}
				if (actionLevel >= 0) {
					for (unsigned int j = 0; j < a->overCond.size(); j++) {
						auxLevel = getFirstGenerationCost(a->overCond[j].var, a->overCond[j].value);
						if (auxLevel < 0 || auxLevel > fl->level) {
							actionLevel = -1;
							break; // Non applicable
						}
						else {
							actionLevel += auxLevel;
						}
					}
					if (actionLevel >= 0) {
						//cout << "N.ACTION " << actionLevel << ": " << a->name << endl;
						actionCostLevel[a->index] = actionLevel;
						effLevel = actionLevel + EPSILON;
						for (unsigned j = 0; j < a->startEff.size(); j++) {
							TVariable v = a->startEff[j].var;
							TValue value = a->startEff[j].value;
							auxLevel = getFirstGenerationCost(v, value);
							if (auxLevel == -1 || auxLevel > effLevel) {
								firstGenerationCost[SASTask::getVariableValueCode(v, value)] = effLevel;
								qPNormal.add(new FluentLevel(v, value, effLevel));
								//cout << "* PROGs: (" << task->variables[v].name << "," << task->values[value].name << ") -> " << effLevel << endl;
							}
						}
						effLevel += a->fixedCostValue;
						for (unsigned j = 0; j < a->endEff.size(); j++) {
							TVariable v = a->endEff[j].var;
							TValue value = a->endEff[j].value;
							auxLevel = getFirstGenerationCost(v, value);
							if (auxLevel == -1 || auxLevel > effLevel) {
								firstGenerationCost[SASTask::getVariableValueCode(v, value)] = effLevel;
								qPNormal.add(new FluentLevel(v, value, effLevel));
								//cout << "* PROGe: (" << task->variables[v].name << "," << task->values[value].name << ") -> " << effLevel << endl;
							}
						}
					}
				}
			}
		}
		delete fl;
	}
}

void CostRPG::init(TState* state, float makespan) {
	//cout << "Makespan = " << makespan << endl;
	TVariable v;
	TValue value;
	float level;
	if (task->variableCosts) {
		for (int i = 0; i < numActions; i++) {
			SASAction* a = &(task->actions[i]);
			if (!a->fixedCost) {
				a->fixedCostValue = task->computeActionCost(a, state->numState, makespan);
			}
		}
	}
	for (unsigned int i = 0; i < state->numSASVars; i++) {
		firstGenerationCost[SASTask::getVariableValueCode(i, state->state[i])] = 0;
	}
	for (int i = 0; i < numActions; i++) {
		SASAction &a = task->actions[i];
		bool insert = true;
		for (unsigned int j = 0; j < a.startCond.size(); j++) {
			if (state->state[a.startCond[j].var] != a.startCond[j].value) {
				insert = false;
				break;
			}
		}
		if (insert) {
			for (unsigned int j = 0; j < a.overCond.size(); j++) {
				if (state->state[a.overCond[j].var] != a.overCond[j].value) {
					insert = false;
					break;
				}
			}
			if (insert) {
				//cout << "Action executable: " << a.name << endl;
				actionCostLevel[a.index] = EPSILON;
				for (unsigned int j = 0; j < a.startEff.size(); j++) {
					v = a.startEff[j].var;
					value = a.startEff[j].value;
					if (getFirstGenerationCost(v, value) == -1) {
						firstGenerationCost[SASTask::getVariableValueCode(v, value)] = EPSILON;
						qPNormal.add(new FluentLevel(v, value, EPSILON));
						//cout << "* PROG: (" << task->variables[v].name << "," << task->values[value].name << ") -> " << EPSILON << endl;
					}
				}
				for (unsigned int j = 0; j < a.endEff.size(); j++) {
					v = a.endEff[j].var;
					value = a.endEff[j].value;
					if (getFirstGenerationCost(v, value) == -1) {
						level = a.fixedCost + EPSILON;
						firstGenerationCost[SASTask::getVariableValueCode(v, value)] = level;
						qPNormal.add(new FluentLevel(v, value, level));
						//cout << "* PROG: (" << task->variables[v].name << "," << task->values[value].name << ") -> " << level << endl;
					}
				}
			}
		}
	}
}
