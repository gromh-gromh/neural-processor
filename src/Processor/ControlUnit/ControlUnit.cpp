#include "ControlUnit.hpp"

ControlUnit::ControlUnit(sc_module_name nm)
    : sc_module(nm),
      neuron_addr_link(slave_count),
      local_layer(0),
      local_neuron(0),
      device_load(0),
      device_busy(0)
{
    for (size_t i = 0; i < slave_count; ++i)
        device_busy.push_back(false);

    SC_THREAD(load_data);
    sensitive << clk_i.pos();

    SC_THREAD(process);
    sensitive << clk_i.pos();

    SC_THREAD(result);
    sensitive << clk_i.pos();

    SC_THREAD(read_mem_mapped);
    sensitive << clk_i.pos();

    SC_THREAD(free_device);
    sensitive << clk_i.pos();
}

void ControlUnit::start()
{
    // cout << "ControlUnit::start" << endl;
    state = LOAD_DATA;
    for(size_t i = 0; i < slave_count; ++i)
        load_o[i]->write(0);
}

inline void ControlUnit::write_mem(float data, size_t addr)
{
    addr_o->write(addr);
    data_io->write(data);
    wr_o->write(1);
    wait();
    wr_o->write(0);
}

inline float ControlUnit::read_mem(size_t addr)
{
    addr_o->write(addr);
    rd_o->write(1);
    wait();
    rd_o->write(0);
    wait();
    wait(SC_ZERO_TIME);
    float value = data_io->read();
    return value;
}

void ControlUnit::load_data()
{
    while (1)
    {
        // cout << "ControlUnit::load_data" << endl;
        while (state != LOAD_DATA)
            wait();

        ioc_wr_o->write(1);
        wait();
        ioc_wr_o->write(0);
        while (!ioc_busy_i.read())
            wait();
        while (ioc_busy_i.read())
            wait();
        input_size = read_mem(0);
        output_size = read_mem(1);
        size_t layer_count = read_mem(2);
        neuron_layer_size.reserve(layer_count);
        neuron_layer_size.resize(layer_count, 0);
        weight_local_addr = layer_count + 1;
        value_local_addr = layer_count + 1;

        neuron_layer_size[0] = input_size;
        for (size_t i = 0; i < layer_count - 2; ++i)
        {
            neuron_layer_size[i + 1] = read_mem(3 + i);
            value_local_addr += neuron_layer_size[i] * neuron_layer_size[i + 1];
        }
        neuron_layer_size[layer_count - 1] = output_size;
        value_local_addr += neuron_layer_size[layer_count] * output_size;
        state = PROCESS;
    }
}

void ControlUnit::process_mem_mapped()
{
    if (!mem_mapped.empty())
    {
        size_t device_to_save = mem_mapped.front().first;
        float value_to_save = mem_mapped.front().second;
        mem_mapped.pop();
        act_data_io->write(value_to_save);
        act_start_o->write(1);
        wait();
        act_start_o->write(0);
        wait();
        wait(SC_ZERO_TIME);
        value_to_save = act_data_io->read();
        write_mem(value_to_save, neuron_addr_link[device_to_save]);
    }
}

void ControlUnit::process()
{
    while (1)
    {
        while (state != PROCESS)
        {
            wait();
        }
        while (local_layer != neuron_layer_size.size() - 1)
        {
            size_t device_to_start = slave_count;
                for (size_t i = 0; i < slave_count; ++i)
                    if (!device_busy[i])
                        device_to_start = i;
            if (device_to_start != slave_count)
            {
                data_cnt_o[device_to_start]->write(neuron_layer_size[local_layer]);
                w_start_addr_o[device_to_start]->write(weight_local_addr);
                v_start_addr_o[device_to_start]->write(value_local_addr);
                load_o[device_to_start]->write(1);

                neuron_addr_link[device_to_start] = value_local_addr + neuron_layer_size[local_layer] + local_neuron;

                weight_local_addr += neuron_layer_size[local_layer];

                if (++local_neuron == neuron_layer_size[local_layer + 1])
                {
                    value_local_addr += neuron_layer_size[local_layer];
                    local_neuron = 0;
                    ++local_layer;
                }

                wait();
                ++device_load;
                device_busy[device_to_start] = true;
                wait();
                load_o[device_to_start]->write(0);
                
            }

            wait();

            process_mem_mapped();

            wait();
        }

        while (device_load)
        {
            process_mem_mapped();
            wait();
        }

        state = RESULT;
    }
}

void ControlUnit::result()
{
    while (1)
    {
        ioc_rd_o->write(0);
        while (state != RESULT)
            wait();
        ioc_res_addr_o->write(value_local_addr);
        ioc_rd_o->write(1);
        wait();
        while (ioc_busy_i)
            wait();
    }
}

void ControlUnit::read_mem_mapped()
{
    while (1)
    {
        for (size_t i = 0; i < slave_count; ++i)
            if (core_wr_i[i]->read()) {
                wait();
                mem_mapped.push({i, core_data_io[i]->read()});
            }
        wait();
    }
}

void ControlUnit::free_device()
{
    while (1)
    {
        for (size_t i = 0; i < slave_count; ++i)
            if (!core_busy_i[i]->read() && device_busy[i])
            {
                device_busy[i] = false;
                --device_load;
            }
        wait();
    }
}