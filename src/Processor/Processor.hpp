#pragma once

#include "./IOController/IOController.hpp"
#include "./ControlUnit/ControlUnit.hpp"
#include "./Memory/Memory.hpp"
#include "./PEActivation/PEActivation.hpp"
#include "./PEMultiplication/PEMultiplication.hpp"


SC_MODULE(Processor)
{
    sc_in<bool> clk_i;

    ControlUnit control_unit;
    IOController io_controller;
    Memory memory;
    PEActivation pe_activation;
    sc_vector<PEMultiplication> pe_multiplication;

    SC_HAS_PROCESS(Processor);

    Processor(sc_module_name nm);
    ~Processor(){};

    void start();

    void print_all_stats() const;
    void reset_all_stats();

    sc_signal<float> act_data;
    sc_signal<bool> act_start;

    sc_signal<bool> ioc_wr;
    sc_signal<bool> ioc_rd;
    sc_signal<size_t> ioc_res_addr;
    sc_signal<bool> ioc_busy;

    std::vector<sc_signal<size_t>> addr;
    std::vector<sc_signal<float>> data;
    std::vector<sc_signal<bool>> wr;
    std::vector<sc_signal<bool>> rd;

    std::vector<sc_signal<size_t>> data_cnt;
    std::vector<sc_signal<size_t>> w_start_addr;
    std::vector<sc_signal<size_t>> v_start_addr;
    std::vector<sc_signal<bool>> load;
    std::vector<sc_signal<bool>> busy;
};
