#pragma once

#include <string>
#include <sstream>

#include <tclap/CmdLine.h>

#include "Logging.hpp"

/** Command line parser.
 *
 * This command line parser uses TCLAP (http://tclap.sourceforge.net/).
 */
class Cmd
{
    public:
        Cmd() {};
        Cmd(int argc, char** argv);

        std::string output;                           ///< output filename
        std::vector<std::string> data_path_vector;    ///< vector of input files
        std::vector<std::string> border_path_vector;  ///< vector of of input files used to determine the borders

        std::string text;                             ///< the full command used to start this program
        std::vector<double> thetas;                           ///< temperatures of the files in the same order

        double lowerBound, upperBound;
        int num_bins;
};
