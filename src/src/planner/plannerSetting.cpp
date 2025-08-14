#include "plannerSetting.hpp"
#include "linearizer.hpp"
#include "../heuristics/hFF.hpp"
#include "plannerConcurrent.hpp"
#include "plannerReversible.hpp"
#include "plannerDeadEnds.hpp"
#include <iostream>
using namespace std;

#define toSeconds(t) (float) (((int) (1000 * (clock() - t)/(float) CLOCKS_PER_SEC))/1000.0)

PlannerSetting::PlannerSetting(SASTask* sTask, bool generateTrace, float timeout) {
	initialTime = clock();
	this->timeout = timeout;
	this->task = sTask;
	this->generateTrace = generateTrace;
	createInitialPlan();
	forceAtEndConditions = checkForceAtEndConditions();
	filterRepeatedStates = checkRepeatedStates();
	initialState = new TState(task);
	task->tilActions = !tilActions.empty();
	checkPlannerType();
}

void PlannerSetting::checkPlannerType() {
	//cout << ";Open end-cond.: " << (forceAtEndConditions ? 'N' : 'Y');
	//cout << "   Memo: " << (filterRepeatedStates ? 'Y' : 'N');
	//cout << "   Mutex: " << (task->hasPermanentMutexAction() ? 'Y' : 'N') << endl;
	float remainingTime = timeout - toSeconds(initialTime);	
	if (!filterRepeatedStates || !forceAtEndConditions) {
		task->domainType = DOMAIN_CONCURRENT;
		//cout << ";Concurrent domain" << endl;
		planner = new PlannerConcurrent(task, initialPlan, initialState, forceAtEndConditions, filterRepeatedStates,
				generateTrace, &tilActions, nullptr, remainingTime);
	}
	else if (task->hasPermanentMutexAction()) {
		task->domainType = DOMAIN_DEAD_ENDS;
		//cout << ";Non-reversible domain (possible dead-ends)" << endl;
		planner = new PlannerDeadEnds(task, initialPlan, initialState, forceAtEndConditions, filterRepeatedStates,
				generateTrace, &tilActions, nullptr, remainingTime);
	}
	else {
		task->domainType = DOMAIN_REVERSIBLE;
		//cout << ";Reversible domain" << endl;
		planner = new PlannerReversible(task, initialPlan, initialState, forceAtEndConditions, filterRepeatedStates,
				generateTrace, &tilActions, nullptr, remainingTime);
	}
}

Plan* PlannerSetting::plan() {
	return planner->plan();
}

Plan* PlannerSetting::improveSolution(uint16_t bestG, float bestGC, bool first) {
	return planner->improveSolution(bestG, bestGC, first);
}

unsigned int PlannerSetting::getExpandedNodes() {
	return planner->getExpandedNodes();
}

std::string PlannerSetting::planToPDDL(Plan* p) {
	return planner->planToPDDL(p);
}

// Creates the initial empty plan that only contains the initial and the TIL fictitious actions
void PlannerSetting::createInitialPlan() {
	SASAction* initialAction = createInitialAction();
	initialPlan = new Plan(initialAction, nullptr, 0);
	initialPlan = createTILactions(initialPlan);
}

// Creates and returns the initial fictitious action
SASAction* PlannerSetting::createInitialAction() {
	vector<unsigned int> varList;
	for (unsigned int i = 0; i < task->variables.size(); i++) {	// Non-numeric effects
		SASVariable &var = task->variables[i];
		for (unsigned int j = 0; j < var.value.size(); j++) {
			if (var.time[j] == 0) {								// Initial state effect
				varList.push_back(i);
				break;
			}
		}
	}
	for (unsigned int i = 0; i < task->numVariables.size(); i++) {	// Numeric effects
		NumericVariable &var = task->numVariables[i];
		for (unsigned int j = 0; j < var.value.size(); j++) {
			if (var.time[j] == 0) {									// Initial state effect
				varList.push_back(i + task->variables.size());
				break;
			}
		}
	}
	return createFictitiousAction(EPSILON, varList, 0, "#initial", false);
}

