CC = g++
# Final version: remove -g and replace -O0 by -O3
CFLAGS = -c -Wall -std=c++11 -O3
LFLAGS = -Wall -std=c++11 -O3
OBJS = tflap.o parser.o syntaxAnalyzer.o parsedTask.o preprocess.o preprocessedTask.o grounder.o groundedTask.o sasTranslator.o mutexGraph.o sasTask.o state.o plan.o linearizer.o planner.o selector.o evaluator.o successors.o hFF.o landmarks.o hLand.o temporalRPG.o costRPG.o DTG.o causalGraph.o memoization.o plateau.o plannerConcurrent.o plannerDeadEnds.o plannerReversible.o plannerSetting.o

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o tflap

parser:	tflap.o parser.o syntaxAnalyzer.o parsedTask.o
	$(CC) $(LFLAGS) $(OBJS) -o tflap

preprocess: preprocess.o preprocessedTask.o
	$(CC) $(LFLAGS) $(OBJS) -o tflap

grounder: grounder.o groundedTask.o
	$(CC) $(LFLAGS) $(OBJS) -o tflap
	
sas: mutexGraph.o sasTranslator.o sasTask.o
	$(CC) $(LFLAGS) $(OBJS) -o tflap

heuristics: state.o hFF.o landmarks.o hLand.o evaluator.o temporalRPG.o costRPG.o DTG.o causalGraph.o
	$(CC) $(LFLAGS) $(OBJS) -o tflap

planner: plan.o state.o planner.o selector.o successors.o linearizer.o memoization.o plateau.o plannerConcurrent.o plannerDeadEnds.o plannerReversible.o plannerSetting.o
	$(CC) $(LFLAGS) $(OBJS) -o tflap
	
tflap.o:
	$(CC) $(CFLAGS) tflap.cpp

parser.o:
	$(CC) $(CFLAGS) parser/parser.cpp

syntaxAnalyzer.o:
	$(CC) $(CFLAGS) parser/syntaxAnalyzer.cpp

parsedTask.o:
	$(CC) $(CFLAGS) parser/parsedTask.cpp
	
preprocess.o:
	$(CC) $(CFLAGS) preprocess/preprocess.cpp
	
preprocessedTask.o:
	$(CC) $(CFLAGS) preprocess/preprocessedTask.cpp

grounder.o:
	$(CC) $(CFLAGS) grounder/grounder.cpp

groundedTask.o:
	$(CC) $(CFLAGS) grounder/groundedTask.cpp

sasTranslator.o:
	$(CC) $(CFLAGS) sas/sasTranslator.cpp

mutexGraph.o:
	$(CC) $(CFLAGS) sas/mutexGraph.cpp

sasTask.o:
	$(CC) $(CFLAGS) sas/sasTask.cpp

planner.o:
	$(CC) $(CFLAGS) planner/planner.cpp

plan.o:
	$(CC) $(CFLAGS) planner/plan.cpp
	
linearizer.o:
	$(CC) $(CFLAGS) planner/linearizer.cpp

selector.o:
	$(CC) $(CFLAGS) planner/selector.cpp	

successors.o:
	$(CC) $(CFLAGS) planner/successors.cpp

memoization.o:
	$(CC) $(CFLAGS) planner/memoization.cpp

plateau.o:
	$(CC) $(CFLAGS) planner/plateau.cpp

state.o:
	$(CC) $(CFLAGS) heuristics/state.cpp

evaluator.o:
	$(CC) $(CFLAGS) heuristics/evaluator.cpp

hFF.o:
	$(CC) $(CFLAGS) heuristics/hFF.cpp

landmarks.o:
	$(CC) $(CFLAGS) heuristics/landmarks.cpp

hLand.o:
	$(CC) $(CFLAGS) heuristics/hLand.cpp

temporalRPG.o:
	$(CC) $(CFLAGS) heuristics/temporalRPG.cpp

costRPG.o:
	$(CC) $(CFLAGS) heuristics/costRPG.cpp

DTG.o:
	$(CC) $(CFLAGS) heuristics/DTG.cpp

causalGraph.o:
	$(CC) $(CFLAGS) heuristics/causalGraph.cpp

plannerConcurrent.o: 
	$(CC) $(CFLAGS) planner/plannerConcurrent.cpp

plannerDeadEnds.o:
	$(CC) $(CFLAGS) planner/plannerDeadEnds.cpp

plannerReversible.o: 
	$(CC) $(CFLAGS) planner/plannerReversible.cpp

plannerSetting.o:
	$(CC) $(CFLAGS) planner/plannerSetting.cpp

clean:
	rm -f *.o
	rm -f tflap

cleanparser:
	rm parser.o
	rm syntaxAnalyzer.o
	rm parsedTask.o

cleanpreprocess:
	rm preprocess.o
	rm preprocessedTask.o
	
cleangrounder:
	rm grounder.o
	rm groundedTask.o

cleansas:
	rm sasTranslator.o
	rm mutexGraph.o
	rm sasTask.o
	
cleanplanner:
	rm plan.o
	rm planner.o
	rm selector.o
	rm successors.o
	rm linearizer.o
	rm memoization.o
	rm plateau.o
	rm plannerConcurrent.o 
	rm plannerDeadEnds.o 
	rm plannerReversible.o 
	rm plannerSetting.o
	
cleanheuristics:
	rm state.o
	rm hFF.o
	rm hLand.o
	rm landmarks.o
	rm temporalRPG.o
	rm costRPG.o
	rm DTG.o
	rm evaluator.o
	rm causalGraph.o
