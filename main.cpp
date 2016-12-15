#include <random>
#include <vector>
#include <algorithm>

#include "Cmd.hpp"
#include "fileOp.hpp"


int main(int argc, char** argv)
{
    Cmd o(argc, argv);

    std::cout << histogramFromFile(o.data_path_vector[0], o.num_bins, o.lowerBound, o.upperBound).ascii_table();
}
