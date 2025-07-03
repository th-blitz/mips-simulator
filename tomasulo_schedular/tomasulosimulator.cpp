
// (c) Dec 2023 Preetham Rakshith Prakash.


#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <limits>

using std::cout;
using std::endl;
using std::string;
using std::vector;

string inputtracename = "trace.txt";
// remove the ".txt" and add ".out.txt" to the end as output name
string outputtracename = inputtracename.substr(0, inputtracename.length() - 4) + ".out.txt";
string hardwareconfigname = "config.txt";

enum Operation
{
	ADD,
	SUB,
	MULT,
	DIV,
	LOAD,
	STORE
};
// The execute cycle of each operation: ADD, SUB, MULT, DIV, LOAD, STORE
const int OperationCycle[6] = {2, 2, 10, 40, 2, 2};

struct HardwareConfig
{
	int LoadRSsize;	 // number of load reservation stations
	int StoreRSsize; // number of store reservation stations
	int AddRSsize;	 // number of add reservation stations
	int MultRSsize;	 // number of multiply reservation stations
	int FRegSize;	 // number of fp registers
};

// We use the following structure to record the time of each instruction
struct InstructionStatus
{
	int cycleIssued;
	int cycleExecuted; // execution completed
	int cycleWriteResult;
};

// Register Result Status structure
struct RegisterResultStatus
{
	string ReservationStationName;
	bool dataReady;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
struct Instruction;

struct CommonDataBus {
	std::unordered_set<string> set;
};

class RegisterResultStatuses
{
public:
	// ...
	RegisterResultStatuses(uint32_t num_of_registers) {
		for (uint32_t i = 0; i < num_of_registers; i++) {
			struct RegisterResultStatus reg_status;
			reg_status.dataReady = false;
			reg_status.ReservationStationName = "";
			_registers.push_back(reg_status);
		}
	}

	string get_register_value(uint32_t reg_num) {
		string reg_value = _registers[reg_num].ReservationStationName;
		return reg_value;
	}

	bool is_data_ready(uint32_t reg_num) {
		if (reg_num >= _registers.size()) {
			cout << "ERROR AT is_data_ready(uint32_t reg_num); reg_num exceeds registers size" << endl;
		}
		return _registers[reg_num].dataReady;
	}

	void write_registers(string bus) {
		for (int i = 0; i < _registers.size(); i++) {
			if ((_registers[i].dataReady == false) && (bus == _registers[i].ReservationStationName) && (bus != "")) {
				_registers[i].dataReady = true;
			}
		}
	}

	void add_output(string out, uint32_t register_num) {
		_registers[register_num].ReservationStationName = out;
		_registers[register_num].dataReady = false;
	}

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/
	/*
	Print all register result status. It is called by PrintRegisterResultStatus4Grade() for grading.
	If you don't want to write such a class, then make sure you call the same function and print the register
	result status in the same format.
	*/
	string _printRegisterResultStatus() const
	{
		std::ostringstream result;
		for (int idx = 0; idx < _registers.size(); idx++)
		{
			result << "F" + std::to_string(idx) << ": ";
			result << _registers[idx].ReservationStationName << ", ";
			result << "dataRdy: " << (_registers[idx].dataReady ? "Y" : "N") << ", ";
			result << "\n";
		}
		return result.str();
	}
/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
private:
	vector<RegisterResultStatus> _registers;
};

// Define your Reservation Station structure

struct pointer_to_station {
	enum Operation type;
	uint32_t index;
};

struct Stations {
	uint32_t size;	
	bool* busy;
	bool* vj;
	bool* vk;
	struct pointer_to_station* qj;
	struct pointer_to_station* qk;
	uint32_t* operations;	
	uint32_t* operation_count;
	uint32_t* instruction_id;
};

struct Stations* get_stations(struct HardwareConfig config) {
	struct Stations* stations = (struct Stations*)malloc(sizeof(struct Stations) * 4);
	stations[0].size = config.LoadRSsize;
	stations[1].size = config.StoreRSsize;
	stations[2].size = config.AddRSsize;
	stations[3].size = config.MultRSsize;
	for (uint32_t i = 0; i < 4; i++ ) {
		uint32_t size = stations[i].size;
		stations[i].busy = (bool*)malloc(sizeof(bool) * size);
		stations[i].vj = (bool*)malloc(sizeof(bool) * size);
		stations[i].vk = (bool*)malloc(sizeof(bool) * size);
		stations[i].qj = (struct pointer_to_station*)malloc(sizeof(struct pointer_to_station) * size);
		stations[i].qk = (struct pointer_to_station*)malloc(sizeof(struct pointer_to_station) * size);
		stations[i].operations = (uint32_t*)malloc(sizeof(uint32_t) * size);
		stations[i].operation_count = (uint32_t*)malloc(sizeof(uint32_t*) * size);
		stations[i].instruction_id = (uint32_t*)malloc(sizeof(uint32_t) * size);
		for (uint32_t j = 0; j < size; j++) {
			stations[i].busy[j] = false;
			stations[i].vj[j] = false;
			stations[i].vk[j] = false;
			stations[i].operations[j] = 0;
			stations[i].operation_count[j] = 0;
			stations[i].instruction_id[j] = 0;
		}	
	}
	return stations;
}

void clear_stations(struct Stations* stations) {
	for (uint32_t i = 0; i < 4; i++) {
		free(stations[i].busy);
		free(stations[i].vj);
		free(stations[i].vk);
		free(stations[i].qj);
		free(stations[i].qk);
		free(stations[i].operations);
		free(stations[i].operation_count);
		free(stations[i].instruction_id);
	}
	free(stations);
}

vector<string> parse_instruction(string line) {
	vector<string> trace;
	uint32_t i = 0, j = 0;
	uint32_t parsed_line_size = 0;
	for (j = 0; j < line.size(); j++) {
		if (line[j] == ' ') {
			trace.push_back(line.substr(i, j - i));
			i = j + 1;
			parsed_line_size += 1;
		}
	}
	
	trace.push_back(line.substr(i, j - i));
	
	return trace;
}

enum Operation get_operation(vector<string> parsed_line) {
	
