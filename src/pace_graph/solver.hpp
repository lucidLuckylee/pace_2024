#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "order.hpp"
#include "pace_graph.hpp"
#include <csignal>

class Solver {
  private:
    /*
     * Necessary to access the current solver instance in the signal_handler
     */
    static Solver *instance;

    /*
     * Helper function to drop the int parameter of the signal handler
     * and output the resulting Order that calls the terminate() function
     */
    static void signal_handler(int signal_num);

  public:
    /* Terminate the current solve function. Should implement the generation of
     * the resulting Order so the signal_handler can call it. For this the Order
     * has to be created from Class member variables. They should be as
     * consistent as possible and atomically updated. One can use sigprocmask to
     * mask the SIGTERM signal.
     */
    virtual Order terminate() = 0;
    virtual Order solve() = 0;

    PaceGraph *graph;

    explicit Solver(PaceGraph &graph);

    ~Solver() { std::signal(SIGTERM, SIG_DFL); }
};

#endif // SOLVER_HPP
