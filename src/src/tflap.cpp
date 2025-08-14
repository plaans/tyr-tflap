/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* May 2015                                             */
/********************************************************/
/* Main method: parses the command-line arguments and   */
/* launches the planner.                                */
/********************************************************/

#include <iostream>
#include <time.h>
#include <string.h>
#include "parser/parser.hpp"
#include "preprocess/preprocess.hpp"
#include "grounder/grounder.hpp"
#include "sas/sasTranslator.hpp"
#include "planner/plan.hpp"
#include "planner/plannerSetting.hpp"
using namespace std;

#define _TRACE_OFF_
//#define _TIME_ON_
#define toSeconds(t) (float) (((int) (1000 * (clock() - t)/(float) CLOCKS_PER_SEC))/1000.0)
#define TIMEOUT 1700

clock_t startTime;

struct PlannerParameters
{
    float total_time;
    char *domainFileName;
    char *problemFileName;
    char *outputFileName;
    bool generateGroundedDomain;
    bool keepStaticData;
    bool noSAS;
    bool generateMutexFile;
    bool generateTrace;
    PlannerParameters() : total_time(0), domainFileName(nullptr),
           problemFileName(nullptr), outputFileName(nullptr), generateGroundedDomain(false), 
           keepStaticData(false), noSAS(false), generateMutexFile(false),
		   generateTrace(false) {}
};

// Parses the domain and problem files
ParsedTask* parseStage(PlannerParameters *parameters) {
    clock_t t = clock();
    Parser parser;
    ParsedTask* parsedTask = parser.parseDomain(parameters->domainFileName);
    parser.parseProblem(parameters->problemFileName);
    #ifdef _TRACE_ON_
        cout << parsedTask->toString() << endl;
    #endif
    float time = toSeconds(t);
    parameters->total_time += time;
    #ifdef _TIME_ON_
        cout << ";Parsing time: " << time << endl;
    #endif
    return parsedTask;
}

// Preprocesses the parsed task
PreprocessedTask* preprocessStage(ParsedTask* parsedTask, PlannerParameters *parameters) {
    clock_t t = clock();
    Preprocess preprocess;
    PreprocessedTask* prepTask = preprocess.preprocessTask(parsedTask);
    float time = toSeconds(t);
    parameters->total_time += time;
    #ifdef _TRACE_ON_
       cout << prepTask->toString() << endl;
    #endif
    #ifdef _TIME_ON_
        cout << ";Preprocessing time: " << time << endl;
    #endif
    return prepTask;
}

// Grounder stage of the preprocessed task
GroundedTask* groundingStage(PreprocessedTask* prepTask, PlannerParameters *parameters) {
    clock_t t = clock();
    Grounder grounder;
    GroundedTask* gTask = grounder.groundTask(prepTask, parameters->keepStaticData);
    float time = toSeconds(t);
    parameters->total_time += time;
#ifdef _TRACE_ON_
     cout << gTask->toString() << endl;
#endif
    #ifdef _TIME_ON_
        cout << ";Grounding time: " << time << endl;
    #endif
    if (parameters->generateGroundedDomain) {
        cout << ";" << gTask->actions.size() << " grounded actions" << endl;
        gTask->writePDDLDomain();
        gTask->writePDDLProblem();
    }
    return gTask;
}

// SAS translation stage
SASTask* sasTranslationStage(GroundedTask* gTask, PlannerParameters *parameters) {
    clock_t t = clock();
    SASTranslator translator;
    SASTask* sasTask = translator.translate(gTask, parameters->noSAS, parameters->generateMutexFile, 
		parameters->keepStaticData);
    float time = toSeconds(t);
    parameters->total_time += time;
    #ifdef _TIME_ON_
        cout << ";SAS translation time: " << time << endl;
    #endif
	#ifdef _TRACE_ON_
        cout << sasTask->toString() << endl;
	#endif
	return sasTask;
}

// Sequential calls to the preprocess stages
SASTask* doPreprocess(PlannerParameters *parameters) {
	parameters->total_time = 0;
	SASTask* sTask = nullptr;
    ParsedTask* parsedTask = parseStage(parameters);
    if (parsedTask != nullptr) {
    	PreprocessedTask* prepTask = preprocessStage(parsedTask, parameters);
        if (prepTask != nullptr) {
        	GroundedTask* gTask = groundingStage(prepTask, parameters);
            if (gTask != nullptr) {
            	sTask = sasTranslationStage(gTask, parameters);
                delete gTask;       
            }
            delete prepTask;
        }
        delete parsedTask;
    }
    return sTask;
}

