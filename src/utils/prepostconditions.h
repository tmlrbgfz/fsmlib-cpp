#ifndef __FSMLIB_UTILS_PREPOSTCONDITIONS_H__
#define __FSMLIB_UTILS_PREPOSTCONDITIONS_H__

/* Mostly adopted from the Microsoft GSL 
 * https://github.com/Microsoft/GSL/blob/master/include/gsl/gsl_assert
 */

#include <exception>
#include <stdexcept>

class ContractViolation : public std::logic_error {
public:
    explicit ContractViolation(char const * const message)
        : std::logic_error(message) {
        }
};

#if defined(__clang__) || defined(__GNUC__)
#define FSMLIB_UTILS_PREPOSTCONDITIONS_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define FSMLIB_UTILS_PREPOSTCONDITIONS_LIKELY(x) (!!(x))
#endif

#define FSMLIB_UTILS_STRINGIFY_HELPER(x) #x
#define FSMLIB_UTILS_STRINGIFY(x) FSMLIB_UTILS_STRINGIFY_HELPER(x)

#define Expects(x) (FSMLIB_UTILS_PREPOSTCONDITIONS_LIKELY(x) ? static_cast<void>(0) : throw ContractViolation("Precondition failure at " __FILE__ ":" FSMLIB_UTILS_STRINGIFY(__LINE__)))
#define Ensures(x) (FSMLIB_UTILS_PREPOSTCONDITIONS_LIKELY(x) ? static_cast<void>(0) : throw ContractViolation("Postcondition failure at " __FILE__ ":" FSMLIB_UTILS_STRINGIFY(__LINE__)))

#endif //__FSMLIB_UTILS_PREPOSTCONDITIONS_H__