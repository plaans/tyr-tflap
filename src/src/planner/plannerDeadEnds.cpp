#include "plannerDeadEnds.hpp"
#include <iostream>
using namespace std;

#define PLATEAU_START 200
#define PLATEAU_LIMIT 400

PlannerDeadEnds::PlannerDeadEnds(SASTask* task, Plan* initialPlan, TState* initialState, bool forceAtEndConditions, 
	bool filterRepeatedStates, bool generateTrace, std::vector<SASAction*>* tilActions, Planner* parentPlanner, 
	float timeout)
	: Planner(task, initialPlan, initialState, forceAtEndConditions, filterRepeatedStates, generateTrace, tilActions,
 	parentPlanner, timeout) {
	this->initialPlan = initialPlan;
	successors->evaluate(initialPlan);
	selA = new Selector();
	selB = new Selector();
	if (successors->informativeLandmarks() || 1.5f * initialPlan->hLand >= initialPlan->h) {	// Landmarks available
		selA->addQueue(SEARCH_G_2HFF);
		selB->addQueue(SEARCH_G_3HLAND);
	} else {	// No landmarks available
		selA->addQueue(SEARCH_G_HFF);
		selB->addQueue(SEARCH_HFF);
	}
	addInitialPlansToSelectors();
	currentSelectorA = true;
	//plateauA = plateauB = currentPlateau = nullptr;
	bestPlanA = bestPlanB = nullptr;
}

void PlannerDeadEnds::addInitialPlansToSelectors() {
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
			selA->add(p);
			selB->add(p);
		}
		if (p->h < initialH) initialH = p->h;
	}
}


Plan* PlannerDeadEnds::plan() {
	while (solution == nullptr && !emptySearchSpace() && !timeExceed()) {
		searchStep();
	}
	return solution;
}

bool PlannerDeadEnds::emptySearchSpace() {
	return selA->size() == 0 && selB->size() == 0;
}

void PlannerDeadEnds::setCurrentSelector() {
	if ((currentSelectorA && selA->size() == 0) || (!currentSelectorA && selB->size() == 0)) {
		currentSelectorA = !currentSelectorA;
	}
	if (currentSelectorA) {
		currentSelector = selA;
		otherSelector = selB;
		//currentPlateau = plateauA;
	} else {
		currentSelector = selB;
		otherSelector = selA;
		//currentPlateau = plateauB;
	}
}

bool PlannerDeadEnds::expandBasePlan(Plan* base) {
	if (base->expanded()) {
		for (unsigned int i = 0; i < base->childPlans->size(); i++)
			currentSelector->add(base->childPlans->at(i));
			/*
			if (currentSelector->add(base->childPlans->at(i)) && currentPlateau != nullptr) {
				cancelPlateauSearch(true);
			}*/
		return false;
	}
	successors->computeSuccessors(base, &sucPlans);
	++expandedNodes;
	/*if (++expandedNodes % 100 == 0) {
		cout << '.';
	}*/
	if (successors->solution != nullptr) {
		solution = successors->solution;
		return false;
	}
	Plan* bestPlan = currentSelectorA ? bestPlanA : bestPlanB;
	if (bestPlan == nullptr || base->h < bestPlan->h ||	(base->h == bestPlan->h && base->g <= bestPlan->g)) {
		if (currentSelectorA) bestPlanA = base;
		else bestPlanB = base;
	}
	return true;
}

void PlannerDeadEnds::addSuccessors(Plan* base) {
	base->addChildren(sucPlans);
	for (Plan* p : sucPlans) {
		if (currentSelector->add(p)) {
			if (currentSelectorA && currentSelector->getBestH() < otherSelector->getBestH()) {
				otherSelector->add(p);
			}
			/*
			if (currentPlateau != nullptr) {
				cancelPlateauSearch(true);
			}
			*/
		}
	}
}

/*
void PlannerDeadEnds::cancelPlateauSearch(bool improve) {
	currentPlateau->exportOpenNodes(currentSelector);
	delete currentPlateau;
	if (currentSelectorA) {
		currentPlateau = plateauA = nullptr;
	}
	else {
		currentPlateau = plateauB = nullptr;
	}
}
*/
/*
void PlannerDeadEnds::checkPlateau() {
	if (currentSelector->inPlateau(PLATEAU_START)) {
		Plan* bestPlan = currentSelectorA ? bestPlanA : bestPlanB;
		if (currentPlateau == nullptr && bestPlan != nullptr && bestPlan->h <= currentSelector->getBestH() + 1) {
			if (currentSelectorA) {
				currentPlateau = plateauA = new Plateau(task, bestPlan, successors, currentSelector->getBestH(), 1);
				bestPlanA = nullptr;
			} else {
				currentPlateau = plateauB = new Plateau(task, bestPlan, successors, currentSelector->getBestH(), 2);
				bestPlanB = nullptr;
			}
		}
		if (currentPlateau != nullptr) {
			bool improve = currentPlateau->searchStep(currentSelector->getBestH() <= 3);
			if (improve || currentSelector->inPlateau(PLATEAU_LIMIT) || currentPlateau->empty()) {
				if (improve) {
					currentSelector->setBestPlan(currentPlateau->getBestPlan());
				} else {
					currentSelector->setIterationsWithoutImproving(PLATEAU_START);
				}
				cancelPlateauSearch(improve);
			}
		}
	}
}
*/

Plan* PlannerDeadEnds::searchStep() {
	setCurrentSelector();
	Plan* base = currentSelector->poll();
	if (!expandBasePlan(base)) return nullptr;
	addSuccessors(base);
	//checkPlateau();
	currentSelectorA = !currentSelectorA;
	return base;
}
