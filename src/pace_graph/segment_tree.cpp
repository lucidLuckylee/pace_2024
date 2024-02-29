#include "segment_tree.hpp"

void SegmentTree::buildTree(std::vector<int> &arr, int node, int start,
                            int end) {
    if (start == end) {
        tree[node] = arr[start];
    } else {
        int mid = (start + end) / 2;
        buildTree(arr, 2 * node, start, mid);
        buildTree(arr, 2 * node + 1, mid + 1, end);
        tree[node] = tree[2 * node] + tree[2 * node + 1];
    }
}

void SegmentTree::updateRangeUtil(int node, int start, int end, int l, int r,
                                  int val) {
    if (lazy[node] != 0) {
        tree[node] += (end - start + 1) * lazy[node];
        if (start != end) {
            lazy[node * 2] += lazy[node];
            lazy[node * 2 + 1] += lazy[node];
        }
        lazy[node] = 0;
    }

    if (start > end || start > r || end < l)
        return;

    if (start >= l && end <= r) {
        tree[node] += (end - start + 1) * val;
        if (start != end) {
            lazy[node * 2] += val;
            lazy[node * 2 + 1] += val;
        }
        return;
    }

    int mid = (start + end) / 2;
    updateRangeUtil(node * 2, start, mid, l, r, val);
    updateRangeUtil(node * 2 + 1, mid + 1, end, l, r, val);
    tree[node] = tree[node * 2] + tree[node * 2 + 1];
}

int SegmentTree::queryRangeUtil(int node, int start, int end, int l, int r) {
    if (start > end || start > r || end < l)
        return 0;

    if (lazy[node] != 0) {
        tree[node] += (end - start + 1) * lazy[node];
        if (start != end) {
            lazy[node * 2] += lazy[node];
            lazy[node * 2 + 1] += lazy[node];
        }
        lazy[node] = 0;
    }

    if (start >= l && end <= r)
        return tree[node];

    int mid = (start + end) / 2;
    return queryRangeUtil(node * 2, start, mid, l, r) +
           queryRangeUtil(node * 2 + 1, mid + 1, end, l, r);
}
