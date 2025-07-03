#include<iostream>
#include <ostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;

// (c) Preetham Rakshith Prakash 
// logic for load hazard implementation - https://youtu.be/V8YFDHft8XA?si=RWnPqI_s_bNx8js- 
// logic and examples for load hazards - https://www.cs.cornell.edu/courses/cs3410/2012sp/lecture/09-pipelined-cpu-i-g.pdf
// more examples for load hazards - http://www.cs.fsu.edu/~zwang/files/cda3101/Fall2017/Lecture7_cda3101.pdf


#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab csa23, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

void Print_if_struct(IFStruct if_struct) {
    cout << "if_struct" << endl;
    cout << "PC : " << if_struct.PC << " " << " nop : " << if_struct.nop << endl;
    cout << " " << endl;
}

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

void Print_id_struct(IDStruct id_struct) {
    cout << "id struct" << endl;
    cout << "Instruction : " << id_struct.Instr << " " << "nop : " << id_struct.nop << endl;
    cout << " " << endl;
}

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

void Print_ex_struct(EXStruct ex_struct) {
    cout << "ex struct" << endl;
    cout << "rd1 : " << ex_struct.Read_data1 << endl;
    cout << "rd2 : " << ex_struct.Read_data2 << endl;
    cout << "Imm : " << ex_struct.Imm << endl;
    cout << "rs : " << ex_struct.Rs << " " << "rt : " << ex_struct.Rt << " " << "rd : " << ex_struct.Wrt_reg_addr << endl;
    cout << "i type ? : " << ex_struct.is_I_type << " " << "rd mem ? : " <<  ex_struct.rd_mem << " " << "wrt mem ? : " << ex_struct.wrt_mem << " " << "alu op : " <<  ex_struct.alu_op << " " << "wrt enable ? : " <<  ex_struct.wrt_enable << " " << " nop : " << ex_struct.nop << endl;
    cout << " " << endl;
}

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;   
};

void Print_mem_struct(MEMStruct mem_struct) {
    cout << "mem struct" << endl;
    cout << "alu result : " << mem_struct.ALUresult << endl;
    cout << "store data : " << mem_struct.Store_data << endl;
    cout << "rs : " << mem_struct.Rs << " " << "rt : " << mem_struct.Rt << "Wrt_reg_addr : " << mem_struct.Wrt_reg_addr << endl;
    cout << "rd mem ? : " << mem_struct.rd_mem << " " << "wrt mem ? : " << mem_struct.wrt_mem << " " << "wrt enable ? : " <<  mem_struct.wrt_enable << " " << "nop : " <<  mem_struct.nop  << endl;
    cout << " " << endl;
}

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

void Print_wb_struct(WBStruct wb_struct) {
    cout << "wb struct" << endl;
    cout << "wrt data : " << wb_struct.Wrt_data << endl;
    cout << "rs : " << wb_struct.Rs << " " << "rt : " << wb_struct.Rt << endl;
    cout << "wrt reg : " << wb_struct.Wrt_reg_addr << " " << "wrt enable ? : " << wb_struct.wrt_enable << " " << "nop : " << wb_struct.nop << endl;
    cout << " " << endl;
}

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{   
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			} else {
                cout<<"Unable to open file";
            };
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}

struct decodeStruct {

    uint32_t opcode;
    uint32_t rs;
    uint32_t rt;
    uint32_t rd;
    
    bool alu_op;
    uint32_t write_enable;

    uint32_t immediate;

    bool is_branch;
    bool i_type;
    bool r_type;
    bool is_load;
    bool is_store;
    bool is_halt;

};

void Print_decode(decodeStruct decode) {
    cout << "decode : " << endl;
    cout << "rs : " << decode.rs << endl;
    cout << "rt : " << decode.rt << endl;
    cout << "rd : " << decode.rd << endl;
    cout << "immediate : " << decode.immediate << endl;
    cout << "alu op : " << decode.alu_op << endl;
    cout << "write enable : " << decode.write_enable << endl;
    cout << "is load : " << decode.is_load << endl;
    cout << "is store : " << decode.is_store << endl;
    cout << "is branch : " << decode.is_branch << endl;
    cout << "is halt : " << decode.is_halt << endl;
    cout << "is i type : " << decode.i_type << endl;
    cout << "is r type : " << decode.r_type << endl;
    return;
}

