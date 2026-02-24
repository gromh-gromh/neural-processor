#include "Processor.hpp"

Processor::Processor(sc_module_name nm)
    : sc_module(nm),
    clk_i("processor_clk"),
    pe_activation("act"),
    control_unit("cu"),
    io_controller("ioc"),
    memory("lm"),
    pe_multiplication("nc", slave_count),
    addr(local_mem_slave_count),
    data(local_mem_slave_count),
    wr(local_mem_slave_count),
    rd(local_mem_slave_count),
    data_cnt(slave_count),
    w_start_addr(slave_count),
    v_start_addr(slave_count),
    load(slave_count),
    busy(slave_count)
{
    for (size_t i = 0; i < slave_count; ++i) {
        pe_multiplication[i].clk_i(clk_i);
    }

    pe_activation.clk_i(clk_i);
    control_unit.clk_i(clk_i);
    io_controller.clk_i(clk_i);
    memory.clk_i(clk_i);

    io_controller.addr_o(addr[0]);
    control_unit.addr_o(addr[0]);
    memory.addr_i(addr[0]);

    io_controller.data_io(data[0]);
    control_unit.data_io(data[0]);
    memory.data_io(data[0]);

    io_controller.wr_o(wr[0]);
    control_unit.wr_o(wr[0]);
    memory.wr_i(wr[0]);

    io_controller.rd_o(rd[0]);
    control_unit.rd_o(rd[0]);
    memory.rd_i(rd[0]);

    control_unit.ioc_res_addr_o(ioc_res_addr);
    io_controller.ioc_res_addr_i(ioc_res_addr);
    control_unit.ioc_busy_i(ioc_busy);
    io_controller.ioc_busy_o(ioc_busy);
    control_unit.ioc_wr_o(ioc_wr);
    io_controller.ioc_wr_i(ioc_wr);
    control_unit.ioc_rd_o(ioc_rd);
    io_controller.ioc_rd_i(ioc_rd);

    pe_activation.act_data_io(act_data);
    control_unit.act_data_io(act_data);
    pe_activation.act_start_i(act_start);
    control_unit.act_start_o(act_start);

    for (size_t i = 0; i < slave_count; ++i) {
        pe_multiplication[i].data_cnt_i(data_cnt[i]);
        control_unit.data_cnt_o(data_cnt[i]);

        pe_multiplication[i].w_start_addr_i(w_start_addr[i]);
        control_unit.w_start_addr_o(w_start_addr[i]);

        pe_multiplication[i].v_start_addr_i(v_start_addr[i]);
        control_unit.v_start_addr_o(v_start_addr[i]);

        pe_multiplication[i].load_i(load[i]);
        control_unit.load_o(load[i]);

        pe_multiplication[i].busy_o(busy[i]);
        control_unit.core_busy_i(busy[i]);

        pe_multiplication[i].addr_o(addr[i + 1]);
        memory.addr_i(addr[i + 1]);

        pe_multiplication[i].data_io(data[i + 1]);
        memory.data_io(data[i + 1]);

        control_unit.core_data_io(data[i + 1]);

        pe_multiplication[i].wr_o(wr[i + 1]);

        control_unit.core_wr_i(wr[i + 1]);

        pe_multiplication[i].rd_o(rd[i + 1]);
        memory.rd_i(rd[i + 1]);
    }
    
    SC_METHOD(start);
}

void Processor::start()
{
    control_unit.start();
}

void Processor::print_all_stats() const
{
    std::cout << "\n=========================================" << std::endl;
    std::cout << "PROCESSOR CORES UTILIZATION STATISTICS" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    unsigned long long total_ops_all = 0;
    double total_utilization = 0;
    
    for (size_t i = 0; i < slave_count; ++i) {
        std::cout << "\nCore " << i << ":" << std::endl;
        pe_multiplication[i].print_stats();
        total_ops_all += pe_multiplication[i].total_operations;
        
        unsigned long long total_cycles = pe_multiplication[i].total_cycles_active + 
                                         pe_multiplication[i].total_cycles_idle;
        if (total_cycles > 0) {
            total_utilization += (double)pe_multiplication[i].total_cycles_active / total_cycles;
        }
    }
    
    std::cout << "\n--- Overall Statistics ---" << std::endl;
    std::cout << "Total multiplication operations across all cores: " << total_ops_all << std::endl;
    if (slave_count > 0) {
        std::cout << "Average core utilization: " << (total_utilization / slave_count * 100.0) << "%" << std::endl;
    }
    
    std::cout << "=========================================" << std::endl;
}

void Processor::reset_all_stats()
{
    for (size_t i = 0; i < slave_count; ++i) {
        pe_multiplication[i].reset_stats();
    }
}
