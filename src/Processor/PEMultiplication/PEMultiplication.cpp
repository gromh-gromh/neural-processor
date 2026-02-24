#include "PEMultiplication.hpp"
#include "../constants.hpp"

PEMultiplication::PEMultiplication() : PEMultiplication("nc") {};

PEMultiplication::PEMultiplication(sc_module_name nm)
    : sc_module(nm)
{
    state = LOAD_DATA;
    SC_THREAD(load_data);
    sensitive << clk_i.pos();

    SC_THREAD(process);
    sensitive << clk_i.pos();

    SC_THREAD(write_result);
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