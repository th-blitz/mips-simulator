#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <bitset>

// Preetham Rakshith Prakash.

using namespace std;

int main (int argc, char** argv) {
	ifstream config;
	config.open(argv[1]);

	int m, w, h;
	config >> m;
	config >> h;
	config >> w;
 
	config.close();

	ofstream out;
	string out_file_name = string(argv[2]) + ".out";
	out.open(out_file_name.c_str());
	
	ifstream trace;
	trace.open(argv[2]);

	string line;
	uint32_t* BHT = (uint32_t*)malloc(sizeof(uint32_t) * (1 << h));
	int* PHT = (int*)malloc(sizeof(int) * (1 << m));
	for (uint32_t i = 0; i < (1 << h); i++) {
		BHT[i] = 0;
	}
	for (uint32_t i = 0; i < (1 << m); i++) {
		PHT[i] = 2;
	}

	// TODO: Implement a two-level branch predictor 
	while (!trace.eof()) {

		getline(trace, line);
		uint32_t space = line.find(' ');
		string pc_string = line.substr(0, space);
		string true_branch_string = line.substr(space + 1);
		int true_branch = -1;
		if (true_branch_string[0] == '1') {
			true_branch = 1;
		}
		uint32_t pc = stoul(pc_string, nullptr, 16);
		
		uint32_t h_index = (pc >> 2) & ((1 << h) - 1);
		uint32_t a = ((pc >> 2) & ((1 << (m - w)) - 1)) << w;
		uint32_t b = BHT[h_index] & ((1 << w) - 1);
		uint32_t pht_index = a + b;

		int our_prediction = (PHT[pht_index] < 2) ? 0 : 1;
		if (!(PHT[pht_index] == 0 && true_branch == -1) && !(PHT[pht_index] == 3 && true_branch == 1)) {
			PHT[pht_index] += true_branch;
		}
		BHT[h_index] = (BHT[h_index] << 1) + ((true_branch == 1) ? 1 : 0);
		
		out << our_prediction << endl; // predict not taken
	}

	free(BHT);
	free(PHT);

	trace.close();	
	out.close();
}

// Path: branchsimulator_skeleton_23.cpp
