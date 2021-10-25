#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <math.h>
#include <fcntl.h>
#include <vector>
#include <iterator>

#include "431project.h"

using namespace std;

/*
 * Enter your PSU IDs here to select the appropriate scanning order.
 */
#define PSU_ID_SUM (994898078+919244886)


/*
 * Some global variables to track heuristic progress.
 * 
 * Feel free to create more global variables to track progress of your
 * heuristic.
 */
unsigned int currentlyExploringDim = 0;
bool currentDimDone = false;
bool isDSEComplete = false;

/*
 * Given a half-baked configuration containing cache properties, generate
 * latency parameters in configuration string. You will need information about
 * how different cache paramters affect access latency.
 * 
 * Returns a string similar to "1 1 1"
 */
std::string generateCacheLatencyParams(string halfBackedConfig) {

	string latencySettings;

	//
	std::stringstream latency;
	int dl1Size = getdl1cachesize(halfBackedConfig);
	int il1Size = getil1cachesize(halfBackedConfig);
	int ul2Size = getl2cachesize(halfBackedConfig);

	int dl1Association = extractConfigPararm( configuration, 4);
	int il1Association = extractConfigPararm( configuration, 6);
	int ul2Association = extractConfigPararm( configuration, 9);
	// This is a dumb implementation.
	latencySettings = "1 1 1";
	//int latencyInt = 0;
	int dl1lat = 0;
	int il1lat = 0;
	int ul2lat = 0;
	//
	//YOUR CODE ENDS HERE
	//
	if (dl1Size == 2048)
		dl1lat = 1;
	else if (dl1Size == 4096)
		dl1lat = 2;
	else if (dl1Size == 8192)
		dl1lat = 3;
	else if (dl1Size == 16384)
		dl1lat = 4;
	else if (dl1Size == 32768)
		dl1lat = 5;
	else if (dl1Size == 65536)
		dl1lat = 6;

	if (dl1Association == 2)
		dl1lat = dl1lat + 1;
	else if (dl1Association == 4)
		dl1lat = dl1lat + 2;
	else if (dl1Association == 8)
		dl1lat = dl1lat + 3;


	if (il1Size == 2048)
		il1lat = 1;
	else if (il1Size == 4096)
		il1lat = 2;
	else if (il1Size == 8192)
		il1lat = 3;
	else if (il1Size == 16384)
		il1lat = 4;
	else if (il1Size == 32768)
		il1lat = 5;
	else if (il1Size == 65536)
		il1lat = 6;
	
	if (il1Association == 2)
		il1lat = il1lat + 1;
	else if (il1Association == 4)
		il1lat = il1lat + 2;
	else if (il1Association == 8)
		il1lat = il1lat + 3;


	if (ul2Size == 32768)
		ul2lat = 5;
	else if (ul2Size == 65536)
		ul2lat = 6;
	else if (ul2Size == 131072)
		ul2lat = 7;
	else if (ul2Size == 262144)
		ul2lat = 8;
	else if (ul2Size == 524288)
		ul2lat = 9;
	else if (ul2Size == 1048576)
		ul2lat = 10;
	
	if (ul2Association == 2)
		ul2lat = ul2lat + 1;
	else if (ul2Association == 4)
		ul2lat = ul2lat + 2;
	else if (ul2Association == 8)
		ul2lat = ul2lat + 3;
	else if (ul2Association == 16)
		ul2lat = ul2lat + 4;

	latencySettings << dl1lat << " " << il1lat << " " << ul2lat

	latency << log(dllsize)
	return latencySettings;
}

/*
 * Returns 1 if configuration is valid, else 0
 */
