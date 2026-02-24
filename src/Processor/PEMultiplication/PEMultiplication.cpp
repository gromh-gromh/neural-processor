#include "PEMultiplication.hpp"
#include "../constants.hpp"

PEMultiplication::PEMultiplication() : PEMultiplication("nc") {};

PEMultiplication::PEMultiplication(sc_module_name nm)
    : sc_module(nm)
{
    state = LOAD_DATA;
    prev_state = LOAD_DATA;

    total_operations = 0;
    total_cycles_active = 0;
    total_cycles_idle = 0;
    load_count = 0;
    process_count = 0;
    cycle_count = 0;

    SC_THREAD(load_data);
    sensitive << clk_i.pos();

    SC_THREAD(process);
    sensitive << clk_i.pos();

    SC_THREAD(write_result);
    sensitive << clk_i.pos();

    SC_THREAD(profile_monitor);
    sensitive << clk_i.pos();
}

void PEMultiplication::load_data()
{
    while (1)
    {
        rd_o.write(0);
        while (state != LOAD_DATA || !load_i.read())
            wait();
        busy_o->write(1);
        size_t size = data_cnt_i->read();
        size_t w_addr = w_start_addr_i->read();
        size_t v_addr = v_start_addr_i->read();
        for (size_t i = 0; i < size; ++i)
        {
            addr_o->write(w_addr + i);
            rd_o->write(1);
            wait();
            rd_o->write(0);
            wait();
            wait(SC_ZERO_TIME);
            weight_queue.push(data_io->read());
            addr_o->write(v_addr + i);
            rd_o->write(1);
            wait();
            rd_o->write(0);
            wait();
            wait(SC_ZERO_TIME);
            value_queue.push(data_io->read());
        }
        state = PROCESS;
    }
}

void PEMultiplication::process()
{
    while (1)
    {
        while (state != PROCESS)
            wait();
        accumulator = 0;
        while (!weight_queue.empty() && !value_queue.empty())
        {
            for (size_t i = 0; i < pe_multiplication_size; ++i)
            {
                if (weight_queue.empty() || value_queue.empty())
                    break;

                total_operations++;
                    
                accumulator += weight_queue.front() * value_queue.front();
                weight_queue.pop();
                value_queue.pop();
            }
            wait();
        }
        state = RESULT;
    }
}

void PEMultiplication::write_result()
{
    while (1)
    {
        while (state != RESULT)
            wait();
        data_io->write(accumulator);
        wr_o->write(1);
        wait();
        wr_o->write(0);
        wait();
        wait(SC_ZERO_TIME);
        busy_o->write(0);
        state = LOAD_DATA;
    }
}

void PEMultiplication::profile_monitor()
{
    while (1)
    {
        cycle_count++;
        
        if (state != LOAD_DATA || !weight_queue.empty()) {
            total_cycles_active++;
        } else {
            total_cycles_idle++;
        }
        
        prev_state = state;
        
        wait();
    }
}

void PEMultiplication::print_stats() const
{
    std::cout << "    Total operations: " << total_operations << std::endl;
    std::cout << "    Active cycles: " << total_cycles_active << std::endl;
    std::cout << "    Idle cycles: " << total_cycles_idle << std::endl;
    
    unsigned long long total_cycles = total_cycles_active + total_cycles_idle;
    if (total_cycles > 0) {
        double utilization = (double)total_cycles_active / total_cycles * 100.0;
        std::cout << "    Utilization: " << utilization << "%" << std::endl;
    }
    
    if (process_count > 0) {
        double avg_ops_per_process = (double)total_operations / process_count;
        std::cout << "    Avg operations per process: " << avg_ops_per_process << std::endl;
    }
}

void PEMultiplication::reset_stats()
{
    total_operations = 0;
    total_cycles_active = 0;
    total_cycles_idle = 0;
    load_count = 0;
    process_count = 0;
    cycle_count = 0;
}