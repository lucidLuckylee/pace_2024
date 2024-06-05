import random

import networkx as nx
import numpy as np


def generate_random_caterpillar(backbone_length,
                                min_leaves_per_node, max_leaves_per_node):
    # Generate the length of the backbone

    # Create the graph
    G = nx.Graph()

    # Add the backbone path to the graph
    backbone_nodes = range(backbone_length)
    nx.add_path(G, backbone_nodes)

    # Add leaves to the backbone nodes
    leaf_counter = backbone_length  # Start counter for naming leaf nodes
    for node in backbone_nodes:
        # Decide the number of leaves for this node
        num_leaves = random.randint(min_leaves_per_node, max_leaves_per_node)
        for _ in range(num_leaves):
            G.add_node(leaf_counter)
            G.add_edge(node, leaf_counter)
            leaf_counter += 1

    return G


def generate_random_lobster(n, p1, p2):
    return nx.random_lobster(n, p1, p2)


def generate_random_tree(n):
    return nx.random_tree(n)


def generate_random_star_forest(star_sizes):
    G = nx.Graph()
    current_node = 0

    for size in star_sizes:
        # Generate a star graph with `size` leaves
        star = nx.star_graph(size)

        # Relabel nodes to ensure they are unique across the forest
        mapping = {i: i + current_node for i in range(size + 1)}
        star = nx.relabel_nodes(star, mapping)
        current_node += size + 1

        # Add the star to the forest
        G = nx.compose(G, star)
    return G


def generate_random_subdivided_wheel_graph(n, subdivision_min, subdivision_max):
    # Create a basic wheel graph
    G = nx.wheel_graph(n)

    # To avoid modifying the graph while iterating over its edges, collect the edges first
    edges_to_subdivide = list(G.edges())

    for edge in edges_to_subdivide:
        G.remove_edge(*edge)

        factor = random.randint(subdivision_min, subdivision_max)
        last_node = edge[0]
        for _ in range(factor):
            new_node = max(G.nodes()) + 1
            G.add_node(new_node)
            G.add_edge(last_node, new_node)
            last_node = new_node

        G.add_edge(last_node, edge[1])

    return G


def create_circular_ladder_graph(n):
    return nx.circular_ladder_graph(n)


def generate_random_disk_intersection_graph(n, radius):
    return nx.random_geometric_graph(n, radius)


def generate_graph_with_small_treewidth(n):
    G = nx.random_tree(n)

    added_edges = 0
    try_limit = 20
    while added_edges < np.log(n) and try_limit > 0:
        a, b = random.sample(list(G.nodes()), 2)
        if not G.has_edge(a, b):
            G.add_edge(a, b)
            added_edges += 1
        try_limit -= 1

    return G


def generate_random_graph(min_size=50, max_size=200, init_crossing_matrix=True):
    graph_generations_methods = [
        "generate_random_caterpillar",
        "generate_random_lobster",
        "generate_random_tree",
        "generate_random_star_forest",
        "generate_random_subdivided_wheel_graph",
        "create_circular_ladder_graph",
        "generate_graph_with_small_treewidth"
    ]

    method = random.choice(graph_generations_methods)
    A = []
    B = []
    G = None
    while len(A) == 0 or len(B) == 0:
        while G is None or not nx.is_bipartite(G) or G.number_of_nodes() < min_size or G.number_of_nodes() > max_size:
            if method == "generate_random_caterpillar":
                backbone_length = random.randint(min_size // 10, max_size // 10)
                p1 = random.randint(1, 10)
                p2 = random.randint(1, 10)
                G = generate_random_caterpillar(backbone_length, min(p1, p2), max(p1, p2))
            elif method == "generate_random_lobster":
                n = random.randint(min_size, max_size)
                p1 = random.random()
                p2 = random.random()
                G = generate_random_lobster(n, p1, p2)
            elif method == "generate_random_tree":
                n = random.randint(min_size, max_size)
                G = generate_random_tree(n)
            elif method == "generate_random_star_forest":
                star_sizes = [random.randint(min_size // 5, max_size // 5) for _ in range(random.randint(1, 10))]
                G = generate_random_star_forest(star_sizes)
            elif method == "generate_random_subdivided_wheel_graph":
                n = random.randint(min_size // 4, max_size // 4)
                p1 = random.randint(1, 10)
                p2 = random.randint(1, 10)
                G = generate_random_subdivided_wheel_graph(n, min(p1, p2), max(p1, p2))
            elif method == "create_circular_ladder_graph":
                n = random.randint(min_size, max_size)
                G = create_circular_ladder_graph(n)
            elif method == "generate_random_disk_intersection_graph":
                n = random.randint(min_size, max_size)
                radius = random.random()
                G = generate_random_disk_intersection_graph(n, radius)
            elif method == "generate_graph_with_small_treewidth":
                n = random.randint(min_size, max_size)
                G = generate_graph_with_small_treewidth(n)

        for component in nx.connected_components(G):
            subgraph = G.subgraph(component)
            set1, set2 = nx.algorithms.bipartite.sets(subgraph)

            if random.random() < 0.5:
                A.extend(set1)
                B.extend(set2)
            else:
                A.extend(set2)
                B.extend(set1)

        for aNode in A:
            if G.degree(aNode) == 0:
                A.remove(aNode)

        for bNode in B:
            if G.degree(bNode) == 0:
                B.remove(bNode)

        if len(A) == 0 or len(B) == 0:
            continue

        random.shuffle(A)
        random.shuffle(B)

    edges_fixed = [[] for _ in range(len(A))]
    edges_free = [[] for _ in range(len(B))]

    for aIndex in range(len(A)):
        for bIndex in range(len(B)):
            if G.has_edge(A[aIndex], B[bIndex]):
                edges_fixed[aIndex].append(bIndex)
                edges_free[bIndex].append(aIndex)

    if init_crossing_matrix:
        crossings_matrix = np.zeros((len(B), len(B)))
        for i in range(len(B)):
            for j in range(i + 1, len(B)):
                for m in edges_free[i]:
                    for n in edges_free[j]:
                        if m > n:
                            crossings_matrix[i][j] += 1
                        elif m < n:
                            crossings_matrix[j][i] += 1
    else:
        crossings_matrix = None

    return edges_fixed, edges_free, len(A), len(B), crossings_matrix
