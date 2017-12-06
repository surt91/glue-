#include "Cmd.hpp"

/** Constructs the command line parser, given argc and argv.
 */
Cmd::Cmd(int argc, char** argv)
{
    for(int i=0; i<argc; ++i)
    {
        text += argv[i];
        text += " ";
    }

    std::string version = VERSION " (Compiled: " __DATE__ " " __TIME__ ")";

    // TCLAP throws exceptions
    try{
        // Usage, delimiter, version
        TCLAP::CmdLine cmd("reads data from a column, creates a histogram per file and merges the histograms of multiple files", ' ', version);

        // value argument
        // -short, --long, description, required, default, type
        TCLAP::MultiArg<std::string> dataPathArg("i", "input", "name of a data file", false, "string", cmd);
        TCLAP::MultiArg<std::string> borderPathArg("b", "borderfiles", "files to determine the border from", false, "string", cmd);
        TCLAP::ValueArg<std::string> outputArg("o", "output", "name of the file for the resulting histogram", false, "", "string", cmd);
        TCLAP::ValueArg<std::string> logfileArg("L", "logfile", "log to file", false, "", "string", cmd);
        TCLAP::ValueArg<int> verboseArg("v", "verbose", "verbosity level:\n"
                                                        "\tquiet  : 0\n"
                                                        "\talways : 1\n"
                                                        "\terror  : 2\n"
                                                        "\twarning: 3\n"
                                                        "\tinfo   : 4 (default)\n"
                                                        "\tdebug  : 5\n"
                                                        "\tdebug2 : 6\n"
                                                        "\tdebug3 : 7",
                                        false, 4, "integer", cmd);
        TCLAP::ValueArg<double> upperArg("u", "upper", "upper bound", false, -1, "double", cmd);
        TCLAP::ValueArg<double> lowerArg("l", "lower", "lower bound", false, -1, "double", cmd);
        TCLAP::ValueArg<int> numBinsArg("B", "bins", "number of bins", false, 100, "int", cmd);
        TCLAP::MultiArg<double> thetaArg("T", "theta", "temperatures corresponding to the input files", false, "double", cmd);
        TCLAP::ValueArg<int> columnArg("c", "column", "in which column is the data", false, 0, "int", cmd);
        TCLAP::ValueArg<int> skipArg("s", "skip", "how many lines to skip", false, 0, "int", cmd);
        TCLAP::ValueArg<int> stepArg("S", "step", "read only every nth line", false, 0, "int", cmd);
        TCLAP::ValueArg<int> thresholdArg("t", "threshold", "number of entries in bin to use for glueing", false, 100, "int", cmd);
        TCLAP::ValueArg<int> parallelArg("p", "parallel", "how many omp threads to use", false, 0, "int", cmd);

        // switch argument
        // -short, --long, description, default
        TCLAP::SwitchArg bootstrapSwitch("", "bootstrap", "perform bootstrapping to estimate errors of the bins", cmd, false);
        TCLAP::SwitchArg forceSwitch("f", "force", "forces the reevaluation of the raw data", cmd, false);
        TCLAP::SwitchArg quietSwitch("q", "quiet", "quiet mode, log only to file (if specified) and not to stdout", cmd, false);

        // Parse the argv array.
        cmd.parse(argc, argv);


        Logger::verbosity = 4;

        // Get the value parsed by each arg.
        Logger::quiet = quietSwitch.getValue();
        Logger::verbosity = verboseArg.getValue();
        Logger::logfilename = logfileArg.getValue();

        // do not log, if there is no way to output
        if(Logger::quiet && Logger::logfilename.empty())
            Logger::verbosity = 0;

        LOG(LOG_INFO) << text;
        LOG(LOG_INFO) << "Verbosity                  " << Logger::verbosity;

        LOG(LOG_INFO) << "Version: " << VERSION;
        LOG(LOG_INFO) << "Compiled: " << __DATE__ << " " << __TIME__;

        LOG(LOG_INFO) << "Logging to " << Logger::logfilename;

        force = forceSwitch.getValue();
        LOG(LOG_INFO) << "force reread               " << force;

        bootstrap = bootstrapSwitch.getValue();
        LOG(LOG_INFO) << "bootstrap                  " << bootstrap;

        parallel = parallelArg.getValue();
        if(parallel)
            omp_set_num_threads(parallel);
        LOG(LOG_INFO) << "use parallel threads:      " << omp_get_num_threads();

        upperBound = upperArg.getValue();
        lowerBound = lowerArg.getValue();
        num_bins = numBinsArg.getValue();
        LOG(LOG_INFO) << "range               [" << lowerBound << ":" << upperBound << "]";
        LOG(LOG_INFO) << "num bins                   " << num_bins;

        column = columnArg.getValue();
        LOG(LOG_INFO) << "column                     " << column;
        skip = skipArg.getValue();
        LOG(LOG_INFO) << "skip                       " << skip;
        step = stepArg.getValue();
        LOG(LOG_INFO) << "step                       " << step;
        threshold = stepArg.getValue();
        LOG(LOG_INFO) << "threshold                  " << threshold;

        data_path_vector = dataPathArg.getValue();
        thetas = thetaArg.getValue();
        if(data_path_vector.size()  == 0)
        {
            LOG(LOG_ERROR) << "You need at least one input file";
            exit(2);
        }
        if(data_path_vector.size() > 1 && data_path_vector.size() != thetas.size())
        {
            LOG(LOG_ERROR) << "You need at a temperature (theta) value for each input file";
            exit(3);
        }
        LOG(LOG_INFO) << "Paths to read the data from: {";
        for(size_t j=0; j<data_path_vector.size(); ++j)
        {
            if(data_path_vector.size() > 1)
            {
                LOG(LOG_INFO) << "  " << thetas[j] << ": " << data_path_vector[j] << ",";
            }
            else
            {
                LOG(LOG_INFO) << "  " << data_path_vector[j];
            }
            std::ifstream is(data_path_vector[j].c_str());
            if(!is.good())
            {
                LOG(LOG_ERROR) << "Can not read " << data_path_vector[j];
                exit(3);
            }
        }
        LOG(LOG_INFO) << "}";

        border_path_vector = borderPathArg.getValue();
        LOG(LOG_INFO) << "Paths to get the borders from {" << border_path_vector << "}";

        output = outputArg.getValue();
        if(output != "" && output != "-")
        {
            std::ofstream os(output);
            if(!os.good())
            {
                LOG(LOG_ERROR) << "Can not write " << output;
                exit(4);
            }
        }
        LOG(LOG_INFO) << "target path                " << output;
    }
    catch(TCLAP::ArgException &e)  // catch any exceptions
    {
        LOG(LOG_ERROR) << e.error() << " for arg " << e.argId();
        std::cerr << e.error() << " for arg " << e.argId();
    }
}
