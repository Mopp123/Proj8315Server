#include "StringUtil.h"


namespace str_util
{
    std::vector<std::string> split(const std::string str, const std::string delim)
    {
        std::vector<std::string> result;
        size_t pos = 0;
        size_t delimPos = 0;
        while ((delimPos = str.find(delim, pos)) != std::string::npos)
        {
            std::string s = str.substr(pos, delimPos - pos);
            pos = delimPos + delim.length();
            result.push_back(s);
        }
        // Also add the last
        std::string last = str.substr(pos, delimPos);
        if (!last.empty())
            result.push_back(last);
        return result;
    }
}