void printPlan(Plan* solution, int numSol, PlannerParameters *parameters, PlannerSetting* planner, clock_t t) {
	//cout << "SOLUTION: " << numSol << endl;
	std::ofstream solFile;
	char fname[256];
	snprintf(fname, sizeof(fname), "%s.%d", parameters->outputFileName, numSol);
	solFile.open(fname);
	float time = toSeconds(t);
	std::string s = planner->planToPDDL(solution);
	//cout << endl << s;
	solFile << s;
	//cout << ";Planning time: " << time << endl;
	solFile << ";Planning time: " << time << endl;
	time += parameters->total_time;
	//cout << ";Total time: " << time << endl;
	solFile << ";Total time: " << time << endl;
	//cout << ";" << planner->getExpandedNodes() << " expanded nodes" << endl;
	solFile << ";" << planner->getExpandedNodes() << " expanded nodes" << endl;
	solFile.close();
}

// Sequential calls to the main planning stages
void startPlanning(PlannerParameters *parameters) {
	clock_t t = clock();
        SASTask* sTask = doPreprocess(parameters);
	if (sTask == nullptr) return;
	PlannerSetting planner(sTask, parameters->generateTrace, TIMEOUT - toSeconds(startTime));
	Plan* solution = planner.plan();
	int numSol = 0;
	uint16_t bestG;
	float bestGC;
	bool first = true;
	do {
		if (solution != nullptr) {
			printPlan(solution, ++numSol, parameters, &planner, t);
			bestG = solution->g;
			bestGC = solution->gc;
		} else {
			bestG = MAX_UINT16;
			bestGC = FLOAT_INFINITY;
		}
		solution = planner.improveSolution(bestG, bestGC, first);
		first = false;
	} while (solution != nullptr);
	delete sTask;
}

// Prints the command-line arguments of the planner
void printUsage() {
     cout << "Usage: tflap <domain_file> <problem_file> <output_file> [-ground] [-static] [-mutex] [-trace]" << endl;
     cout << " -ground: generates the GroundedDomain.pddl and GroundedProblem.pddl files." << endl;
     cout << " -static: keeps the static data in the planning task." << endl;
     cout << " -nsas: does not make translation to SAS (finite-domain variables)." << endl;
     cout << " -mutex: generates the mutex.txt file with the list of static mutex facts." << endl; 
	 cout << " -trace: generates the trace.txt file with the search tree." << endl;
}

// Compare two strings
bool compareStr(char* s1, const char* s2) {
     unsigned int l = strlen(s1);
     if (l != strlen(s2)) return false;
     for (unsigned int i = 0; i < l; i++)
         if (tolower(s1[i]) != s2[i]) return false;
     return true;
}

// Main method
int main(int argc, char* argv[]) {
    startTime = clock();
    if (argc < 4) {
       printUsage();
    } else {
       PlannerParameters parameters;
       int param = 1;
       while (param < argc) {
         if (argv[param][0] != '-') {
            if (parameters.domainFileName == nullptr) parameters.domainFileName = argv[param];
            else if (parameters.problemFileName == nullptr) parameters.problemFileName = argv[param];
            else if (parameters.outputFileName == nullptr) parameters.outputFileName = argv[param];
	    else { parameters.domainFileName = nullptr; break; }
         } else {
            if (compareStr(argv[param], "-ground")) parameters.generateGroundedDomain = true;
            else if (compareStr(argv[param], "-static")) parameters.keepStaticData = true;
            else if (compareStr(argv[param], "-nsas")) parameters.noSAS = true;
            else if (compareStr(argv[param], "-mutex")) parameters.generateMutexFile = true;
	    else if (compareStr(argv[param], "-trace")) parameters.generateTrace = true;
	    else { parameters.domainFileName = nullptr; break; }
         }
         param++;
       }
       if (parameters.domainFileName == nullptr || parameters.problemFileName == nullptr) printUsage();
       else startPlanning(&parameters);
    }
    return 0;
}
