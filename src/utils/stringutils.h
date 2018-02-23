#ifndef __FSMLIB_UTILS_STRINGUTILS_H__
#define __FSMLIB_UTILS_STRINGUTILS_H__

#include <iterator>
#include <numeric>

template<class Iterator>
std::string concatenate(Iterator begin, Iterator end, std::string const &separator) {
    Iterator second = begin;
    std::advance(second, 1);
    return std::accumulate(second, end, *begin, [&separator](std::string const &current, std::string const &value){
        return current + separator + value;
    });
}

#endif //__FSMLIB_UTILS_STRINGUTILS_H__