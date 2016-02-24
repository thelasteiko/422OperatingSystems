/*
 * Siimulation.c
 *
 *  Created on: Feb 23, 2016
 *      Author: Author: Melinda Robertson, Chetayana, Jason Hall, Shewangizaw Gebremariam
 */


int main (void) {
	char experimentOne[10] = "deadlock";
	char experimentTwo[10] = "non deadlock";
	runSimulator(experimentOne);
	runSimulator(experimentTwo);
	return 0;
}

/**
 * Simulates the deadlock run
 */
void doSimulationDeadLock(){

}

/**
 * Simulates the non dead lock run
 */
void doSimulationNonDeadLock(){

}

/**
 * Runs the simulation
 * Limits the number of the simulation
 */
runSimulator(char experiment[]) {
	//Simulation limit
	int simulationLimit = 10;
	//Simulation counter
	int counter = 0;

	if (strcmp(experiment, "deadlock") == 0) {
		for (counter = 0; counter < simulationLimit; counter++) {
			doSimulationDeadLock();
		}
	} else {
		for (counter = 0; counter < simulationLimit; counter++) {
			doSimulationNonDeadLock();
		}

	}

}

