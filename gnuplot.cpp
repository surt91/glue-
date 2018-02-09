#include "gnuplot.hpp"

/** Write a gnuplot script visualizing the output for quality assesment.
 *
 *  \param gp   struct containing all relevant filenames
 */
void write_gnuplot_quality(const GnuplotData &gp)
{
    std::ofstream os(gp.gnuplot_name);
    os << "set output '" << (gp.gnuplot_name + ".png") << "'\n";
    os << "set terminal pngcairo size 1920,1440\n";
    os << "set multiplot layout 3, 2\n";
    os << "unset key\n";
    os << "set palette defined ( 0 'green', 1 'blue', 2 'red', 3 'orange' )\n";

    os << "set cbtics (";
    for(size_t i=0; i<gp.temperatures.size(); ++i)
    {
        double T = gp.temperatures[i];
        if(T>1000)
            os << "\"∞\" " << i << ", ";
        else
            os << "\"" << T << "\" " << i << ", ";
    }
    os << ")\n\n";

    // raw data plot
    os << "set title 'raw data'\n";
    os << "set xl 't'\n";
    os << "set yl 's'\n";
    os << "p ";
    int idx = 0;
    for(auto &s : gp.raw_names)
    {
        if(has_suffix(s, ".gz"))
            os << "'< zcat " << s << "' every 100 u 0:1:(" << idx << ") w l palette, \\\n";
        else
            os << "'" << s << "' every 100 u 0:1:(" << idx << ") w l palette, \\\n";
        ++idx;
    }
    os << "\n\n";

    // raw histogram plot
    os << "set title 'raw histograms'\n";
    os << "set xl 's'\n";
    os << "set yl '#'\n";
    os << "p '" << gp.hist_name << "' u 1:2:-1 w p palette\n\n";

    // raw histogram plot
    os << "set title 'raw logarithmic histograms'\n";
    os << "set xl 's'\n";
    os << "set yl 'P_T(s)'\n";
    os << "set log y\n";
    os << "p '" << gp.hist_name << "' u 1:2:-1 w p palette\n";
    os << "unset log\n\n";

    os << "set format y '10^{%.0f}'\n";

    // corrected histogram plot
    os << "set title 'raw corrected histograms'\n";
    os << "set xl 's'\n";
    os << "set yl 'e^{s/T} P_T(s)'\n";
    os << "p '" << gp.corrected_name << "' u 1:($2/log(10)):-1 w p palette\n\n";

    // glued histogram plot
    os << "set title 'unnormalized glued data'\n";
    os << "set xl 's'\n";
    os << "set yl 'a e^{s/T} Z(T) P_T(s)'\n";
    os << "p '" << gp.glued_name << "' u 1:($2/log(10)):-1 w p palette\n\n";

    // glued histogram plot
    os << "set title 'normalized data'\n";
    os << "set xl 's'\n";
    os << "set yl 'P(s)'\n";
    os << "p '" << gp.finished_name << "' u 1:($2/log(10))\n\n";

    os << "unset multiplot\n";

    os.close();
    LOG(LOG_INFO) << "plotting quality plot";
    system(("gnuplot " + gp.gnuplot_name).c_str());
}
