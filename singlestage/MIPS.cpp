// Copyright (c) Preetham Rakshith Prakash 

#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define ADDU (1)
#define SUBU (3)
#define AND (4)
#define OR  (5)
#define NOR (7)

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize (65536)

bool DEBUG = false;

class RF
{
  public:
    bitset<32> ReadData1, ReadData2; 
    RF()
    { 
      Registers.resize(32);  
      Registers[0] = bitset<32> (0);  
      // TESTING REGISTERS;
    }

    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {   
      /**
       * @brief Reads or writes data from/to the Register.
       *
       * This function is used to read or write data from/to the register, depending on the value of WrtEnable.
       * Put the read results to the ReadData1 and ReadData2.
       */
      // TODO: implement!

      ReadData1 = Registers[RdReg1.to_ulong()];
      ReadData2 = Registers[RdReg2.to_ulong()];

      if (WrtEnable.test(0) == 1) {
        Registers[WrtReg.to_ulong()] = WrtData;
      }

      if (DEBUG) {
        cout << "Register File Input :: " << " RdReg1 : " << RdReg1 << " RdReg2 : " << RdReg2 << " WrtReg : " << WrtReg << " WrtData : " << WrtData << " WrtEnable : " << WrtEnable << endl;
        cout << "Register File Output :: " << " ReadData1 : " << ReadData1 << " ReadData2 : " << ReadData2 << endl;
      }

    }

    void OutputRF()
    {
      ofstream rfout;
      rfout.open("RFresult.txt",std::ios_base::app);
      if (rfout.is_open())
      {
        rfout<<"A state of RF:"<<endl;
        for (int j = 0; j<32; j++)
        {        
          rfout << Registers[j]<<endl;
        }

      }
      else cout<<"Unable to open file";
      rfout.close();

    }     
  private:
    vector<bitset<32>> Registers;
};


class ALU
{
  public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {   
      /**
       * @brief Implement the ALU operation here.
       *
       * ALU operation depends on the ALUOP, which are definded as ADDU, SUBU, etc. 
       */
      // TODO: implement!

      // https://inst.eecs.berkeley.edu/~cs61c/resources/MIPS_help.html

      uint32_t oprand_1 = oprand1.to_ulong();
      uint32_t oprand_2 = oprand2.to_ulong();
      uint32_t result = 0x00;

      switch(ALUOP.to_ulong()) {
        case ADDU:
          result = oprand_1 + oprand_2;
          break;
        case SUBU:
          result = oprand_1 - oprand_2;
          break;
        case AND:
          result = oprand_1 & oprand_2;
          break;
        case OR:
          result = oprand_1 | oprand_2;
          break;
        case NOR:
          result = ~(oprand_1 | oprand_2);
          break;
        default:
          result = 0;
      }
      ALUresult = result;

      if (DEBUG) {
        cout << "ALU :: " << " ALUOP : " << ALUOP << " oprand1 : " << oprand1 << " oprand2 : " << oprand2 << " ALUresult : " << ALUresult << endl;
      }

      return ALUresult;
    }            
};


class INSMem
{
  public:
    bitset<32> Instruction;
    INSMem()
    {       IMem.resize(MemSize); 
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

      }
      else cout<<"Unable to open file";
      imem.close();

    }

    bitset<32> ReadMemory (bitset<32> ReadAddress) 
    {    
      // TODO: implement!
      /**
       * @brief Read Instruction Memory (IMem).
       *
       * Read the byte at the ReadAddress and the following three byte,
       * and return the read result. 
       */
      
      // https://www.geeksforgeeks.org/cpp-bitset-and-its-application/
      // IMem is big endian. so load data from 0 -> 3. 
      // load to a 32 bit word from left to right. (MSB -> LSB).
      uint32_t address = ReadAddress.to_ulong();
      uint32_t Inst = 0x00;
      for (uint8_t i = 0; i < 4; i++) {
        Inst <<= 8;
        Inst |= IMem[address + i].to_ulong();
      }
      Instruction = Inst;

      if (DEBUG) {
        cout << "Instruction Memory :: " << " ReadAddress : " << ReadAddress << " Instruction : " << Instruction << endl;
      }

      return Instruction;     
    }     

  private:
    vector<bitset<8>> IMem;

};

class DataMem    
{
  public:
    bitset<32> readdata;  
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
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem) 
    {    
      /**
       * @brief Reads/writes data from/to the Data Memory.
       *
       * This function is used to read/write data from/to the DataMem, depending on the readmem and writemem.
       * First, if writemem enabled, WriteData should be written to DMem, clear or ignore the return value readdata,
       * and note that 32-bit WriteData will occupy 4 continious Bytes in DMem. 
       * If readmem enabled, return the DMem read result as readdata.
       */
      // TODO: implement!

      uint32_t address = Address.to_ulong();
      uint32_t write_data = WriteData.to_ulong();
      readdata.reset();
      if (writemem.to_ulong() == 1) {
        DMem[address + 0] = write_data >> 24;
        DMem[address + 1] = (write_data & 0x00ff0000) >> 16;
        DMem[address + 2] = (write_data & 0x0000ff00) >> 8;
        DMem[address + 3] = write_data & 0x000000ff;
      } else if (readmem.to_ulong() == 1) {
        uint32_t read_data = 0;
        for (uint8_t i = 0; i < 4; i++) {
          read_data <<= 8;
          read_data |= DMem[address + i].to_ulong();
        }
        readdata = read_data;
      }

      if (DEBUG) {
        cout << "DataMem :: " << " Address : " << Address << " WriteData : " << WriteData << " readmem : " << readmem << " writemem : " << writemem << " readdata : " << readdata << endl;
      }

      return readdata;     
    }   

    void OutputDataMem()
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



