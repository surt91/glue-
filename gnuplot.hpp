#pragma once

#include <string>
#include <vector>

#include "Cmd.hpp"
#include "fileOp.hpp"


struct GnuplotData {
    std::string hist_name;
    std::string corrected_name;
    std::string glued_name;
    std::string gnuplot_name;
    std::vector<std::string> raw_names;

    GnuplotData()
        : hist_name("hist.dat"),
          corrected_name("corrected.dat"),
          glued_name("glued.dat"),
          gnuplot_name("plot.gp")
    {
    }

    GnuplotData(const Cmd &o)
        : hist_name("hist.dat"),
          corrected_name("corrected.dat"),
          glued_name("glued.dat"),
          gnuplot_name(o.output + ".gp"),
          raw_names(o.data_path_vector)
    {
    }
};

void write_gnuplot_quality(const GnuplotData &gp);
