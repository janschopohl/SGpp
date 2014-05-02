#include "opt/sle/SolverArmadillo.hpp"
#include "opt/tools/Printer.hpp"

#ifdef USEARMADILLO
#include <armadillo>
#endif

#include <cstddef>
#include <iostream>

namespace sg
{
namespace opt
{
namespace sle
{

#ifdef USEARMADILLO
bool solveInternal(const arma::mat &A, const std::vector<double> &b, std::vector<double> &x)
{
    arma::uword n = static_cast<arma::uword>(b.size());
    arma::vec b_armadillo(b);
    arma::vec x_armadillo(n);
    
    tools::printer.printStatusUpdate("solving with Armadillo");
    
    if (arma::solve(x_armadillo, A, b_armadillo))
    {
        x = arma::conv_to<std::vector<double>>::from(x_armadillo);
        return true;
    } else
    {
        return false;
    }
}
#endif

bool SolverArmadillo::solve(System &system, std::vector<double> &x) const
{
#ifdef USEARMADILLO
    tools::printer.printStatusBegin("Solving linear system (Armadillo)...");
    
    arma::uword n = static_cast<arma::uword>(system.getDimension());
    const std::vector<double> &b = system.getRHS();
    arma::mat A(n, n);
    size_t nnz = 0;
    
    A.zeros();
    
    //#pragma omp parallel for ordered schedule(dynamic)
    for (arma::uword i = 0; i < n; i++)
    {
        for (arma::uword j = 0; j < n; j++)
        {
            double entry = system.getMatrixEntry(i, j);
            
            //#pragma omp critical
            A(i,j) = entry;
            
            if (A(i,j) != 0)
            {
                //#pragma omp atomic
                nnz++;
            }
        }
        
        if (i % 100 == 0)
        {
            //#pragma omp ordered
            {
                char str[10];
                snprintf(str, 10, "%.1f%%",
                         static_cast<double>(i) / static_cast<double>(n) * 100.0);
                tools::printer.printStatusUpdate("constructing matrix (" + std::string(str) + ")");
            }
        }
    }
    
    tools::printer.printStatusUpdate("constructing matrix (100.0%)");
    tools::printer.printStatusNewLine();
    
    {
        char str[10];
        double nnz_ratio = static_cast<double>(nnz) /
                           (static_cast<double>(n) * static_cast<double>(n));
        snprintf(str, 10, "%.1f%%", nnz_ratio * 100.0);
        tools::printer.printStatusUpdate("nnz ratio: " + std::string(str));
        tools::printer.printStatusNewLine();
    }
    
    bool result = solveInternal(A, b, x);
    
    if (result)
    {
        tools::printer.printStatusEnd();
        return true;
    } else
    {
        tools::printer.printStatusEnd("error: could not solve linear system!");
        return false;
    }
#else
    std::cerr << "Error in sg::opt::sle::SolverArmadillo::solve: "
              << "SG++ was compiled without Armadillo support!\n";
    return false;
#endif
}

/*bool SolverArmadillo::solve(const std::vector<uint32_t> &Ti, const std::vector<uint32_t> &Tj,
                            const std::vector<double> &Tx, const std::vector<double> &b,
                            std::vector<double> &x) const
{
#ifdef USEARMADILLO
    tools::printer.printStatusBegin("Solving sparse linear system (Armadillo)...");
    
    size_t nnz = Tx.size();
    arma::uword n = static_cast<arma::uword>(b.size());
    arma::mat A(n, n);
    
    A.zeros();
    
    for (size_t k = 0; k < nnz; k++)
    {
        if (k % 100 == 0)
        {
            char str[10];
            snprintf(str, 10, "%.1f%%",
                     static_cast<double>(k) / static_cast<double>(nnz) * 100.0);
            tools::printer.printStatusUpdate("constructing full matrix (" +
                                             std::string(str) + ")");
        }
        
        A(static_cast<arma::uword>(Ti[k]), static_cast<arma::uword>(Tj[k])) = Tx[k];
    }
    
    tools::printer.printStatusUpdate("constructing full matrix (100.0%)");
    tools::printer.printStatusNewLine();
    
    bool result = solveInternal(A, b, x);
    
    if (result)
    {
        tools::printer.printStatusEnd();
        return true;
    } else
    {
        tools::printer.printStatusEnd("error: could not solve linear system!");
        return false;
    }
#else
    std::cerr << "Error in sg::opt::sle::SolverArmadillo::solve: "
              << "SG++ was compiled without Armadillo support!\n";
    return false;
#endif
}*/

}
}
}
