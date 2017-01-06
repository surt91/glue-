#pragma once

#include <string>
#include <sstream>

#include <tclap/CmdLine.h>

#include "Logging.hpp"

// test, if we are using openmp
#ifdef _OPENMP
   #include <omp.h>
#else
   #define omp_get_thread_num() 0
   #define omp_get_num_threads() 0
   #define omp_set_num_threads(x)
#endif

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
        std::vector<double> thetas;                   ///< temperatures of the files in the same order

        int column;                                   ///< in which column of the file is the interesting data
        int skip;                                     ///< how many lines to skip of the file (~ equilibration time)
        int step;                                     ///< only read every nth line (~ autocorrelation time)

        double lowerBound, upperBound;
        int num_bins;

        bool force;

        int parallel;
};
