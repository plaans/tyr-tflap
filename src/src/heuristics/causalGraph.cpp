/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* Nov 2017                                             */
/********************************************************/
/* Causal graph                             			*/
/********************************************************/

#include <iostream>
#include "causalGraph.hpp"
using namespace std;

//#define DEBUG_CG_ON

/********************************************************/
/* CLASS: CausalGraph                                   */
/********************************************************/

CausalGraph::CausalGraph(SASTask* task) {
	this->task = task;
#ifdef DEBUG_CG_ON
	cout << task->toString() << endl;
#endif
	numVariables = task->variables.size();
	dependences = new unsigned int*[numVariables];
	for (unsigned int i = 0; i < numVariables; i++) {
		dependences[i] = new unsigned int[numVariables];
		for (unsigned int j = 0; j < numVariables; j++)
			dependences[i][j] = 0;
	}
	unsigned int numActions = task->actions.size();
	for (unsigned int i = 0; i < numActions; i++) {
		SASAction* a = &(task->actions[i]);
		checkDependences(a);
	}
#ifdef DEBUG_CG_ON
	cout << "ALL DEPENDENCES:" << endl;
	for (unsigned int i = 0; i < numVariables; i++) {
		for (unsigned int j = 0; j < numVariables; j++) {
			if (dependences[i][j] > 0)
				cout << task->variables[i].name << " --" << dependences[i][j] <<
					"--> " << task->variables[j].name << endl;
		}
	}
#endif
	pruneCausalGraph();
	//removeTransitiveOrderings();
#ifdef DEBUG_CG_ON
	for (unsigned int i = 0; i < numVariables; i++) {
		for (unsigned int j = 0; j < numVariables; j++) {
			if (dependences[i][j] > 0)
				cout << task->variables[i].name << " --" << dependences[i][j] <<
					"--> " << task->variables[j].name << endl;
		}
	}
#endif
}

CausalGraph::~CausalGraph() {
	if (dependences != nullptr) {
		for (unsigned int i = 0; i < numVariables; i++)
			delete[] dependences[i];
		delete[] dependences;
		dependences = nullptr;
	}
}

void CausalGraph::pruneCausalGraph() {
	vector<bool> inComponent(numVariables, false);
	vector<StronglyConnectedComponent> components;
	for (unsigned int i = 0; i < numVariables; i++) {
		if (!inComponent[i]) {
			vector<bool> successors(numVariables, false);
			getSuccessorsByDFS(i, &successors);
			vector<bool> predecessors(numVariables, false);
			getPredecessorsByDFS(i, &predecessors);
			components.emplace_back(&successors, &predecessors);
			components.back().checkVector(&inComponent);
		}
	}
	for (unsigned int i = 0; i < components.size(); i++) {
#ifdef DEBUG_CG_ON
		cout << components[i].toString() << endl;
#endif
		pruneComponent(&components[i]);
	}
}

void CausalGraph::getSuccessorsByDFS(unsigned int v, std::vector<bool>* successors) {
	(*successors)[v] = true;
	for (unsigned int i = 0; i < numVariables; i++) {
		if (dependences[v][i] && !(*successors)[i]) {
			getSuccessorsByDFS(i, successors);
		}
	}
}

void CausalGraph::getPredecessorsByDFS(unsigned int v, std::vector<bool>* predecessors) {
	(*predecessors)[v] = true;
	for (unsigned int i = 0; i < numVariables; i++) {
		if (dependences[i][v] && !(*predecessors)[i]) {
			getPredecessorsByDFS(i, predecessors);
		}
	}
}

void CausalGraph::checkDependences(SASAction* a) {
	std::vector<TOrdering> alreadyAdded;
#ifdef DEBUG_CG_ON
	cout << "Checking action: " << a->name << endl;
#endif
	for (unsigned int i = 0; i < a->startEff.size(); i++) {
		TVariable v = a->startEff[i].var;	// Modified variable
		for (unsigned int j = 0; j < a->startCond.size(); j++)
			addDependence(v, a->startCond[j].var, &alreadyAdded, 2);	// Transition condition
		for (unsigned int j = 0; j < a->startEff.size(); j++)
			addDependence(v, a->startEff[j].var, &alreadyAdded, 1);		// Co-occurring effects
	}
	for (unsigned int i = 0; i < a->endEff.size(); i++) {
		TVariable v = a->endEff[i].var;	// Modified variable
		for (unsigned int j = 0; j < a->startCond.size(); j++)
			addDependence(v, a->startCond[j].var, &alreadyAdded, 2);	// Transition condition
		for (unsigned int j = 0; j < a->overCond.size(); j++)
			addDependence(v, a->overCond[j].var, &alreadyAdded, 2);		// Transition condition
		for (unsigned int j = 0; j < a->endCond.size(); j++)
			addDependence(v, a->endCond[j].var, &alreadyAdded, 2);		// Transition condition
		for (unsigned int j = 0; j < a->startEff.size(); j++)
			addDependence(v, a->startEff[j].var, &alreadyAdded, 1);		// Co-occurring effects
		for (unsigned int j = 0; j < a->endEff.size(); j++)
			addDependence(v, a->endEff[j].var, &alreadyAdded, 1);		// Co-occurring effects
	}
}

