#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

// (c) 2023 Preetham Rakshith Prakash 

using namespace std;

#define MemSize (65536)

class PhyMem    
{
  public:
    bitset<32> readdata;  
    PhyMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("pt_initialize.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {      
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open page table init file";
      dmem.close();

    }  
    bitset<32> outputMemValue (bitset<12> Address) 
    {    
      bitset<32> readdata;
      /**TODO: implement!
       * Returns the value stored in the physical address 
       */
      uint32_t addr = Address.to_ulong();
      uint32_t data = 0x00;
      for (uint8_t i = 0; i < 4; i++) {
        data <<= 8;
        data |= DMem[addr + i].to_ulong();
      }
      readdata = bitset<32>(data);
      return readdata;     
    }              

  private:
    vector<bitset<8>> DMem;

};  

int main(int argc, char *argv[])
{
    PhyMem myPhyMem;

    ifstream traces;
    ifstream PTB_file;
    ofstream tracesout;

    string outname;
    outname = "pt_results.txt";

    traces.open(argv[1]);
    PTB_file.open(argv[2]);
    tracesout.open(outname.c_str());

    //Initialize the PTBR
    bitset<12> PTBR;
    PTB_file >> PTBR;

    uint32_t ptbr_value = PTBR.to_ulong();

    string line;
    bitset<14> virtualAddr;

    /*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

    // Read a virtual address form the PageTable and convert it to the physical address - CSA23
    if(traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        {
            //TODO: Implement!
            // Access the outer page table 

            // If outer page table valid bit is 1, access the inner page table 

            //Return valid bit in outer and inner page table, physical address, and value stored in the physical memory.
            // Each line in the output file for example should be: 1, 0, 0x000, 0x00000000
          
          virtualAddr = bitset<14>(line);
       
          uint32_t outer_page_table = virtualAddr.to_ulong() >> 10;
          uint32_t inner_page_table = ((virtualAddr << 4) >> 10).to_ulong();
          uint32_t offset = ((virtualAddr << 8) >> 8).to_ulong();
          uint32_t level_1_address = (PTBR.to_ulong() + (outer_page_table << 2));
          uint32_t level_1_data = myPhyMem.outputMemValue(bitset<12>(level_1_address)).to_ulong();

          uint32_t valid_1 = 0; 
          uint32_t valid_2 = 0;  
          uint32_t physical_address = 0;
          uint32_t data = 0;       
          if ((0x1 & level_1_data) == 1) {
            valid_1 = 1;
            uint32_t level_2_address = (level_1_data >> 20) + (inner_page_table << 2);
            uint32_t level_2_data = myPhyMem.outputMemValue(bitset<12>(level_2_address)).to_ulong();
           
            if ((0x1 & level_2_data) == 1) {
              valid_2 = 1;
              physical_address = (((level_2_data >> 26) << 6) | offset);
              data = myPhyMem.outputMemValue(bitset<12>(physical_address)).to_ulong();
            } else {
              valid_2 = 0;
            }
          }
          stringstream ss1, ss2;
          ss1 << setfill('0') << setw(3) << hex << physical_address;
          ss2 << setfill('0') << setw(8) << hex << data;
          string result = to_string(valid_1) + ", " + to_string(valid_2) + ", 0x" + ss1.str() + ", 0x" + ss2.str() + "\n";
          tracesout << result;
        }
        traces.close();
        tracesout.close();
    }

    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
