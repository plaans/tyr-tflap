#ifndef HLAND_H
#define HLAND_H

#include <vector>
#include <unordered_map>
#include "../sas/sasTask.hpp"
#include "state.hpp"
#include "landmarks.hpp"

class LandmarkCheck {			// Landmarks reachability for heuristic purposes
private:
	std::vector<TVariable> vars;
	std::vector<TValue> values;
	std::vector<LandmarkCheck*> prev;
	std::vector<LandmarkCheck*> next;
	bool checked;
	bool single;

public:
	LandmarkCheck(LandmarkNode* n);
	void addNext(LandmarkCheck* n);
	void addPrev(LandmarkCheck* n);
	void removeSuccessor(LandmarkCheck* n);
	void removePredecessor(LandmarkCheck* n);
	bool isGoal(SASTask* task);
	bool goOn(TState* s);
	bool isInitialState(TState* state);
	inline void uncheck() { checked = false; }
	inline void check() { checked = true; }
	inline bool isChecked() { return checked; }
	inline bool isSingle() { return single; }
	inline unsigned int numPrev() { return prev.size(); }
	inline unsigned int numNext() { return next.size(); }
	inline TVariable getVar() { return vars[0]; }
	inline TValue getValue() { return values[0]; }
	inline LandmarkCheck* getNext(unsigned int i) { return next[i]; }
	inline LandmarkCheck* getPrev(unsigned int i) { return prev[i]; }
	std::string toString(SASTask* task, bool showNext);
};

class LandmarkHeuristic {		// Landmarks heuristic
private:
	SASTask* task;
	std::vector<LandmarkCheck*> nodes;
	std::vector<LandmarkCheck*> rootNodes;

	void addRootNode(LandmarkCheck* n, TState* state, std::vector<LandmarkCheck*>* toDelete);
	bool hasRootPredecessor(LandmarkCheck* n);

public:
	LandmarkHeuristic(); 
	~LandmarkHeuristic();
	void initialize(SASTask* task, std::vector<SASAction*>* tilActions);
	void initialize(TState* state, SASTask* task, std::vector<SASAction*>* tilActions);
	void uncheckNodes();
	uint16_t evaluate();
	void copyRootNodes(std::vector<LandmarkCheck*>* v);
	std::string toString(SASTask* task);
	inline unsigned int getNumNodes() { return nodes.size(); }
	inline uint16_t countUncheckedNodes() {
		uint16_t n = 0;
		for (unsigned int i = 0; i < nodes.size(); i++)
			if (!nodes[i]->isChecked()) n++;
		return n;
	}
	int getNumInformativeNodes();
};

#endif