// Variable v depends on w
void CausalGraph::addDependence(TVariable v, TVariable w, std::vector<TOrdering>* alreadyAdded, unsigned int weight) {
	if (v == w) return;
	TOrdering newArc = getOrdering(v, w);
	for (unsigned int i = 0; i < alreadyAdded->size(); i++) {
		if (alreadyAdded->at(i) == newArc) return;
	}
	alreadyAdded->push_back(newArc);
	dependences[v][w] += weight;
#ifdef DEBUG_CG_ON
	cout << "CG dependence: " << task->variables[v].name << " depends on " << task->variables[w].name
			<< ", weight " << dependences[v][w] << endl;
#endif
}

void CausalGraph::pruneComponent(StronglyConnectedComponent* c) {
	while (c->size() > 1) {
		vector<unsigned int> weights(numVariables, 0);
		calculateWeights(c, &weights);
		TVariable v = getLowestLevelVariable(c, &weights);
		removeArcs(v, c);
		c->removeVar(v);
	}
}

void CausalGraph::removeArcs(TVariable v, StronglyConnectedComponent* c) {
	for (unsigned int i = 0; i < c->size(); i++) {
		dependences[c->getVar(i)][v] = 0;
	}
}

TVariable CausalGraph::getLowestLevelVariable(StronglyConnectedComponent* c, std::vector<unsigned int>* weights) {
	TVariable v = c->getVar(0);
	unsigned int weight = (*weights)[v];
	for (unsigned int i = 1; i < c->size(); i++) {
		TVariable w = c->getVar(i);
		if ((*weights)[w] < weight) {
			v = w;
			weight = (*weights)[w];
		}
	}
	return v;
}

void CausalGraph::calculateWeights(StronglyConnectedComponent* c, std::vector<unsigned int>* weights) {
	unsigned int size = c->size();
	for (unsigned int i = 0; i < size; i++) {
		TVariable v = c->getVar(i);
		unsigned int w = 0;
		for (unsigned int j = 0; j < size; j++) {
			w += dependences[c->getVar(j)][v];
		}
		(*weights)[v] = w;
#ifdef DEBUG_CG_ON
		cout << "* Weight of var. " << v << " is " << w << endl;
#endif
	}
}

void CausalGraph::propagateDependences() {
	for (unsigned int i = 0; i < numVariables; i++) {
		std::vector<bool> successors(numVariables, false);
		getSuccessorsByDFS(i, &successors);
#ifdef DEBUG_CG_ON
		cout << "Sucessors of variable: " << i << endl;
#endif
		for (unsigned int j = 0; j < numVariables; j++) {
			if (successors[j]) {
#ifdef DEBUG_CG_ON
				cout << "* " << j << endl;
#endif
				dependences[i][j] = 1;
			}
		}
	}
}

void CausalGraph::removeTransitiveOrderings() {
	for (unsigned int i = 0; i < numVariables; i++) {
		for (unsigned int j = 0; j < numVariables; j++) {
			if (dependences[i][j]) {
				std::vector<bool> visited(numVariables, false);
				if (checkIndirectReachability(i, i, j, &visited))
					dependences[i][j] = false;
			}
		}
	}
}

bool CausalGraph::checkIndirectReachability(TVariable orig, TVariable current, TVariable dst,
		std::vector<bool> *visited) {
	(*visited)[current] = true;
	for (unsigned int adj = 0; adj < numVariables; adj++) {
		if (dependences[current][adj] && !visited->at(adj)) {
			if (adj == dst && current != orig) return true;
			if (adj != dst && checkIndirectReachability(orig, adj, dst, visited)) return true;
		}
	}
	return false;
}


/********************************************************/
/* CLASS: StronglyConnectedComponent                    */
/********************************************************/

StronglyConnectedComponent::StronglyConnectedComponent(std::vector<bool>* successors, std::vector<bool>* predecessors) {
	for (unsigned int i = 0; i < successors->size();i++) {
		if (successors->at(i) && predecessors->at(i))
			vars.push_back(i);
	}
}

void StronglyConnectedComponent::checkVector(std::vector<bool>* v) {
	for (unsigned int i = 0; i < vars.size(); i++) {
		(*v)[vars[i]] = true;
	}
}

void StronglyConnectedComponent::removeVar(TVariable v) {
	for (unsigned int i = 0; i < vars.size(); i++) {
		if (vars[i] == v) {
			vars.erase(vars.begin() + i);
			break;
		}
	}
}

std::string StronglyConnectedComponent::toString() {
	std::string s = "[" + std::to_string(vars[0]);
	for (unsigned int i = 1; i < vars.size(); i++)
		s += "," + std::to_string(vars[i]);
	return s + "]";
}

