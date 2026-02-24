#pragma once
#include <systemc.h>

SC_MODULE(PEActivation)
{
    sc_in<bool> clk_i;
    sc_inout<float> act_data_io;
    sc_in<bool> act_start_i;

    SC_HAS_PROCESS(PEActivation);

    PEActivation(sc_module_name nm);
    ~PEActivation() {};

    void process();
};
