#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "order.hpp"
#include "pace_graph.hpp"
#include <csignal>
#include <iostream>

// TODO make class singleton so there can only exist one solver at a time
class Solver {
  private:
    // Necessary to access the current solver instance when we handle SIGTERM
    // signals
    static Solver * instance;

    // Helper function to drop the int parameter of the signal handler
    // and output the resulting Order
    static void signal_handler(int signal_num) {
        if (Solver::instance) {
            Order result = Solver::instance->terminate();
            // TODO: Properly output the order as expected by the verifier
            std::cout << result.convert_to_real_node_id(*Solver::instance->graph) << std::endl;
            exit(0);
        }
        exit(signal_num);
    }

  public:
    virtual Order terminate() = 0;
    virtual Order solve() = 0;

    // TODO: Is this annoying to be used as a pointer or fine?
    //       We could copy the graph into the class at the beginning
    PaceGraph *graph;

    explicit Solver(PaceGraph &graph) {
        Solver::instance = this;
        std::cout << Solver::instance << std::endl;
        this->graph = &graph;
        signal(SIGTERM, signal_handler);
    };

    ~Solver() { signal(SIGTERM, SIG_DFL); }
};

Solver* Solver::instance = nullptr;

#endif // SOLVER_HPP
