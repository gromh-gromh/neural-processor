#include "./Processor/Processor.hpp"
#include <systemc.h>
#include <chrono>

int sc_main(int, char*[]) {
    sc_time start_time = sc_time_stamp();
    auto real_start = std::chrono::high_resolution_clock::now();
    
    sc_core::sc_clock clk("clk", sc_time(100, SC_NS));
    Processor processor("processor");
    processor.clk_i(clk);
    
    sc_start(200, sc_core::SC_SEC);
    
    sc_time end_time = sc_time_stamp();
    auto real_end = std::chrono::high_resolution_clock::now();
    
    sc_time sim_duration = end_time - start_time;
    auto real_duration = std::chrono::duration_cast<std::chrono::microseconds>
                        (real_end - real_start);
    
    cout << "=========================================" << endl;
    cout << "Simulation Time Profile:" << endl;
    cout << "  Simulated time: " << sim_duration << endl;
    cout << "  Simulated cycles: " << sim_duration / sc_time(100, SC_NS) << endl;
    cout << "=========================================" << endl;
    
    processor.print_all_stats();
    
    return 0;
}