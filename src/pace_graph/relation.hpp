#include <iostream>
#include <set>
#include <utility>

class PartialOrdering {
  private:
    std::set<std::pair<int, int>> pairs;

  public:
    // Add a pair to the relation
    void set_a_lt_b(int a, int b) { pairs.insert(std::make_pair(a, b)); }

    bool lt(int a, int b) {
        return pairs.find(std::make_pair(a, b)) != pairs.end();
    }

    bool incomparable(int a, int b) {
        return a != b && pairs.find(std::make_pair(a, b)) != pairs.end() &&
               pairs.find(std::make_pair(b, a)) != pairs.end();
    }

    bool dependent(int a, int b, int c) {
        return incomparable(a, c) || incomparable(b, c);
    }

    void remove(int x, int y) { pairs.erase(std::make_pair(x, y)); }

    void print() const {
        for (const auto &pair : pairs) {
            std::cout << "(" << pair.first << ", " << pair.second << ")"
                      << std::endl;
        }
    }
};
