#include <omp.h>
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

class Graph {
    int V;
    vector<vector<int>> adj;

public:
    Graph(int V) {
        this->V = V;
        adj.resize(V);
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u); // undirected graph
    }

    // ---------------- SEQUENTIAL BFS ----------------
    void BFS_seq(int start) {
        vector<bool> visited(V, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        while (!q.empty()) {
            int node = q.front();
            q.pop();

            cout << node << " ";

            for (int neigh : adj[node]) {
                if (!visited[neigh]) {
                    visited[neigh] = true;
                    q.push(neigh);
                }
            }
        }
    }

    // ---------------- PARALLEL BFS ----------------
    void BFS_parallel(int start) {
        vector<bool> visited(V, false);
        vector<int> frontier, next_frontier;

        frontier.push_back(start);
        visited[start] = true;

        while (!frontier.empty()) {
            next_frontier.clear();

            #pragma omp parallel for
            for (int i = 0; i < frontier.size(); i++) {
                int node = frontier[i];

                cout << "Thread " << omp_get_thread_num()
                     << " visited " << node << endl;

                for (int neigh : adj[node]) {
                    if (!visited[neigh]) {
                        #pragma omp critical
                        {
                            if (!visited[neigh]) {
                                visited[neigh] = true;
                                next_frontier.push_back(neigh);
                            }
                        }
                    }
                }
            }

            frontier = next_frontier;
        }
    }

    // ---------------- SEQUENTIAL DFS ----------------
    void DFS_seq_util(int v, vector<bool> &visited) {
        visited[v] = true;
        cout << v << " ";

        for (int neigh : adj[v]) {
            if (!visited[neigh]) {
                DFS_seq_util(neigh, visited);
            }
        }
    }

    void DFS_seq(int start) {
        vector<bool> visited(V, false);
        DFS_seq_util(start, visited);
    }

    // ---------------- PARALLEL DFS ----------------
    void DFS_parallel_util(int v, vector<bool> &visited) {
        bool alreadyVisited = false;

        // Safe check + update
        #pragma omp critical
        {
            if (visited[v]) {
                alreadyVisited = true;
            } else {
                visited[v] = true;
                cout << "Thread " << omp_get_thread_num()
                     << " visited " << v << endl;
            }
        }

        if (alreadyVisited) return;

        // Parallel exploration
        #pragma omp parallel for
        for (int i = 0; i < adj[v].size(); i++) {
            int neigh = adj[v][i];
            if (!visited[neigh]) {
                DFS_parallel_util(neigh, visited);
            }
        }
    }

    void DFS_parallel(int start) {
        vector<bool> visited(V, false);
        DFS_parallel_util(start, visited);
    }
};

// ---------------- MAIN ----------------
int main() {
    Graph g(6);

    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);
    g.addEdge(3, 5);

    cout << "\nSequential BFS:\n";
    double start = omp_get_wtime();
    g.BFS_seq(0);
    double end = omp_get_wtime();
    cout << "\nTime: " << (end - start) << endl;

    cout << "\n\nParallel BFS:\n";
    start = omp_get_wtime();
    g.BFS_parallel(0);
    end = omp_get_wtime();
    cout << "Time: " << (end - start) << endl;

    cout << "\nSequential DFS:\n";
    start = omp_get_wtime();
    g.DFS_seq(0);
    end = omp_get_wtime();
    cout << "\nTime: " << (end - start) << endl;

    cout << "\n\nParallel DFS:\n";
    start = omp_get_wtime();
    g.DFS_parallel(0);
    end = omp_get_wtime();
    cout << "Time: " << (end - start) << endl;

    return 0;
}