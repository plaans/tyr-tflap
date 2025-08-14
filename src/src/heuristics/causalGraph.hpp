#ifndef CAUSAL_GRAPH_H
#define CAUSAL_GRAPH_H

#include "../sas/sasTask.hpp"

class StronglyConnectedComponent {
private:
	std::vector<TVariable> vars;

public:
	StronglyConnectedComponent(std::vector<bool>* successors, std::vector<bool>* predecessors);
	void checkVector(std::vector<bool>* v);
	inline unsigned int size() { return vars.size(); }
	inline TVariable getVar(unsigned int i) { return vars[i]; }
	void removeVar(TVariable v);
	std::string toString();
};

class CausalGraph {
private:
	SASTask* task;
	unsigned int numVariables;
	unsigned int** dependences;

	void checkDependences(SASAction* a);
	void addDependence(TVariable v, TVariable w, std::vector<TOrdering>* alreadyAdded, unsigned int weight);
	void pruneCausalGraph();
	void getSuccessorsByDFS(unsigned int v, std::vector<bool>* successors);
	void getPredecessorsByDFS(unsigned int v, std::vector<bool>* predecessors);
	void pruneComponent(StronglyConnectedComponent* c);
	void calculateWeights(StronglyConnectedComponent* c, std::vector<unsigned int>* weights);
	TVariable getLowestLevelVariable(StronglyConnectedComponent* c, std::vector<unsigned int>* weights);
	void removeArcs(TVariable v, StronglyConnectedComponent* c);
	void propagateDependences();
	void removeTransitiveOrderings();
	bool checkIndirectReachability(TVariable orig, TVariable current, TVariable dst, std::vector<bool> *visited);

public:
	CausalGraph(SASTask* task);
	~CausalGraph();
	inline bool checkDependence(TVariable v1, TVariable v2) { return dependences[v1][v2] > 0; }
};

#endif
