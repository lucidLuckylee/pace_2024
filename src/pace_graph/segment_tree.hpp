#ifndef PACE2024_SEGMENT_TREE_HPP
#define PACE2024_SEGMENT_TREE_HPP

#include <bits/stdc++.h>

/**
 * SegmentTree class implements a segment tree with lazy propagation.
 * It supports efficient range updates (adding a value to all elements in a
 * segment) and queries (sum of elements in a segment) on an array of integers.
 */
class SegmentTree {
  private:
    int n;
    std::vector<int> tree, lazy;

    void buildTree(std::vector<int> &arr, int node, int start, int end);

    void updateRangeUtil(int node, int start, int end, int l, int r, int val);

    int queryRangeUtil(int node, int start, int end, int l, int r);

  public:
    /**
     * Constructor that initializes and builds the segment tree from the given
     * array.
     * @param arr The array based on which the segment tree is built.
     */
    SegmentTree(std::vector<int> &arr) {
        n = arr.size();
        tree.assign(4 * n, 0);
        lazy.assign(4 * n, 0);
        buildTree(arr, 1, 0, n - 1);
    }

    /**
     * Updates all elements in the specified range by adding the given value to
     * them.
     * @param l The starting index of the range to be updated (inclusive).
     * @param r The ending index of the range to be updated (inclusive).
     * @param val The value to be added to each element in the specified range.
     */
    void updateRange(int l, int r, int val) {
        updateRangeUtil(1, 0, n - 1, l, r, val);
    }

    /**
     * Updates a single element in the array by adding the given value to it.
     * This is a convenience method that internally calls updateRange.
     * @param idx The index of the element to be updated.
     * @param val The value to be added to the element.
     */
    void updateSingle(int idx, int val) { updateRange(idx, idx, val); }

    /**
     * Queries the sum of elements in the specified range.
     * @param l The starting index of the range (inclusive).
     * @param r The ending index of the range (inclusive).
     * @return The sum of the elements in the specified range.
     */
    int queryRange(int l, int r) { return queryRangeUtil(1, 0, n - 1, l, r); }
};

#endif // PACE2024_SEGMENT_TREE_HPP
