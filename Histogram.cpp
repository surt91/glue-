#include "Histogram.hpp"

Histogram::Histogram()
    : m_cur_min(0),
      m_total(0),
      m_sum(0),
      above(0),
      below(0)
{
}

Histogram::Histogram(const int num_bins, const double lower, const double upper)
    : num_bins(num_bins),
      lower(lower),
      upper(upper),
      m_cur_min(0),
      m_total(0),
      m_sum(0),
      above(0),
      below(0),
      data(num_bins, 0)
{
    double binwidth = (upper - lower) / num_bins;
    bins.reserve(num_bins);
    for(int i=0; i<num_bins; ++i)
        bins.emplace_back(lower + i*binwidth);
    bins.emplace_back(upper);
}

Histogram::Histogram(const std::vector<double> bins)
    : num_bins(bins.size()-1),
      lower(bins.front()),
      upper(bins.back()),
      m_cur_min(0),
      m_total(0),
      m_sum(0),
      above(0),
      below(0),
      bins(bins),
      data(num_bins, 0)
{
}

Histogram::Histogram(const std::string filename)
    : m_cur_min(0),
      m_total(0),
      m_sum(0),
      above(0),
      below(0)
{
    readFromFile(filename);
}

/** Adds an entry to the corresponding bin.
 *
 * \param where A value for which the corresponding bin is updated
 * \param what  The value by which the bin should be updated (default 1)
 */
void Histogram::add(double where, double what)
{
    if(where >= upper)
    {
        above += what;
        return;
    }
    if(where < lower)
    {
        below += what;
        return;
    }

    int idx = std::upper_bound(bins.begin(), bins.end(), where) - bins.begin();
    --idx;

    double tmp = data[idx];
    data[idx] += what;
    ++m_total;
    m_sum += what;

    // see if this is the current minimum and update, if necessary
    if(tmp == m_cur_min)
    {
        m_cur_min += what;
        for(int i=0; i<num_bins; ++i)
            if(data[i] < m_cur_min)
                m_cur_min = data[i];
    }
}

/// minimum value of all bins
int Histogram::get_num_bins() const
{
    return num_bins;
}

/// minimum value of all bins
int Histogram::min() const
{
    return m_cur_min;
}

/// mean value of all bins
double Histogram::mean() const
{
    return (double) m_sum / num_bins;
}

/// number of insertions into the histogram
int Histogram::count() const
{
    return m_total;
}

/// sum of all bins (equal to count, for standard histograms)
int Histogram::sum() const
{
    return m_sum;
}

/// sets all entries to zero and clears statistical data
void Histogram::reset()
{
    m_cur_min = 0;
    m_total = 0;
    m_sum = 0;
    for(int i=0; i<num_bins; ++i)
        data[i] = 0;
}

/** trim the histogram
 *
 * discard all bins left of the smallest without entries
 * and all bins right of the largest without entries
 */
void Histogram::trim()
{
    int left=0;
    int right=num_bins;

    int i=0;
    while(data[i] == 0)
    {
        ++i;
        left = i;
    }
    for(; i<num_bins; ++i)
        if(data[i] == 0)
        {
            right = i;
            break;
        }

    if(left == right)
    {
        LOG(LOG_ERROR) << "The Histogram is empty after trimming!";
    }

    lower = bins[left];
    upper = bins[right];
    num_bins = right-left;

    std::vector<double> new_bins(num_bins+1);
    std::vector<double> new_data(num_bins);
    for(int i=left, j=0; i<=right; ++i, ++j)
        new_bins[j] = bins[i];

    // minimum needs to be updated
    m_cur_min = data[left+1];
    for(int i=left, j=0; i<right; ++i, ++j)
    {
        new_data[j] = data[i];
        if(data[i] < m_cur_min)
            m_cur_min = data[i];
    }

    bins = new_bins;
    data = new_data;
}

double Histogram::operator[](const double value) const
{
    return operator[](value);
}

double& Histogram::operator[](const double value)
{
    if(value >= upper)
        return above;
    if(value < lower)
        return below;

    int idx = std::upper_bound(bins.begin(), bins.end(), value) - bins.begin();
    --idx;
    return data[idx];
}

double& Histogram::at(int idx)
{
    return data[idx];
}

/// vector of num_bins elements containing their centers
const std::vector<double> Histogram::centers() const
{
    std::vector<double> c;
    c.reserve(num_bins);
    for(int i=0; i<num_bins; ++i)
        c.emplace_back((bins[i] + bins[i+1]) / 2);
    return c;
}

/// vector of num_bins elements containing their data
const std::vector<double>& Histogram::get_data() const
{
    return data;
}

/// vector of of num_bins + 1 elements containing their borders
const std::vector<double>& Histogram::borders() const
{
    return bins;
}

/// a string with the data as ascii, two colums
const std::string Histogram::ascii_table() const
{
    std::stringstream ss;
    ss << ("# centers counts\n");
    auto c = centers();
    auto d = get_data();
    for(int i=0; i<num_bins; ++i)
        ss << c[i] << " " << d[i] << "\n";
    return ss.str();
}

/// save the histogram to a file, can be loaded by Histogram::readFromFile
void Histogram::writeToFile(const std::string filename) const
{
    std::ofstream os(filename);
    if(!os.good())
    {
        LOG(LOG_ERROR) << "can not write " << filename;
    }

    for(const auto &i : bins)
        os << i << " ";
    os << "\n";

    for(const auto &i : data)
        os << i << " ";
    os << "\n";
}

// load a histogram to a file, as saved by Histogram::writeToFile
void Histogram::readFromFile(const std::string filename)
{
    std::ifstream is(filename);
    if(!is.good())
    {
        LOG(LOG_ERROR) << "can not read " << filename;
    }

    // the format is pretty strict: 2 lines, in the first borders
    // separated by space, in the second counts separated by whitespace
    std::string lineBorders, lineData;
    std::getline(is, lineBorders);
    std::getline(is, lineData);

    std::string item;
    std::stringstream ss;

    ss.str(lineBorders);
    while (std::getline(ss, item, ' '))
        bins.push_back(std::stod(item));

    num_bins = bins.size()-1;
    lower = bins.front();
    upper = bins.back();
    ss. clear();

    ss.str(lineData);
    while (std::getline(ss, item, ' '))
        data.push_back(std::stod(item));

    if(bins.size() != data.size() + 1)
    {
        LOG(LOG_ERROR) << "expected one more bin border than data, "
                          "got " << bins.size() << " and " << data.size();
    }
}

std::ostream& operator<<(std::ostream& os, const Histogram &obj)
{
    os << "[";
    for(int i=0; i<obj.num_bins; ++i)
        os << "[" <<obj.bins[i] << " - " << obj.bins[i+1] << "] :" << obj.data[i] << std::endl;
    os << "] ";
    return os;
}
