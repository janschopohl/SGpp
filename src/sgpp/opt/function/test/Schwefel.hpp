#ifndef SGPP_OPT_FUNCTION_TEST_SCHWEFEL_HPP
#define SGPP_OPT_FUNCTION_TEST_SCHWEFEL_HPP

#include "opt/function/TestFunction.hpp"

namespace sg
{
namespace opt
{
namespace function
{
namespace test
{

class Schwefel : public TestFunction
{
public:
    Schwefel(size_t d) : TestFunction(d)
    {
    }
    
    double evalUndisplaced(const std::vector<double> &x)
    {
        double result = 0.0;
        
        for (size_t t = 0; t < d; t++)
        {
            double xt = 1000.0 * x[t] - 500.0;
            result -= xt * std::sin(std::sqrt(std::abs(xt)));
        }
        
        return result;
    }
    
    double getOptimalPointUndisplaced(std::vector<double> &x)
    {
        x = std::vector<double>(d, 0.9209687);
        //return -418.9829 * static_cast<double>(d);
        return evalUndisplaced(x);
    }
};

}
}
}
}

#endif
