//
// Created by Lotte on 2024-04-08.
//
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include "../pace_graph/order.hpp"
#include "heuristic.hpp"
#include <vector>

#ifndef PACE2024_GENETIC_ALGO_HPP
#define PACE2024_GENETIC_ALGO_HPP

std::vector<std::vector<int>>
createPopulationRandom(PaceGraph &graph, int populationSize,
                       std::vector<std::vector<int>> &pre_computed_orders);

std::vector<int> createRandomOrder(int number_of_free_vertices);

std::vector<std::vector<int>>
selection(PaceGraph &graph, int populationSize,
          std::vector<std::vector<int>> population);

std::vector<std::vector<int>> crossover(std::vector<int> &parent1,
                                        std::vector<int> &parent2);

long checkFitness(PaceGraph &graph, const std::vector<int> &current_order);

class GeneticAlgoSolver : Heuristic {
  public:
    explicit GeneticAlgoSolver(std::function<bool(int)> has_time_left)
        : Heuristic(std::move(has_time_left)) {}

    Order solve(PaceGraph &graph) override;

    static Order
    solveWithParameters(PaceGraph &graph, int itterations, int populationSize,
                        std::vector<std::vector<int>> &pre_computed_orders);
};

#endif // PACE2024_GENETIC_ALGO_HPP
