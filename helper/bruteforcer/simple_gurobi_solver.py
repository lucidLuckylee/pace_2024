import gurobipy as gp
import numpy as np
from gurobipy import GRB


def gurobi_solver(lenA, lenB, crossing_matrix):
    m = gp.Model()
    m.setParam('OutputFlag', 0)
    variable_matrix = [[m.addVar(vtype=GRB.BINARY, name=f"{u}, {v}") for v in range(lenB)] for u in range(lenB)]

    obj = gp.LinExpr()

    for u in range(lenB):
        for v in range(lenB):
            if u != v:
                obj += variable_matrix[u][v] * crossing_matrix[u][v]
                m.addConstr(variable_matrix[u][v] == 1 - variable_matrix[v][u])

                for w in range(lenB):
                    if w != u and w != v:
                        m.addConstr(variable_matrix[u][v] + variable_matrix[v][w] - variable_matrix[u][w] <= 1)

    m.setObjective(obj, GRB.MINIMIZE)
    m.optimize()

    number_of_elements_smaller = [0] * lenB

    for u in range(lenB):
        for v in range(lenB):
            if u != v:
                number_of_elements_smaller[u] += variable_matrix[v][u].X

    sorted_tuples = sorted(enumerate(number_of_elements_smaller), key=lambda x: x[1])

    sorted_indices = [index for index, element in sorted_tuples]
    m.close()
    return [i + 1 + lenA for i in sorted_indices]


if __name__ == '__main__':
    path = "tmp.gr"

    with open(path, "r") as f:
        infos = f.readline().split(" ")
        lenA = int(infos[2])
        lenB = int(infos[3])

        edges_free = [[] for _ in range(lenB)]

        for edge in f.readlines():
            u = int(edge.split(" ")[0]) - 1
            v = int(edge.split(" ")[1]) - 1 - lenA

            edges_free[v].append(u)

        crossings_matrix = np.zeros((lenB, lenB))
        for i in range(lenB):
            for j in range(i + 1, lenB):
                for m in edges_free[i]:
                    for n in edges_free[j]:
                        if m > n:
                            crossings_matrix[i][j] += 1
                        elif m < n:
                            crossings_matrix[j][i] += 1

        gurobi_solver(lenA, lenB, crossings_matrix)
