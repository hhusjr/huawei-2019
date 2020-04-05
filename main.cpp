#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <fstream>
#include <thread>
#include <map>
#include <algorithm>

#define MAX_V 300000
#define N_CPU 9

std::vector<int> edges[MAX_V];
std::set<int> vertexes;
std::map<int, int> id2v;
int in_deg[MAX_V];
int v2id[MAX_V];
int n_cycles;
std::vector<std::vector<int> > cycles;
/* 避免并发情况 */
std::vector<std::vector<int> > partial_cycles[N_CPU];

void read_data() {
    std::ifstream fs("../test_data.txt");
    fs.tie(nullptr);
    char separator;
    int u, v, w;

    while (fs >> u >> separator >> v >> separator >> w) {
        if (!id2v.count(u)) {
            id2v[u] = id2v.size();
            v2id[id2v[u]] = u;
        }
        if (!id2v.count(v)) {
            id2v[v] = id2v.size();
            v2id[id2v[v]] = v;
        }
        int v_u = id2v[u];
        int v_v = id2v[v];
        edges[v_u].emplace_back(v_v);
        vertexes.insert(v_u);
        vertexes.insert(v_v);
        in_deg[v_v]++;
    }
}

void top_sort() {
    std::queue<int> q;
    for (auto &vertex : vertexes) {
        if (!in_deg[vertex]) {
            q.push(vertex);

        }
    }
    while (!q.empty()) {
        int head = q.front();
        q.pop();
        for (auto &out_point : edges[head]) {
            in_deg[out_point]--;
            if (!in_deg[out_point]) {
                q.push(out_point);
            }
        }
    }
}

bool has_edge(int source, int p) {
    for (auto &out_point : edges[source]) {
        if (p == out_point) {
            return true;
        }
    }
    return false;
}

void dfs(int root, int source, int dist, bool vis[], int route_st[], int &route_st_sp, int mod) {
    route_st[++route_st_sp] = root;
    if (dist >= 2 && has_edge(root, source)) {
        std::vector<int> temp;
        temp.reserve(route_st_sp + 1);
        for (int i = 0; i <= route_st_sp; i++) {
            temp.emplace_back(route_st[i]);
        }
        partial_cycles[mod].emplace_back(temp);
    }
    if (dist >= 6) {
        route_st_sp--;
        return;
    }
    for (auto &out_point : edges[root]) {
        if (v2id[out_point] <= v2id[source] || vis[out_point] || !in_deg[out_point]) {
            continue;
        }
        vis[out_point] = true;
        dfs(out_point, source, dist + 1, vis, route_st, route_st_sp, mod);
        vis[out_point] = false;
    }
    route_st_sp--;
}

void search_partition(int mod) {
    bool vis[MAX_V] = {false};
    for (auto &vertex : vertexes) {
        if (vertex % N_CPU != mod || !in_deg[vertex]) {
            continue;
        }
        vis[vertex] = true;
        int route_st[8];
        int route_st_sp = -1;
        dfs(vertex, vertex, 0, vis, route_st, route_st_sp, mod);
        vis[vertex] = false;
    }
}

void merge_partitions() {
    for (auto &partial_cycle : partial_cycles) {
        for (auto &cycle : partial_cycle) {
            cycles.emplace_back(cycle);
            n_cycles++;
        }
    }
    std::sort(cycles.begin(), cycles.end(), [](std::vector<int> x, std::vector<int> y) {
        unsigned long size_x = x.size();
        unsigned long size_y = y.size();
        if (size_x != size_y) {
            return size_x < size_y;
        }
        for (int i = 0; i < size_x; i++) {
            if (v2id[x[i]] == v2id[y[i]]) {
                continue;
            }
            return v2id[x[i]] < v2id[y[i]];
        }
        return false;
    });
}

void create_workers() {
    std::thread threads[N_CPU];
    for (int mod = 0; mod < N_CPU; mod++) {
        threads[mod] = std::thread(&search_partition, mod);
    }
    for (auto &thread : threads) {
        thread.join();
    }
}

void response() {
    std::cout << n_cycles << std::endl;
    for (int i = 0; i < n_cycles; i++) {
        int j;
        for (j = 0; j < cycles[i].size() - 1; j++) {
            std::cout << v2id[cycles[i][j]] << ",";
        }
        std::cout << v2id[cycles[i][j]] << std::endl;
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    read_data();
    top_sort();
    create_workers();
    merge_partitions();
    response();
}
