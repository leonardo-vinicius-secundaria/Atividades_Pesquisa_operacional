/*
 * ============================================================
 *  Questão 2 — Programação Linear Inteira (PLI Binária)
 *  Problema da Clique Máxima (Maximum Clique)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Dado um grafo G = (V, E), encontrar o maior subconjunto de
 *  vértices tal que todo par de vértices seja adjacente (clique).
 *
 *  Variáveis de decisão (BINÁRIAS):
 *      x[i] = 1 se o vértice i pertence à clique, 0 caso contrário
 *
 *  Função objetivo — Maximizar tamanho da clique:
 *      max Z = Σ_i x[i]
 *
 *  Restrições:
 *      Para todo par (i,j) que NÃO é aresta do grafo:
 *          x[i] + x[j] <= 1   (não podem estar juntos na clique)
 *      x[i] ∈ {0,1}
 *
 *  Exemplo de grafo com 7 vértices:
 *      Subgrafo {0,1,2,3} é uma clique completa (todos pares conectados)
 *
 *      Arestas:
 *          (0-1),(0-2),(0-3),(0-4)         ← 0 conecta-se a 1,2,3,4
 *          (1-2),(1-3),(1-5)               ← 1 conecta-se a 2,3,5
 *          (2-3),(2-6)                     ← 2 conecta-se a 3,6
 *          (3-6)                           ← 3 conecta-se a 6
 *          (4-5),(4-6)                     ← 4 conecta-se a 5,6
 *          (5-6)                           ← 5 conecta-se a 6
 *
 *  Clique máxima esperada: {0,1,2,3} com tamanho 4
 *
 * ============================================================
 */

#include <ilcplex/ilocplex.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

ILOSTLBEGIN

int main() {
    IloEnv env;

    try {
        IloModel model(env, "Clique_Maxima");

        // ── Dados do grafo ────────────────────────────────────
        // Altere nVerts e adj[][] conforme o grafo do enunciado
        const int nVerts = 7;

        // Matriz de adjacência: adj[i][j] = 1 se aresta (i,j) existe
        int adj[nVerts][nVerts] = {
        //    0  1  2  3  4  5  6
            { 0, 1, 1, 1, 1, 0, 0 },   // 0
            { 1, 0, 1, 1, 0, 1, 0 },   // 1
            { 1, 1, 0, 1, 0, 0, 1 },   // 2
            { 1, 1, 1, 0, 0, 0, 1 },   // 3
            { 1, 0, 0, 0, 0, 1, 1 },   // 4
            { 0, 1, 0, 0, 1, 0, 1 },   // 5
            { 0, 0, 1, 1, 1, 1, 0 }    // 6
        };

        // ── Variáveis de decisão (BINÁRIAS) ───────────────────
        IloIntVarArray x(env, nVerts, 0, 1);
        for (int i = 0; i < nVerts; i++) {
            std::string nome = "x_" + std::to_string(i);
            x[i].setName(nome.c_str());
        }

        // ── Função objetivo: maximizar tamanho da clique ──────
        IloExpr tamanho(env);
        for (int i = 0; i < nVerts; i++)
            tamanho += x[i];
        model.add(IloMaximize(env, tamanho));
        tamanho.end();

        // ── Restrições: pares de vértices não adjacentes ──────
        // Se (i,j) ∉ E então x[i] + x[j] <= 1
        // (dois vértices não conectados não podem coexistir na clique)
        int nRestr = 0;
        for (int i = 0; i < nVerts; i++) {
            for (int j = i + 1; j < nVerts; j++) {
                if (!adj[i][j]) {
                    std::string nome = "nao_aresta_"
                                     + std::to_string(i) + "_"
                                     + std::to_string(j);
                    model.add(x[i] + x[j] <= 1).setName(nome.c_str());
                    nRestr++;
                }
            }
        }

        // ── Resolve ───────────────────────────────────────────
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());

        if (!cplex.solve()) {
            std::cerr << "Solução não encontrada. Status: "
                      << cplex.getStatus() << std::endl;
            env.end();
            return 1;
        }

        // ── Resultados ────────────────────────────────────────
        std::cout << "============================================\n";
        std::cout << "  RESULTADO — Problema da Clique Máxima\n";
        std::cout << "============================================\n";
        std::cout << "  Status          : " << cplex.getStatus() << "\n";
        std::cout << "  Tamanho máximo  : "
                  << (int)cplex.getObjValue() << " vértices\n";
        std::cout << "  Restrições geradas (não-arestas): "
                  << nRestr << "\n";

        // Vértices na clique
        std::cout << "\n  Vértices da clique máxima: { ";
        std::vector<int> clique;
        for (int i = 0; i < nVerts; i++) {
            if ((int)cplex.getValue(x[i])) {
                std::cout << i << " ";
                clique.push_back(i);
            }
        }
        std::cout << "}\n";

        // Verificação: todos os pares da clique são arestas?
        std::cout << "\n  Verificação (todos os pares são arestas?):\n";
        bool valido = true;
        for (int a = 0; a < (int)clique.size(); a++) {
            for (int b = a + 1; b < (int)clique.size(); b++) {
                int u = clique[a], v = clique[b];
                bool aresta = adj[u][v];
                std::cout << "  (" << u << "," << v << "): "
                          << (aresta ? "aresta OK" : "ERRO - nao aresta!") << "\n";
                if (!aresta) valido = false;
            }
        }
        std::cout << "\n  Clique válida: " << (valido ? "SIM" : "NAO") << "\n";
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