// Create a fictitious action with the given duration and with the effects with the modified variables in the given time point
SASAction* PlannerSetting::createFictitiousAction(float actionDuration, vector<unsigned int> &varList,
		float timePoint, string name, bool isTIL) {
	SASAction* a = new SASAction();
	a->index = MAX_UNSIGNED_INT;
	a->name = name;
	a->isTIL = isTIL;
	SASDuration duration;
	duration.time = 'N';
	duration.comp = '=';
	duration.exp.type = 'N';	// Number (epsilon duration)
	duration.exp.value = actionDuration;
	a->duration.push_back(duration);
	for (unsigned int i = 0; i < varList.size(); i++) {
		unsigned int varIndex = varList[i];
		if (varIndex < task->variables.size()) {	//	Non-numeric effect
			SASVariable &var = task->variables[varIndex];
			for (unsigned int j = 0; j < var.value.size(); j++) {
				if (var.time[j] == timePoint) {
					a->endEff.emplace_back(varIndex, var.value[j]);
					break;
				}
			}
		} else {									// Numeric effect
			varIndex -= task->variables.size();
			NumericVariable &var = task->numVariables[varIndex];
			for (unsigned int j = 0; j < var.value.size(); j++) {
				if (var.time[j] == timePoint) {
					SASNumericEffect eff;
					eff.op = '=';
					eff.var = varIndex;
					eff.exp.type = 'N';				// Number
					eff.exp.value = var.value[j];
					a->endNumEff.push_back(eff);
					break;
				}
			}
		}
	}
	return a;
}

// Adds the fictitious TIL actions to the initial plan. Returns the resulting plan
Plan* PlannerSetting::createTILactions(Plan* parentPlan) {
	Plan* result = parentPlan;
	unordered_map<float, vector<unsigned int> > til;			// Time point -> variables modified at that time
	for (unsigned int i = 0; i < task->variables.size(); i++) {	// Non-numeric effects
		SASVariable &var = task->variables[i];
		for (unsigned int j = 0; j < var.value.size(); j++) {
			if (var.time[j] > 0) {								// Time-initial literal
				if (til.find(var.time[j]) == til.end()) {
					vector<unsigned int> v;
					v.push_back(i);
					til[var.time[j]] = v;
				} else til[var.time[j]].push_back(i);
			}
		}
	}
	for (unsigned int i = 0; i < task->numVariables.size(); i++) {	// Numeric effects
		NumericVariable &var = task->numVariables[i];
		for (unsigned int j = 0; j < var.value.size(); j++) {
			if (var.time[j] > 0) {									// Numeric time-initial literal
				if (til.find(var.time[j]) == til.end()) {
					vector<unsigned int> v;
					v.push_back(i + task->variables.size());
					til[var.time[j]] = v;
				} else til[var.time[j]].push_back(i + task->variables.size());
			}
		}
	}
	for (auto it = til.begin(); it != til.end(); ++it) {
		float timePoint = it->first;
		SASAction* a = createFictitiousAction(timePoint, it->second, timePoint, "#til" + to_string(timePoint), true);
		tilActions.push_back(a);
		result = new Plan(a, result, timePoint, 0);
	}
	return result;
}

bool PlannerSetting::checkForceAtEndConditions() {	// Check if it's required to leave at-end conditions not supported for some actions
	vector< vector<unsigned short> > varValues;
	varValues.resize(task->variables.size());
	for (unsigned int i = 0; i < task->variables.size(); i++) {
		SASVariable &var = task->variables[i];
		for (unsigned int j = 0; j < var.value.size(); j++) {
			varValues[i].push_back((unsigned short)var.value[j]);
		}
	}
	RPG rpg(varValues, task, true, &tilActions);
	for (unsigned int i = 0; i < task->goals.size(); i++) {
		if (rpg.isExecutable(&(task->goals[i])))
			return true;
	}
	return false;
}

bool PlannerSetting::checkRepeatedStates() {
	TVariable v;
	TValue value;
	for (SASAction& a: task->actions) {
		for (unsigned int i = 0; i < a.startEff.size(); i++) {
			v = a.startEff[i].var;
			value = a.startEff[i].value;
			for (unsigned int j = 0; j < a.endEff.size(); j++) {
				if (a.endEff[j].var == v && a.endEff[j].value != value) {
					// If (v = value) is required by any action, the domain is concurrent
					vector<SASAction*> &req = task->requirers[v][value];
					for (unsigned int k = 0; k < req.size(); k++) {
						if (req[k] != &a) {
							return false;
						}
					}
					break;
				}
			}
		}
	}
	return true;
}
