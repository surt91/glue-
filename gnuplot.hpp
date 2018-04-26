#pragma once

#include <string>
#include <vector>

#include "Cmd.hpp"
#include "fileOp.hpp"


struct GnuplotData {
    std::string hist_name;
    std::string corrected_name;
    std::string glued_name;
    std::string finished_name;
    std::string gnuplot_name;
    std::vector<std::string> raw_names;
    std::vector<double> temperatures;

    int column;

    GnuplotData()
        : hist_name("hist.dat"),
          corrected_name("corrected.dat"),
          glued_name("glued.dat"),
          finished_name("finished.dat"),
          gnuplot_name("plot.gp"),
          column(1)
    {
    }

    GnuplotData(const Cmd &o)
        : hist_name("hist.dat"),
          corrected_name("corrected.dat"),
          glued_name("glued.dat"),
          finished_name("finished.dat"),
          gnuplot_name(o.output + ".gp"),
          raw_names(o.data_path_vector),
          temperatures(o.thetas),
          column(o.column)
    {
    }
};

void write_gnuplot_quality(const GnuplotData &gp);
