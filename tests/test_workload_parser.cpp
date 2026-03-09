#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include "accelsim/core/workload_runner.hpp"

TEST(WorkloadParser, ParsesDepsAndIds) {
    namespace fs = std::filesystem;
    const auto path = fs::temp_directory_path() / "accelsim_parser_test.csv";
    std::ofstream out(path.string());
    out << "id,op_type,latency,memory_bytes,deps\n";
    out << "1,COMPUTE,3,0,\n";
    out << "2,LOAD,8,64,1\n";
    out << "3,STORE,6,64,1|2\n";
    out.close();

    accelsim::SimulatorConfig cfg;
    const auto workload = accelsim::load_workload_csv(path.string(), &cfg);

    ASSERT_EQ(workload.size(), 3u);
    EXPECT_EQ(cfg.workload_name, "accelsim_parser_test");
    EXPECT_EQ(workload[2].deps.size(), 2u);
}