int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;
  
  // Program counter initialized from 0.
  bitset<32> PC = bitset<32>(0x00);
  uint32_t next_PC = 0x00;

  // Decode variables.
  uint32_t op_code = 0x00;
  uint32_t rs = 0x00;
  uint32_t rt = 0x00;
  uint32_t rd = 0x00;
  uint32_t funct = 0x00;

  bool is_load = false;
  bool is_store = false;
  bool is_branch = false;
  bool i_type = false;
  bool j_type = false;
  bool r_type = false;

  
  uint32_t alu_op = 0x00;
  uint32_t write_enable = 0x01;

  uint32_t write_data = 0x00;

  uint32_t read_data_1 = 0x00;
  uint32_t read_data_2 = 0x00;
  uint32_t read_data_2_alu = 0x00;

  uint32_t alu_result = 0x00;
  uint32_t immediate = 0x00;
  uint32_t mem_read_data = 0x00;

  uint32_t j_type_PC = 0x00;

  while (1)  // TODO: implement!
  {
    // Fetch: fetch an instruction from myInsMem.

    // If current instruction is "11111111111111111111111111111111", then break; (exit the while loop)

    // decode(Read RF): get opcode and other signals from instruction, decode instruction

    // Execute: after decoding, ALU may run and return result

    // Read/Write Mem: access data memory (myDataMem)

    // Write back to RF: some operations may write things to RF
    
    r_type = false;
    j_type = false;
    i_type = false;
    is_load = false;
    is_store = false;
    is_branch = false;
    j_type_PC = 0;

    if (DEBUG) {
      cout << " " << endl;
      cout << " " << endl;
      cout << "Program Counter :: " << PC << " " << PC.to_ulong() << endl;
    }

    bitset<32> instructions = myInsMem.ReadMemory(PC);
    PC = PC.to_ulong() + 4;

    op_code = (instructions >> 26).to_ulong();
    if (op_code == 0x3f) {
      if (DEBUG) {
        cout << "EXIT()" << endl;
      }
      break;
    }

    j_type_PC = (PC.to_ulong() & 0xf0000000) | ((instructions << 6) >> 4).to_ulong() ;

    rs = ((instructions << 6) >> 27).to_ulong();
    rt = ((instructions << 11) >> 27).to_ulong();
    rd = ((instructions << 16) >> 27).to_ulong();
    funct = ((instructions << 26) >> 26).to_ulong();

    if (op_code == 0x23) {
      is_load = true; // is_load ?
    }

    if (op_code == 0x2b) {
      is_store = true; // is_store ?
    }

    if ((op_code != 0) && (op_code != 2) && (op_code != 3)) {
      i_type = true;
      if (op_code == 0x04) {
        is_branch = true;
      }
      rd = rt;
      immediate = ((instructions << 16) >> 16).to_ulong();
      if ((immediate & 0x8000) == 0x8000) {
        immediate |= 0xffff0000;
      }
    } else if ((op_code == 2) || (op_code == 3)) {
      j_type = true;
      PC = j_type_PC;
    } else {
      r_type = true;
    }

    if ( (op_code == 0x23) || (op_code == 0x2b) ) {
      alu_op = 1;
    } else if (op_code == 0x00) {
      alu_op = (funct << 3) >> 3;
    } else {
      alu_op = (op_code << 3) >> 3;
    }

    if (is_store || is_branch || j_type) {
      write_enable = 0;
    } else {
      write_enable = 1;
    }

    // if (is_store) {
    //   rd = rs;
    //   rs = rt;
    //   rt = rd;
    // }

    myRF.ReadWrite(bitset<5>(rs), bitset<5>(rt), bitset<5>(rd), bitset<32>(write_data), bitset<1>(0));
    read_data_1 = myRF.ReadData1.to_ulong();
    read_data_2 = myRF.ReadData2.to_ulong();
    read_data_2_alu = myRF.ReadData2.to_ulong();

    if (is_branch) {
      if (read_data_1 == read_data_2) {
        PC = PC.to_ulong() + (immediate << 2);
      } 
    }

    if (i_type) {
      read_data_2_alu = immediate;
    }

    alu_result = myALU.ALUOperation( bitset<3>(alu_op), bitset<32>(read_data_1), bitset<32>(read_data_2_alu)).to_ulong();

    mem_read_data = myDataMem.MemoryAccess(bitset<32>(alu_result), bitset<32>(read_data_2), bitset<1>(is_load), bitset<1>(is_store)).to_ulong();

    if (is_load == true) {
      alu_result = mem_read_data;
    }

    myRF.ReadWrite(bitset<5>(rs), bitset<5>(rt), bitset<5>(rd), bitset<32>(alu_result), bitset<1>(write_enable));

    /**** You don't need to modify the following lines. ****/
    myRF.OutputRF(); // dump RF;    
  }
  myDataMem.OutputDataMem(); // dump data mem

  return 0;
}
