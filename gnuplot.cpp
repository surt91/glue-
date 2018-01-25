#include "gnuplot.hpp"

// https://stackoverflow.com/a/2072890/1698412
bool ends_with(std::string const &value, std::string const &ending)
{
    if(ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

/** Write a gnuplot script visualizing the output for quality assesment.
 *
 *  \param gp   struct containing all relevant filenames
 */
void write_gnuplot_quality(const GnuplotData &gp)
{
    std::ofstream os(gp.gnuplot_name);
    os << "set output '" << (gp.gnuplot_name + ".png") << "'\n";
    os << "set terminal pngcairo size 1920,1080\n";
    os << "set multiplot layout 2, 2\n";
    os << "unset key\n";
    os << "set palette defined ( 0 'green', 1 'blue', 2 'red', 3 'orange' )\n\n";

    // raw data plot
    os << "set title 'raw data'\n";
    os << "p ";
    for(auto &s : gp.raw_names)
    {
        if(ends_with(s, ".gz"))
            os << "'< zcat " << s << "' every 100 w l, ";
        else
            os << "'" << s << "' every 100 w l, ";
    }
    os << "\n";

    // raw histogram plot
    os << "set title 'raw logarithmic histograms'\n";
    os << "p '" << gp.hist_name << "' u 1:2:-1 w l palette\n";

    // corrected histogram plot
    os << "set title 'raw corrected histograms'\n";
    os << "p '" << gp.corrected_name << "' u 1:2:-1 w p palette\n";

    // glued histogram plot
    os << "set title 'unnormalized glued data'\n";
    os << "p '" << gp.glued_name << "' u 1:2:-1 w p palette\n";

    os << "unset multiplot\n";

    os.close();
    LOG(LOG_INFO) << "plotting quality plot";
    system(("gnuplot " + gp.gnuplot_name).c_str());
}