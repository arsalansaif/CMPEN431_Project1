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
 *
 */
#define PSU_ID_SUM (919244886+994898078)

/*
 * Some global variables to track heuristic progress.
 * 
 * Feel free to create more global variables to track progress of your
 * heuristic.
 */
unsigned int currentlyExploringDim = 11;
bool currentDimDone = false;
bool isDSEComplete = false;
int Indexer = 0;
bool result = true;

/*
 * Given a half-baked configuration containing cache properties, generate
 * latency parameters in configuration string. You will need information about
 * how different cache paramters affect access latency.
 * 
 * Returns a string similar to "1 1 1"
 */
 //using std::to_string;
std::string generateCacheLatencyParams(string halfBackedConfig) {

	string latencySettings;

	//
	std::stringstream latency;
	
	

	// int dl1Association = extractConfigPararm(halfBackedConfig, 4);
	// int il1Association = extractConfigPararm(halfBackedConfig, 6);
	// int ul2Association = extractConfigPararm(halfBackedConfig, 9);

	// int il1block_size1 = extractConfigPararm(halfBackedConfig, 2);
	// int ul2block_size1 = extractConfigPararm(halfBackedConfig, 8);
	
	// int il1SetSize1 = extractConfigPararm(halfBackedConfig, 5);
	// int dl1SetSize1 = extractConfigPararm(halfBackedConfig, 3);
	// int ul2SetSize1 = extractConfigPararm(halfBackedConfig, 7);

	// int dl1Size = il1block_size1 * dl1SetSize1 * dl1Association;
	// int il1Size = il1block_size1 * il1SetSize1 * il1Association;
	// int ul2Size = ul2block_size1 * ul2SetSize1 * ul2Association;
	int dl1Size=getdl1size(halfBackedConfig);
	int il1Size=getil1size(halfBackedConfig);
	int ul2Size=getl2size(halfBackedConfig);
	

	// This is a dumb implementation.
	latencySettings = "1 1 1";
	//int latencyInt = 0;
	int dl1lat = 0;
	int il1lat = 0;
	int ul2lat = 0;
	//
	//YOUR CODE ENDS HERE
	//
	

	// latencySettings = "";
	// latencySettings += to_string(dl1lat);
	// latencySettings += " ";
	// latencySettings += to_string(il1lat);
	// latencySettings += " ";
	// latencySettings += to_string(ul2lat);
	//latency << dl1lat << " " << il1lat << " " << ul2lat;
	//latencySettings = latency;
	latency << log2(dl1Size)-11+extractConfigPararm(halfBackedConfig, 4) << " " << log2(il1Size)-11+extractConfigPararm(halfBackedConfig, 6) << " " << log2(ul2Size)-15+extractConfigPararm(halfBackedConfig, 9);
	latencySettings = latency.str();
	return latencySettings;
}

/*
 * Returns 1 if configuration is valid, else 0
 */
int validateConfiguration(std::string configuration) {

	// FIXME - YOUR CODE HERE
	// int il1block_size = extractConfigPararm(configuration, 2);
	// int ifq = extractConfigPararm(configuration, 0);
	// int ul2block_size = extractConfigPararm(configuration, 8);
	// int ul2Asso = extractConfigPararm(configuration, 9);
	// int il1SetSize = extractConfigPararm(configuration, 5);
	// int dl1SetSize = extractConfigPararm(configuration, 3);
	// int ul2SetSize = extractConfigPararm(configuration, 7);
	// int dl1Asso = extractConfigPararm(configuration, 4);
	// int il1Asso = extractConfigPararm(configuration, 6);

	// 

	int dl1blocksize = 8*(1 << extractConfigPararm(configuration,2));
    int il1blocksize = 8*(1 << extractConfigPararm(configuration,2));
	int ul2blocksize = 16 << extractConfigPararm(configuration,8);
	int ifq = 8*(1 << extractConfigPararm(configuration,0));
	int dl1Size=getdl1size(configuration);
	int il1Size=getil1size(configuration);
	int ul2Size=getl2size(configuration);

	if (il1blocksize < ifq)
	 	return 0;
	else if (ul2blocksize < 2 * il1blocksize || ul2blocksize > 128)
		return 0;
	else if (il1Size < 2048 || il1Size > 65536)
		return 0;
	else if (dl1Size < 2048 || dl1Size > 65536)
		return 0;
	else if (ul2Size < 32768 || ul2Size > 1024000)
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
	
	
	std::string startingConfig = GLOB_baseline;
	std::string nextconfiguration = currentconfiguration;
	// Continue if proposed configuration is invalid or has been seen/checked before.
	while (!validateConfiguration(nextconfiguration) ||
		GLOB_seen_configurations[nextconfiguration]) {
		// Check if DSE has been completed before and return current
		// configuration.
		

		std::stringstream ss;

		string bestConfig;
		if (optimizeforEXEC == 1)
			bestConfig = bestEXECconfiguration;

		if (optimizeforEDP == 1)
			bestConfig = bestEDPconfiguration;
		if(isDSEComplete) {
			return bestConfig;
		}
		// Fill in the dimensions already-scanned with the already-selected best
		// value.
		for (int dim = 0; dim < currentlyExploringDim; ++dim) {
			ss << extractConfigPararm(bestConfig, dim) << " ";
		}

		// Handling for currently exploring dimension. This is a very dumb
		// implementation.
		int nextValue = extractConfigPararm(nextconfiguration,
				currentlyExploringDim) + 1;

		if (result == true){
			result = false;
			nextValue = 0;
		}
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
		
		
		std::vector<int> ParamList;
		ParamList.push_back(11);
		ParamList.push_back(2);
		ParamList.push_back(3);
		ParamList.push_back(4);
		ParamList.push_back(5);
		ParamList.push_back(6);
		ParamList.push_back(7);
		ParamList.push_back(8);
		ParamList.push_back(9);
		ParamList.push_back(10);
		ParamList.push_back(12);
		ParamList.push_back(13);
		ParamList.push_back(14);
		ParamList.push_back(0);
		ParamList.push_back(1);

		if (currentDimDone) {
			Indexer++;
			currentlyExploringDim = ParamList[Indexer];
			currentDimDone = false;
			result = true;
		}

		// Signal that DSE is complete after this configuration.
		//if best config = starting config, done, otherwise starting config = best config, start exploring
		
		if (Indexer == (NUM_DIMS - NUM_DIMS_DEPENDENT))
			if (bestConfig == startingConfig){

				isDSEComplete = true;
			}
			else{
				startingConfig = bestConfig;
				Indexer = 0;
			}
	}
	return nextconfiguration;
}

