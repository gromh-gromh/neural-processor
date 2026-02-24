#include "PEActivation.hpp"

PEActivation::PEActivation(sc_module_name nm):
    sc_module(nm),
    clk_i("pe_activation_clk"),
    act_data_io("pe_activation_data"),
    act_start_i("pe_activation_start")
{
    SC_THREAD(process);
    sensitive << clk_i.pos();
}

void PEActivation::process()
{
    while(1)
    {
        if(act_start_i.read())
        {
            float value = act_data_io->read();
            wait();
            act_data_io->write(1.0 / (exp(-value) + 1));
        }
        wait();
    }
}