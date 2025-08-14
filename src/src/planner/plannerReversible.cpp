#include "plannerReversible.hpp"
#include <iostream>
using namespace std;

#define PLATEAU_START 100
#define PLATEAU_LIMIT 500

PlannerReversible::PlannerReversible(SASTask* task, Plan* initialPlan, TState* initialState, bool forceAtEndConditions, 
	bool filterRepeatedStates, bool generateTrace, std::vector<SASAction*>* tilActions, Planner* parentPlanner, 
	float timeout)
	: Planner(task, initialPlan, initialState, forceAtEndConditions, filterRepeatedStates, generateTrace, tilActions,
 	parentPlanner, timeout) {
	this->initialPlan = initialPlan;
	successors->evaluate(initialPlan);
	if (tilActions != nullptr && tilActions->empty()) this->tilActions = tilActions = nullptr;
	sel = new Selector();
	if (successors->informativeLandmarks() || 1.5f * initialPlan->hLand >= initialPlan->h) {	// Landmarks available
		if (tilActions != nullptr) {
			sel->addQueue(SEARCH_G_2HFF + SEARCH_PLATEAU);
			sel->addQueue(SEARCH_G_3HLAND + SEARCH_PLATEAU);
		} else {
			sel->addQueue(SEARCH_HFF);
			sel->addQueue(SEARCH_HLAND);
		}
	} else {	// No landmarks available
		sel->addQueue(SEARCH_G_3HFF);
		//selB->addQueue(SEARCH_HFF);
	}
	addInitialPlansToSelectors();
	plateau = nullptr;
	bestPlan = nullptr;
}

void PlannerReversible::addInitialPlansToSelectors() {
	initialH = FLOAT_INFINITY;
	solution = nullptr;
	vector<Plan*> suc;
	successors->computeSuccessors(initialPlan, &suc);
	initialPlan->addChildren(suc);
	for (Plan* p : suc) {
		if (p->isSolution()) {
			solution = p;
			//cout << "Solution found" << endl;
		} else {
			sel->add(p);
		}
		if (p->h < initialH) initialH = p->h;
	}
}


Plan* PlannerReversible::plan() {
	std::ofstream traceFile;
	if (generateTrace) {
		traceFile.open("trace.txt");
		writeTrace(traceFile, initialPlan);
	}
	while (solution == nullptr && !emptySearchSpace() && !timeExceed()) {
		searchStep();
		if (generateTrace) {
			writeTrace(traceFile, base);
			if (plateau != nullptr) break;
		}
	}
	if (generateTrace) {
		traceFile.close();
		exit(0);
	}
	return solution;
}

bool PlannerReversible::emptySearchSpace() {
	return sel->size() == 0;
}

bool PlannerReversible::expandBasePlan(Plan* base) {
	if (base->expanded()) {
		for (unsigned int i = 0; i < base->childPlans->size(); i++)
			if (sel->add(base->childPlans->at(i)) && plateau != nullptr) {
				cancelPlateauSearch(true);
			}
		return false;
	}
	successors->computeSuccessors(base, &sucPlans);
	++expandedNodes;
	/*	
	if (++expandedNodes % 100 == 0) {
		cout << '.';
	}*/
	if (successors->solution != nullptr) {
		solution = successors->solution;
		return false;
	}
	if (bestPlan == nullptr || base->h < bestPlan->h ||	(base->h == bestPlan->h && base->g <= bestPlan->g)) {
		bestPlan = base;
	}
	return true;
}

void PlannerReversible::addSuccessors(Plan* base) {
	base->addChildren(sucPlans);
	for (Plan* p : sucPlans) {
		if (sel->add(p)) {
			if (plateau != nullptr) {
				cancelPlateauSearch(true);
			}
		}
	}
}

void PlannerReversible::cancelPlateauSearch(bool improve) {
	plateau->exportOpenNodes(sel);
	delete plateau;
	plateau = nullptr;
}

void PlannerReversible::checkPlateau() {
	if (sel->inPlateau(PLATEAU_START)) {
		if (plateau == nullptr && bestPlan != nullptr && bestPlan->h <= sel->getBestH() + 1 && tilActions == nullptr) {
			plateau = new Plateau(task, bestPlan, successors, sel->getBestH(), 1);
			bestPlan = nullptr;
		}
		if (plateau != nullptr) {
			bool improve = plateau->searchStep(sel->getBestH() <= 6);
			if (improve || sel->inPlateau(PLATEAU_LIMIT) || plateau->empty()) {
				if (improve) {
					sel->setBestPlan(plateau->getBestPlan());
				} else {
					sel->setIterationsWithoutImproving(PLATEAU_START);
				}
				cancelPlateauSearch(improve);
			}
		}
	}
}

Plan* PlannerReversible::searchStep() {
	base = sel->poll();
	if (!expandBasePlan(base)) return nullptr;
	addSuccessors(base);
	checkPlateau();
	return base;
}