decodeStruct decoder(uint32_t instruction) {
    
    decodeStruct decode;

    uint32_t op_code = (instruction >> 26);
    uint32_t rs = ((instruction << 6) >> 27);
    uint32_t rt = ((instruction << 11) >> 27);
    uint32_t rd = ((instruction << 16) >> 27);
    uint32_t funct = ((instruction << 26) >> 26);

    uint32_t immediate = 0;
    bool alu_op = false;
    bool write_enable = false;

    bool is_load = false;
    bool is_store = false;
    bool is_branch = false;
    bool is_halt = false;
    bool is_r_type = false;
    bool is_i_type = false;

    if (op_code == 0x23) {
        is_load = true;
        is_i_type = true;
    } else if (op_code == 0x2b) {
        is_store = true;
        is_i_type = true;
    } else if (op_code == 0x05) {
        is_branch = true;
        is_i_type = true;
    } else if (op_code == 0x3f) {
        is_halt = true;
        rs = 0;
        rt = 0;
        rd = 0;
    } else if (funct != 0) {
        is_r_type = true;
    }

    if (is_r_type == true) {
        alu_op = (funct == 0x21) ? (true) : (false);
        write_enable = true;
    } else if (is_i_type == true) {
        if (is_load == true) {
            alu_op = true;
            write_enable = true;
        } else if (is_store == true) {
            alu_op = true;
            write_enable = false;
        } else if (is_branch == true) {
            alu_op = false;
        }
        rd = rt;
        immediate = ((instruction << 16) >> 16);
    }

    decode.rs = rs;
    decode.rt = rt;
    decode.rd = rd;
    decode.immediate = immediate;
    decode.is_load = is_load;
    decode.is_store = is_store;
    decode.alu_op = alu_op;
    decode.i_type = is_i_type;
    decode.r_type = is_r_type;
    decode.is_branch = is_branch;
    decode.write_enable = write_enable;
    decode.is_branch = is_branch;
    decode.is_halt = is_halt;

    return decode;
}


