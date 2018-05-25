#include "fileOp.hpp"

/** Gets the n-th word in the given String.
 *
 *  Words are separated by spaces, other whitespace does not work
 *
 * \param line  string, which should be splitted
 * \param n     which word
 * \return the n-th word as a string
 */
std::string getNthWord(const std::string &line, int n)
{
    std::string word;
    std::stringstream ss;
    ss.str(line);
    int ctr = 0;
    do
    {
        std::getline(ss, word, ' ');
        ++ctr;
    } while(ctr <= n);
    return word;
}

/** Tests if a given string ends with a given substring.
 *
 * see: http://stackoverflow.com/a/20446239/1698412
 *
 * \param str       string which should be tested
 * \param suffix    suffix for which to test
 * \return true, if str ends with suffix, otherwise false
 */
bool has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/** Wrapper function to write some string into a file given by a string.
 *  If the file is either an empty string or -, write to stdout.
 *
 * \param file  file to write to
 * \param text  text to write into the file
 */
void write_out(std::string file, std::string text)
{
    if(file == "" || file == "-")
        std::cout << text;
    else
    {
        std::ofstream os(file, std::ios::app);
        os << text;
    }
}

/** tests if the given file contains a histogram, i.e. is too short for
 * a datafile.
 *
 * this is suboptimal and a dirty hack that needs to die
 */
bool isHistogramFile(std::string filename)
{
    igzstream is(filename.c_str());
    std::string item;

    int ctr = 0;
    while(std::getline(is, item) && ctr < 5)
    {
        if(item[0] != '#' && item[0] != '\n')
            ++ctr;
    }

    return ctr < 4;
}

bool fileReadable(std::string filename)
{
    std::ifstream is(filename.c_str());
    LOG(LOG_DEBUG) << filename << " " << is.good();
    return is.good();
}