	string op = parsed_line[0];
	enum Operation opt;

	if (op == "LOAD") {
		opt = LOAD;
	} else if (op == "STORE") {
		opt = STORE;
	} else if (op == "ADD") {
		opt = ADD;
	} else if (op == "SUB") {
		opt = SUB;
	} else if (op == "MULT") {
		opt = MULT;
	} else if (op == "DIV") {
		opt = DIV;
	} else {
		cout << "ERROR IN PARSING OPERATION ENUM AT get_operation(vec<str> parsed_line)" << endl;
	}
	return opt;
}

string get_station_name(uint32_t station_num) {
	string station_name;
	switch (station_num) {
		case 0:
			station_name = "Load";
			break;
		case 1:
			station_name = "Store";
			break;
		case 2:
			station_name = "Add";
			break;
		case 3:
			station_name = "Mult";
			break;
		default:
			cout << "error in parsing station number ot station name at get_station_name() " << endl;
	}
	return station_name;
}

uint32_t get_station_number(enum Operation opt) {
	uint32_t station_num;
	switch (opt) {
			case ADD:
				station_num = 2;
				break;
			case SUB:
				station_num = 2;
				break;
			case MULT:
				station_num = 3;
				break;
			case DIV:
				station_num = 3;
				break;
			case LOAD:
				station_num = 0;
				break;
			case STORE:
				station_num = 1;
				break;
			default:
				cout << "wrong stations assigned at stations assignemtn in switch case" << endl;
		}
	return station_num;
}

struct pointer_to_station get_station_pointer_from_string(string station) {
	uint32_t i = 0;
	while (isalpha(station[i])) {
		i += 1;
	}
	string station_type = station.substr(0, i);
	enum Operation type;
	uint32_t station_num = stoi(station.substr(i));
	
