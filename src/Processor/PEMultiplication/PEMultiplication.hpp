#pragma once

#include "systemc.h"
#include <queue>

SC_MODULE(PEMultiplication)
{
    sc_in<bool> clk_i;
    sc_in<size_t> data_cnt_i;
    sc_in<size_t> w_start_addr_i;
    sc_in<size_t> v_start_addr_i;
    sc_in<bool> load_i;
    sc_out<bool> busy_o;

    sc_out<size_t> addr_o;
    sc_inout<float> data_io;
    sc_out<bool> wr_o;
    sc_out<bool> rd_o;

    sc_event process_event;
    sc_event write_result_event;

    std::queue<float> weight_queue;
    std::queue<float> value_queue;

    SC_HAS_PROCESS(PEMultiplication);
    PEMultiplication();
    PEMultiplication(sc_module_name nm);
    ~PEMultiplication(){};

    void process();
    void write_result();
    void load_data();

    void print_stats() const;
    void reset_stats();
    
    unsigned long long total_operations;
    unsigned long long total_cycles_active;
    unsigned long long total_cycles_idle;
    unsigned long long load_count;
    unsigned long long process_count;

private:
    enum
    {
        LOAD_DATA = 0,
        PROCESS = 1,
        RESULT = 2,
    } state;
    float accumulator;

    unsigned long long cycle_count;
    int prev_state;
    void profile_monitor();
};