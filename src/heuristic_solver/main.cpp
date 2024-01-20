// heuristic_solver/main.cpp
#include "../pace_graph/pace_graph.hpp"
#include <iostream>

int main() {
  try {
    // Example: Creating a PaceGraph from a file
    PaceGraph graphFromFile = PaceGraph::from_file("../data/website_20.gr");
    
    std::cout << "PaceGraph objects created successfully." << std::endl;
    std::cout << "Graph in gr format: " << std::endl;
    std::cout << graphFromFile.to_gr() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1; // Return a non-zero value to indicate an error
  }

  return 0;
}