	if (station_type == "Load") {
		type = LOAD;
	} else if (station_type == "Add") {
		type = ADD;
	} else if (station_type == "Mult") {
		type = MULT;
	} else if (station_type == "Store") {
		type = STORE;
	} else {
		cout << "error at pointer to station conversion function : no match type for station type" << endl;
	}
	struct pointer_to_station pts;
	pts.type = type;
	pts.index = station_num;
	return pts;
}

bool compare_stations(string station, struct pointer_to_station pts2) {
	bool comparision = false;
	if (!station.empty()) {
		struct pointer_to_station pts1 = get_station_pointer_from_string(station);
		if ((pts1.type == pts2.type) && (pts1.index == pts2.index)) {
			comparision = true;
		}
	}
	return comparision;
}

uint32_t broadcast_stations(struct Stations* stations, string* bus, vector<InstructionStatus>* inst_statuses, uint32_t cycle) {
	uint32_t min_instruction_id = 100000;
	uint32_t station_type, station_number;
	uint32_t writes = 0;
	for (uint32_t i = 0; i < 4; i++) {
			for (uint32_t j = 0; j < stations[i].size; j++) {
				if (stations[i].busy[j] == true) {
					
					if (stations[i].operation_count[j] == OperationCycle[stations[i].operations[j]]) {
						if ((stations[i].instruction_id[j] < min_instruction_id)) {
							min_instruction_id = stations[i].instruction_id[j];
							station_type = i;
							station_number = j;
						} 
					}
				}
			}
	}

	if (min_instruction_id < 100000) {
		uint32_t i = station_type;
		uint32_t j = station_number;
		bus[0] = get_station_name(i) + std::to_string(j);
		stations[i].busy[j] = false;
		inst_statuses[0][stations[i].instruction_id[j]].cycleWriteResult = cycle + 1;
		writes += 1;
	}	

	return writes;
}

bool tick_stations(struct Stations* stations, string bus, vector<InstructionStatus>* inst_statuses, uint32_t cycle) {
	bool stations_busy = false;
	for (uint32_t i = 0; i < 4; i++) {
		for (uint32_t j = 0; j < stations[i].size; j++) {
			if (stations[i].busy[j] == true) {
				stations_busy = true;
				if (stations[i].vj[j] == true && stations[i].vk[j] == true) {
					if (stations[i].operation_count[j] < OperationCycle[stations[i].operations[j]]) {
						stations[i].operation_count[j] += 1;
						if (stations[i].operation_count[j] == OperationCycle[stations[i].operations[j]]) {
							inst_statuses[0][stations[i].instruction_id[j]].cycleExecuted = cycle + 1;
						}
					} 
						
				} else {	
					if (stations[i].vj[j] == false) {
						if (compare_stations(bus, stations[i].qj[j])) {
							stations[i].vj[j] = true;
						}
					}
					if (stations[i].vk[j] == false) {
						if (compare_stations(bus, stations[i].qk[j])) {
							stations[i].vk[j] = true;
						}
					}
				}
			}
		}
	}
	return stations_busy;
}



/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

/*
print the instruction status, the reservation stations and the register result status
@param filename: output file name
@param instructionStatus: instruction status
*/
void PrintResult4Grade(const string &filename, const vector<InstructionStatus> &instructionStatus)
{
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Instruction Status:\n";
	for (int idx = 0; idx < instructionStatus.size(); idx++)
	{
		outfile << "Instr" << idx << ": ";
		outfile << "Issued: " << instructionStatus[idx].cycleIssued << ", ";
		outfile << "Completed: " << instructionStatus[idx].cycleExecuted << ", ";
		outfile << "Write Result: " << instructionStatus[idx].cycleWriteResult << ", ";
		outfile << "\n";
	}
	outfile.close();
}

/*
print the register result status each 5 cycles
@param filename: output file name
@param registerResultStatus: register result status
@param thiscycle: current cycle
*/
void PrintRegisterResultStatus4Grade(const string &filename,
									 const RegisterResultStatuses &registerResultStatus,
									 const int thiscycle)
{
	if (thiscycle % 1 != 0)
		return;
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Cycle " << thiscycle << ":\n";
	outfile << registerResultStatus._printRegisterResultStatus() << "\n";
	outfile.close();
}

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		hardwareconfigname = argv[1];
		inputtracename = argv[2];
	}

	HardwareConfig hardwareConfig;
	std::ifstream config;
	config.open(hardwareconfigname);
	config >> hardwareConfig.LoadRSsize;  // number of load reservation stations
	config >> hardwareConfig.StoreRSsize; // number of store reservation stations
	config >> hardwareConfig.AddRSsize;	  // number of add reservation stations
	config >> hardwareConfig.MultRSsize;  // number of multiply reservation stations
	config >> hardwareConfig.FRegSize;	  // number of fp registers
	config.close();

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

	// Read instructions from a file (replace 'instructions.txt' with your file name)
	// ...
	
	// Initialize the register result status
	// RegisterResultStatuses registerResultStatus();
	// ...
	RegisterResultStatuses registers(hardwareConfig.FRegSize);
	// Initialize the instruction status table
	vector<InstructionStatus> instructionStatus;
	// ...
	string line;
	std::ifstream trace;
	trace.open(inputtracename);


	Stations* stations = get_stations(hardwareConfig);
	bool get_next_instruction = true;
	uint32_t instruction_id = -1;
	uint32_t cycles = 0;
	string bus;
	bool end_of_file = false;
	uint32_t write_backs = 0;	
	uint32_t fetched_instructions = 0;
	while (!((end_of_file == true) && (write_backs == fetched_instructions) && (instruction_id == (fetched_instructions - 1)) && (bus == ""))) {
		
		if ((end_of_file == false) && (get_next_instruction == true)) {
			getline(trace, line);
			instruction_id += 1;
		}
		get_next_instruction = false;
		
		vector<string> parsed_line = parse_instruction(line);
		
		if ((parsed_line.size() < 4) && (end_of_file == false)) {
			end_of_file = true;
			instruction_id -= 1;
		} else if (trace.eof() == true) {
			end_of_file = true;
		}
		
		// cout << " cycle : " << cycles << " | instruction : ";
		// for (uint32_t i = 0; i < parsed_line.size(); i++) {
		// 	cout << parsed_line[i] << " ";
		// }
		// cout << endl;

		write_backs += broadcast_stations(stations, &bus, &instructionStatus, cycles);


		tick_stations(stations, bus, &instructionStatus, cycles);
		registers.write_registers(bus);
		bus = "";
		
		// cout << "Operatoin : " << opt << " station : " << station_num << " " << get_station_name(station_num) << endl;
		
		if (instruction_id == fetched_instructions) {
			enum Operation opt = get_operation(parsed_line);
			uint32_t station_num = get_station_number(opt);
			for (uint32_t i = 0; i < stations[station_num].size; i++) {
				if (stations[station_num].busy[i] == false) {

					stations[station_num].busy[i] = true;
					stations[station_num].instruction_id[i] = instruction_id;
					stations[station_num].operations[i] = opt;
					stations[station_num].operation_count[i] = 0;
					
					if (parsed_line[2][0] == 'F') {
					
						uint32_t reg_num = stoi(parsed_line[2].substr(1));
						bool is_data_ready = registers.is_data_ready(reg_num);
						if (is_data_ready == true) {
							stations[station_num].vj[i] = true;
						} else {
							string reg_value = registers.get_register_value(reg_num);
							if (!reg_value.empty()) {
								stations[station_num].qj[i] = get_station_pointer_from_string(reg_value);
								stations[station_num].vj[i] = false;
							} else {
								cout << "register miss at parsed_line[2][0]" << endl;
							}
						}
					} else { stations[station_num].vj[i] = true; }
					
					if (opt == STORE) {
						if (parsed_line[1][0] == 'F') {
							uint32_t reg_num = stoi(parsed_line[1].substr(1));
							bool is_data_ready = registers.is_data_ready(reg_num);
							if (is_data_ready == true) {
								stations[station_num].vj[i] = true;
							} else {
								string reg_value = registers.get_register_value(reg_num);
								if (!reg_value.empty()) {
									stations[station_num].qj[i] = get_station_pointer_from_string(reg_value);
									stations[station_num].vj[i] = false;
								} else {
									cout << "reg miss at pointer[1][0]" << endl;;
								}	
							}
						} else { cout << "error in issuing store instruction in reading first register " << endl; }
					} 
					if (parsed_line[3][0] == 'F') {
						uint32_t reg_num = stoi(parsed_line[3].substr(1));
						bool is_data_ready = registers.is_data_ready(reg_num);
						if (is_data_ready == true) {
							stations[station_num].vk[i] = true;
						} else {
							string reg_value = registers.get_register_value(reg_num);
							if (!reg_value.empty()) {
								stations[station_num].qk[i] = get_station_pointer_from_string(reg_value);
								stations[station_num].vk[i] = false;
							} else {
								cout << "reg miss at parsed_line[3][0]" << endl;;
							}
						}
					} else { stations[station_num].vk[i] = true; }

					if (opt != STORE) {
						uint32_t reg_num = stoi(parsed_line[1].substr(1));
						registers.add_output(get_station_name(station_num) + std::to_string(i), reg_num);
					}
		
					get_next_instruction = true;
					fetched_instructions += 1;

					struct InstructionStatus inst_status;
					inst_status.cycleIssued = cycles + 1;
					instructionStatus.push_back(inst_status);
					break;
				}
			}
		}

  		// cout << "myset contains:";
  		// for ( auto it = bus.set.begin(); it != bus.set.end(); ++it )
    	// 		std::cout << " " << *it;
  		// 	std::cout << std::endl;

		// cout << ((get_next_instruction) ? ("Issued"):("Stalled")) << " | CDB : " << bus <<  endl;
	
		cycles += 1;
		PrintRegisterResultStatus4Grade(outputtracename, registers, cycles);

	}

	clear_stations(stations);
	trace.close();

	// Simulate Tomasulo:

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

	// At the end of the program, print Instruction Status Table for grading
	PrintResult4Grade(outputtracename, instructionStatus);

	return 0;
}
