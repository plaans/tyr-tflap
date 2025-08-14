#include "evaluator.hpp"
#include <iostream>
#include <assert.h>
#include <time.h>
#include "hFF.hpp"
using namespace std;

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* May 2017                                             */
/********************************************************/
/* Plan heuristic evaluator                             */
/********************************************************/

/********************************************************/
/* CLASS: Evaluator                                     */
/********************************************************/

void Evaluator::evaluate(Plan* p, TState* state, float makespan, bool helpfulActions) {
	p->hLand = landmarks.countUncheckedNodes();
	/*
	SASAction &g = task->goals[0];
	for (unsigned int i = 0; i < g.startCond.size(); i++) {
		RPG rpg(state, task, forceAtEndConditions, tilActions);
		TVariable v = g.startCond[i].var;
		TValue value = g.startCond[i].value;
		cout << task->values[value].name << endl;
		uint16_t h = rpg.evaluateSingle(v, value);
		cout << "H= " << h << endl;
	}
	exit(0);
	*/
	RPG rpg(state, task, forceAtEndConditions, tilActions);
	p->h = rpg.evaluate(task->hasPermanentMutexAction());
	if (priorityGoals != nullptr) {
		p->hAux = rpg.evaluate(priorityGoals, task->hasPermanentMutexAction());
	}
}

void Evaluator::initialize(TState* state, SASTask* task, std::vector<SASAction*>* a, bool forceAtEndConditions) {
	this->task = task;
	this->forceAtEndConditions = forceAtEndConditions;
	tilActions = a;
	if (state == nullptr) landmarks.initialize(task, a);
	else landmarks.initialize(state, task, a);
	//if (informativeLandmarks()) {
	//cout << ";Computed landmarks: " << landmarks.getNumInformativeNodes() << endl;
	//}
	//CausalGraph causalGraph(task);
	//dtgs.initialize(task, &causalGraph);
	//initialSearch = true;
}

bool Evaluator::informativeLandmarks() {
	return landmarks.getNumInformativeNodes() > 0;
}

/*
void Evaluator::setImproveSearch() {
	initialSearch = false;
}
*/

float Evaluator::evaluateCG(TState* state) {
	//return dtgs.evaluateCostWithoutContext(state);
	return 0;
}
