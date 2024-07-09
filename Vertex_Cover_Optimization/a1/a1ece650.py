import matplotlib.pyplot as plt
import networkx as nx

edges = [
    (2, 1), (6, 3), (5, 7), (9, 6), (11, 1), (9, 1), (14, 3), (8, 14), (2, 4), (10, 15),
    (2, 10), (4, 15), (14, 9), (3, 15), (7, 6), (3, 11), (11, 4), (7, 3), (6, 4), (11, 9),
    (3, 1), (5, 13)
]

# Create a graph
G = nx.Graph()

# Add edges to the graph
G.add_edges_from(edges)

# Draw the graph
plt.figure(figsize=(8, 6))
pos = nx.spring_layout(G)  # Positions nodes using Fruchterman-Reingold force-directed algorithm
nx.draw(G, pos, with_labels=True, node_size=500, node_color='skyblue', font_weight='bold', font_size=8)
plt.title('Undirected Graph')
plt.show()