int main()
{
    // cout << "\n\n\n\n\n\n\n\n";

    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

    stateStruct state, newState;
    decodeStruct decoded;
    
    int cycle = 0;
    uint32_t instruction = 0;
    bitset<32> Alu_Result;
    bitset<32> Mem_Data;
    bitset<32> ReadReg1;
    bitset<32> ReadReg2;
    uint32_t immediate;
			
    WBStruct Write_Back;

    state.IF.PC = 0;

    state.WB.wrt_enable = 0;

    state.MEM.rd_mem = 0;
    state.MEM.wrt_mem = 0;
    state.MEM.wrt_enable = 0;

    state.EX.is_I_type = 0;
    state.EX.rd_mem = 0;
    state.EX.wrt_mem = 0;
    state.EX.alu_op = 0;
    state.EX.wrt_enable = 0;

    state.IF.nop = 0;
    state.ID.nop = 1;
    state.EX.nop = 1;
    state.MEM.nop = 1;
    state.WB.nop = 1;

    uint32_t jump_address = 0;
    
    while (1) {
        
        /* --------------------- WB stage --------------------- */
        // Print_wb_struct(state.WB);

        // struct WBStruct {
        //     bitset<32>  Wrt_data;
        //     bitset<5>   Rs;
        //     bitset<5>   Rt;     
        //     bitset<5>   Wrt_reg_addr;
        //     bool        wrt_enable;
        //     bool        nop;     
        // };

        if (state.WB.wrt_enable == true && state.WB.nop == false) {
            myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
        }
        
        /* --------------------- MEM stage --------------------- */

        // struct MEMStruct {
        //     bitset<32>  ALUresult;
        //     bitset<32>  Store_data;
        //     bitset<5>   Rs;
        //     bitset<5>   Rt;    
        //     bitset<5>   Wrt_reg_addr;
        //     bool        rd_mem;
        //     bool        wrt_mem; 
        //     bool        wrt_enable;    
        //     bool        nop;    
        // };

        if (state.MEM.nop == false) {
            if (state.MEM.rd_mem == true) {
                Mem_Data = myDataMem.readDataMem(state.MEM.ALUresult);
                newState.WB.Wrt_data = Mem_Data;
            } else if (state.MEM.wrt_mem == true) {
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                newState.WB.Wrt_data = state.MEM.ALUresult;
            } else {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            } 
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;
            newState.WB.nop = false;
        } else {
            newState.WB = state.WB;
            newState.WB.nop = true;
        }

        // Print_mem_struct(state.MEM);
        //////
       
        /* --------------------- EX stage --------------------- */
        
        // struct EXStruct {
        //     bitset<32>  Read_data1;
        //     bitset<32>  Read_data2;
        //     bitset<16>  Imm;
        //     bitset<5>   Rs;
        //     bitset<5>   Rt;
        //     bitset<5>   Wrt_reg_addr;
        //     bool        is_I_type;
        //     bool        rd_mem;
        //     bool        wrt_mem; 
        //     bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
        //     bool        wrt_enable;
        //     bool        nop;  
        // };
        

        if (state.EX.nop == false) {

            uint32_t alu_input_1 = state.EX.Read_data1.to_ulong();
            uint32_t read_data_2  = state.EX.Read_data2.to_ulong();
            uint32_t alu_input_2 = read_data_2;


            // MEM TO EX AND WB TO EX FORWARDING.
            // Forwarding for both execution ( addu and subu ) and sw. 
            if (state.MEM.nop == false && state.MEM.wrt_enable == true && state.EX.Rs == state.MEM.Wrt_reg_addr) {
                alu_input_1 = state.MEM.ALUresult.to_ulong();
            } else if (state.WB.nop == false && state.WB.wrt_enable == true && state.EX.Rs == state.WB.Wrt_reg_addr) {
                alu_input_1 = state.WB.Wrt_data.to_ulong();
            }

            if (state.MEM.nop == false && state.MEM.wrt_enable == true && state.EX.Rt == state.MEM.Wrt_reg_addr) {
                read_data_2 = state.MEM.ALUresult.to_ulong();
            } else if (state.WB.nop == false && state.WB.wrt_enable == true && state.EX.Rt == state.WB.Wrt_reg_addr) {
                read_data_2 = state.WB.Wrt_data.to_ulong();
            }
            
            immediate = state.EX.Imm.to_ulong();
            if ((immediate & 0x8000) == 0x8000) {
                immediate |= 0xffff0000;
            }
        
            if (state.EX.is_I_type == true) {
                alu_input_2 = immediate;
                newState.MEM.Store_data = read_data_2; // Storing the Store value in case of i type.
            } else {
                alu_input_2 = read_data_2;
                newState.MEM.Store_data = 0; // If not I type then set mem.store_data to 0.
            }

            uint32_t alu_result = 0;
            if (state.EX.alu_op == true) {
                alu_result = alu_input_1 + alu_input_2;
            } else if (state.EX.alu_op == false) {
                alu_result = alu_input_1 - alu_input_2;
            } else {
                cout << "alu op code bool error " << endl;
            }

            // cout << "ALU OPERATION : " << alu_result << " " << alu_input_1 << " " << alu_input_2 << endl;

            newState.MEM.ALUresult = alu_result;
            newState.MEM.Store_data = read_data_2;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.nop = false;
        } else {
            newState.MEM = state.MEM;
            newState.MEM.nop = true;
        }

        // Print_ex_struct(state.EX);

        /////////////////
        
        /* --------------------- ID stage --------------------- */
        
        
        decodeStruct decoded_instruction = decoder(state.ID.Instr.to_ulong());

        bitset<5> rs = decoded_instruction.rs;
        bitset<5> rt = decoded_instruction.rt;
        bitset<5> rd = decoded_instruction.rd;            
        
        bool load_dependency_for_stalling = false;
        // check if stall is required if we encounter load dependencies during decoding. 
        if (state.EX.nop == false && state.EX.rd_mem == true) {
            if (decoded_instruction.i_type == true || decoded_instruction.r_type == true) {
                // load - load hazards.
                if (decoded_instruction.is_load == true) {
                    if (rs == state.EX.Wrt_reg_addr) {
                        load_dependency_for_stalling = true;
                    } 
                } else {
                    // load -exec and load - store hazards.
                    if (rt == state.EX.Wrt_reg_addr || rs == state.EX.Wrt_reg_addr) {
                        load_dependency_for_stalling = true;
                    }
                }
            }
        }

        if (state.ID.nop == false) {
            newState.EX.Read_data1 = myRF.readRF(rs);
            newState.EX.Read_data2 = myRF.readRF(rt);
            newState.EX.Rs = rs;
            newState.EX.Rt = rt;
            newState.EX.Wrt_reg_addr = rd;
            newState.EX.wrt_enable = decoded_instruction.write_enable;
            newState.EX.Imm = bitset<16>(decoded_instruction.immediate);
            newState.EX.is_I_type = decoded_instruction.i_type;
            newState.EX.rd_mem = decoded_instruction.is_load;
            newState.EX.wrt_mem = decoded_instruction.is_store;
            newState.EX.alu_op = decoded_instruction.alu_op;
            newState.EX.nop = false;
            if (load_dependency_for_stalling == true) {
                newState.EX.nop = true;
                state.IF.nop = true;
            } else if (decoded_instruction.is_branch == true) {
                // jump instruction. 
                if (newState.EX.Read_data1.to_ulong() != newState.EX.Read_data2.to_ulong()) {
                    immediate = decoded_instruction.immediate;
                    if ((immediate & 0x8000) == 0x8000) {
                        immediate |= 0xffff0000;
                    }
                    immediate <<= 2;
                    state.IF.PC = state.IF.PC.to_ulong() + immediate;
                }
                newState.EX.nop = true;
            }
        } else {
            newState.EX = state.EX;
            newState.EX.nop = true;
        }

        // Print_id_struct(state.ID);
        //////
       
        /* --------------------- IF stage --------------------- */

        // newState.IF.nop = state.IF.nop;
        if (state.IF.nop == false) {
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            newState.IF.PC = state.IF.PC.to_ulong() + 4;
        } else {
            newState.ID.Instr = state.ID.Instr;
            newState.IF.PC = state.IF.PC;
        }

        if (newState.ID.Instr.to_ulong() == 0xffffffff) {
            newState.IF.nop = true;
            newState.ID.nop = true;
        } else if (newState.ID.Instr.to_ulong() == 0x00000000) {
            newState.ID.nop = true;
            newState.IF.nop = false;
        } else {
            newState.ID.nop = false;
            newState.IF.nop = false;
        }

        // Print_if_struct(state.IF);

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop) {
            break;
        }
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
        cycle += 1;

        state = newState; 
        /*** The end of the cycle and updates the current state with the values calculated in this cycle. csa23 ***/ 
        // cout << "--------------------------- END OF CYCLE --------------------------------" << endl;

    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}