int validateConfiguration(std::string configuration) {

	// FIXME - YOUR CODE HERE
	int il1block_size = extractConfigPararm(configuration, 2);
	int ifq = extractConfigPararm(configuration, 0);
	int ul2block_size = extractConfigPararm(configuration, 8);
	int ul2Asso = extractConfigPararm(configuration, 9);
	int il1SetSize = extractConfigPararm(configuration, 5);
	int dl1SetSize = extractConfigPararm(configuration, 3);
	int ul2SetSize = extractConfigPararm(configuration, 7);
	int dl1Asso = extractConfigPararm(configuration, 4);
	int il1Asso = extractConfigPararm(configuration, 6);

	if (il1block_size < ifq)
		return 0;
	else if (ul2block_size < 2 * il1block_size || ul2block_size > 128)
		return 0;
	else if (il1block_size * il1SetSize * il1Asso < 2048 || il1block_size * il1SetSize * il1Asso > 65536)
		return 0;
	else if (il1block_size * dl1SetSize * dl1Asso < 2048 || il1block_size * dl1SetSize * dl1Asso > 65536)
		return 0;
	else if (ul2block_size * ul2SetSize  * ul2Asso < 32768 || ul2block_size * ul2SetSize  * ul2Asso > 1024000)
		return 0;
	// The below is a necessary, but insufficient condition for validating a
	// configuration.
	return isNumDimConfiguration(configuration);
}

/*
 * Given the current best known configuration, the current configuration,
 * and the globally visible map of all previously investigated configurations,
 * suggest a previously unexplored design point. You will only be allowed to
 * investigate 1000 design points in a particular run, so choose wisely.
 *
 * In the current implementation, we start from the leftmost dimension and
 * explore all possible options for this dimension and then go to the next
 * dimension until the rightmost dimension.
 */
std::string generateNextConfigurationProposal(std::string currentconfiguration,
		std::string bestEXECconfiguration, std::string bestEDPconfiguration,
		int optimizeforEXEC, int optimizeforEDP) {

	//
	// Some interesting variables in 431project.h include:
	//
	// 1. GLOB_dimensioncardinality
	// 2. GLOB_baseline
	// 3. NUM_DIMS
	// 4. NUM_DIMS_DEPENDENT
	// 5. GLOB_seen_configurations

	std::string nextconfiguration = currentconfiguration;
	// Continue if proposed configuration is invalid or has been seen/checked before.
	while (!validateConfiguration(nextconfiguration) ||
		GLOB_seen_configurations[nextconfiguration]) {

		// Check if DSE has been completed before and return current
		// configuration.
		if(isDSEComplete) {
			return bestConfig;
		}

		std::stringstream ss;

		string bestConfig;
		if (optimizeforEXEC == 1)
			bestConfig = bestEXECconfiguration;

		if (optimizeforEDP == 1)
			bestConfig = bestEDPconfiguration;

		// Fill in the dimensions already-scanned with the already-selected best
		// value.
		for (int dim = 0; dim < currentlyExploringDim; ++dim) {
			ss << extractConfigPararm(bestConfig, dim) << " ";
		}

		// Handling for currently exploring dimension. This is a very dumb
		// implementation.
		int nextValue = extractConfigPararm(nextconfiguration,
				currentlyExploringDim) + 1;

		if (nextValue >= GLOB_dimensioncardinality[currentlyExploringDim]) {
			nextValue = GLOB_dimensioncardinality[currentlyExploringDim] - 1;
			currentDimDone = true;
		}

		ss << nextValue << " ";

		// Fill in remaining independent params with 0.
		for (int dim = (currentlyExploringDim + 1);
				dim < (NUM_DIMS - NUM_DIMS_DEPENDENT); ++dim) {
			ss << extractConfigPararm(bestConfig, dim) << " ";
		}

		//
		// Last NUM_DIMS_DEPENDENT3 configuration parameters are not independent.
		// They depend on one or more parameters already set. Determine the
		// remaining parameters based on already decided independent ones.
		//
		string configSoFar = ss.str();

		// Populate this object using corresponding parameters from config.
		ss << generateCacheLatencyParams(configSoFar);

		// Configuration is ready now.
		nextconfiguration = ss.str();

		// Make sure we start exploring next dimension in next iteration.
		// update
		if (currentDimDone) {
			currentlyExploringDim++;
			currentDimDone = false;
		}

		// Signal that DSE is complete after this configuration.
		//if best config = starting config, done, otherwise starting config = best config, start exploring
		if (currentlyExploringDim == (NUM_DIMS - NUM_DIMS_DEPENDENT))
			isDSEComplete = true;
	}
	return nextconfiguration;
}
