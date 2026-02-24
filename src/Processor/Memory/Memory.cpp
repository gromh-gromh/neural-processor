#include "Memory.hpp"

Memory::Memory(sc_module_name nm)
    : sc_module(nm),
      addr_i("lm_addr"),
      data_io("lm_data"),
      wr_i("lm_wr"),
      rd_i("lm_rd")
{
    SC_THREAD(bus_write);
    sensitive << clk_i.pos();

    SC_THREAD(bus_read);
    sensitive << clk_i.pos();
}

void Memory::bus_read()
{
    while (1)
    {
        for (size_t i = 0; i < 1; ++i)
        {
            if (wr_i[i]->read())
            {
                mem[addr_i[i]->read()] = data_io[i]->read();
            }
        }
        wait();
    }
}

void Memory::bus_write()
{
    while (1)
    {
        for (size_t i = 0; i < local_mem_slave_count; ++i)
        {
            if (prepared_write_queue[i].first) {
                data_io[i]->write(prepared_write_queue[i].second);
                prepared_write_queue[i] = {false, 0};
            }
            if (rd_i[i]->read())
            {
                prepared_write_queue[i] = {true, mem[addr_i[i]->read()]};
            }
        }
        wait();
    }
}