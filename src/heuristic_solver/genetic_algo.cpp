//
// Created by Lotte on 2024-04-08.
//

#include "genetic_algo.hpp"
#include <algorithm>
#include <random>
#include <vector>

#include "../pace_graph/order.hpp"

Order GeneticAlgoSolver::solve(PaceGraph &graph) {
    std::vector<std::vector<int>> pre_computed_orders;
    return solveWithParameters(graph, 100, 50, pre_computed_orders);
}

Order GeneticAlgoSolver::solveWithParameters(
    PaceGraph &graph, int iterations, int population_size,
    std::vector<std::vector<int>> &precomputed_orders) {

    // create a start population
    std::vector<std::vector<int>> population =
        createPopulationRandom(graph, population_size, precomputed_orders);

    // create new generations
    for (int i = 0; i < iterations; ++i) {

        // crossover
        std::vector<std::vector<int>> old_population = population;
        std::random_device rd;
        auto rng = std::default_random_engine{rd()};
        std::shuffle(begin(old_population), end(old_population), rng);
        while (old_population.size() > 1) {
            std::vector<std::vector<int>> children =
                crossover(old_population[0], old_population[1]);
            old_population.erase(old_population.cbegin());
            old_population.erase(old_population.cbegin());

            for (std::vector<int> &child : children) {
                // here would be the right place for mutation
                population.push_back(child);
            }
        }

        // selection
        population = selection(graph, population_size, population);
    }

    // find best order
    std::vector<int> best_order;
    long best_score = 0;
    for (const std::vector<int> &order : population) {
        long current_score = checkFitness(graph, order);
        if (current_score > best_score) {
            best_score = current_score;
            best_order = order;
        }
    }
    return Order(best_order);
}

std::vector<std::vector<int>>
createPopulationRandom(PaceGraph &graph, int populationSize,
                       std::vector<std::vector<int>> &pre_computed_orders) {
    std::vector<std::vector<int>> population;
    for (const std::vector<int> &order : pre_computed_orders) {
        population.push_back(order);
    }

    while (population.size() < populationSize) {
        population.push_back(createRandomOrder(graph.size()));
    }
    return population;
}

std::vector<int> createRandomOrder(int number_of_free_vertices) {
    std::random_device rd;
    auto rng = std::default_random_engine{rd()};

    // initial order
    std::vector<int> order;
    order.reserve(number_of_free_vertices);

    for (int i = 0; i < number_of_free_vertices; ++i) {
        order.push_back(i);
    }

    // shuffle vertices
    std::shuffle(begin(order), end(order), rng);

    return order;
}

long checkFitness(PaceGraph &graph, const std::vector<int> &current_order) {
    Order order(current_order);

    // my guess
    return 1 / order.count_crossings(graph);

    // based on paper by Pascale Kuntz, Bruno Pinaud, Rémi Lehn
    return pow(2, -order.count_crossings(graph));
}

std::vector<std::vector<int>>
selection(PaceGraph &graph, int goalPopulationSize,
          std::vector<std::vector<int>> &population) {
    std::vector<std::vector<int>> new_population;

    // tournament style
    // rounds up
    int size_of_tournament =
        (population.size() + goalPopulationSize - 1) / goalPopulationSize;

    // shuffle population
    std::random_device rd;
    auto rng = std::default_random_engine{rd()};
    std::shuffle(begin(population), end(population), rng);

    for (int i = 0; i < goalPopulationSize; ++i) {

        // create the set of vertices where the best one is selected for the
        // next generation
        std::vector<std::vector<int>> fight_set;
        fight_set.reserve(size_of_tournament);
        for (int j = 0; j < size_of_tournament; ++j) {
            if (!population.empty()) {
                fight_set.push_back(population.front());
                population.erase(population.begin());
            }
        }

        long best_score = 0;
        std::vector<int> best_order;
        for (const std::vector<int> &order : fight_set) {
            long current_score = checkFitness(graph, order);
            if (current_score > best_score) {
                best_score = current_score;
                best_order = order;
            }
        }
        new_population.push_back(best_order);
    }
    return new_population;
}

std::vector<std::vector<int>> crossover(std::vector<int> &parent1,
                                        std::vector<int> &parent2) {

    // we cut both parents at some position and then exchange them (with
    // some tricks to still get a valid solution)
    std::random_device rd;  // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(
        1, int(parent1.size() - 1)); // define the range

    int cut_at = distr(gen);

    std::vector<int> child1(parent1.begin(), parent1.begin() + cut_at);
    std::vector<int> child2(parent2.begin(), parent2.begin() + cut_at);

    for (int i = cut_at; i < parent1.size(); ++i) {

        int index = i;
        auto j = std::find(child1.begin(), child1.end(), parent2[index]);
        while (j != child1.end()) {
            // auto l = std::find(child1.begin(), child1.end(), parent2[i]);
            index = j - child1.begin();
            j = std::find(child1.begin(), child1.end(), parent2[index]);
        }
        child1.push_back(parent2[index]);
    }

    // same code just for the second child

    for (int i = cut_at; i < parent2.size(); ++i) {

        int index = i;
        auto j = std::find(child2.begin(), child2.end(), parent1[index]);
        while (j != child2.end()) {
            // auto l = std::find(child1.begin(), child1.end(), parent2[i]);
            index = j - child2.begin();
            j = std::find(child2.begin(), child2.end(), parent1[index]);
        }
        child2.push_back(parent1[index]);
    }
    // add children to population

    return std::vector<std::vector<int>>{child1, child2};
}

void mutation() {}
