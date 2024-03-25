#include <iostream>
#include <set>
#include <utility>

class PartialOrdering {
  private:
    std::set<std::pair<int, int>> pairs;

    void transitive_closure() {
        // Flag to indicate whether any new pairs were added in this iteration
        bool added_new_pairs = true;

        // Keep iterating until no new pairs can be added
        while (added_new_pairs) {
            added_new_pairs = false;
            for (const auto &pair1 : pairs) {
                for (const auto &pair2 : pairs) {
                    if (pair1.second == pair2.first) {
                        std::pair<int, int> new_pair(pair1.first, pair2.second);
                        // If the new pair is not already in the set, add it
                        if (pairs.find(new_pair) == pairs.end()) {
                            pairs.insert(new_pair);
                            added_new_pairs = true;
                        }
                    }
                }
            }
        }
    }

  public:
    /* Adds a pair to the relation
     * @return True if the pair got newly added. False if it already existed.
     */
    bool set_a_lt_b(int a, int b) {
        if (lt(a, b)) {
            return false;
        }
        pairs.insert(std::make_pair(a, b));
        // Add all transitive relations
        transitive_closure();
        return true;
    }

    bool lt(int a, int b) {
        return pairs.find(std::make_pair(a, b)) != pairs.end();
    }

    bool incomparable(int a, int b) {
        return a != b && pairs.find(std::make_pair(a, b)) == pairs.end() &&
               pairs.find(std::make_pair(b, a)) == pairs.end();
    }

    bool dependent(int a, int b, int c) {
        return (incomparable(a, c) || incomparable(b, c)) && a != c && b != c;
    }

    void remove(int x, int y) { pairs.erase(std::make_pair(x, y)); }

    void print() const {
        for (const auto &pair : pairs) {
            std::cout << "(" << pair.first << ", " << pair.second << ")"
                      << std::endl;
        }
    }
};
