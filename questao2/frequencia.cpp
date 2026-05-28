/*
 * ============================================================
 *  Questão 2 — Programação Linear Inteira (PLI Binária)
 *  Problema de Frequência (Graph Coloring)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Duas antenas interferem se seus raios se interceptam.
 *  Antenas que interferem entre si não podem usar a mesma
 *  frequência. O objetivo é minimizar o número de frequências.
 *  Equivalente ao Problema de Coloração de Grafos (NP-difícil).
 *
 *  Variáveis de decisão (BINÁRIAS):
 *      x[i][k] = 1 se a antena i usa a frequência k
 *      y[k]    = 1 se a frequência k é utilizada por alguma antena
 *
 *  Função objetivo — Minimizar número de frequências usadas:
 *      min Z = Σ_k y[k]
 *
 *  Restrições (soluções viáveis):
 *      [Uma freq./antena]  Σ_k x[i][k] = 1           ∀ i
 *      [Sem interferência] x[i][k] + x[j][k] <= 1    ∀ (i,j)∈E, ∀ k
 *      [Ativação]          x[i][k] <= y[k]            ∀ i, k
 *      [Quebra simetria]   y[k] >= y[k+1]             ∀ k < nFreq-1
 *      [Binariedade]       x[i][k], y[k] ∈ {0,1}
 *
 *  Exemplo: 7 antenas com o seguinte grafo de interferência:
 *
 *      0 ── 1 ── 2
 *      |    |    |
 *      3 ── 4 ── 5
 *      |    |    |
 *      6 ── ─ ── ─  (3-6, 4-6, 5-6)
 *
 *  Número cromático esperado: 3 frequências
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
        IloModel model(env, "Frequencia_Antenas");

        // ── Dados do grafo de interferência ───────────────────
        // Altere nAnt e edges conforme o problema do enunciado
        const int nAnt = 7;   // número de antenas

        struct Aresta { int u, v; };
        std::vector<Aresta> edges = {
            {0,1}, {0,2}, {0,3},
            {1,2}, {1,4},
            {2,5},
            {3,4}, {3,6},
            {4,5}, {4,6},
            {5,6}
        };

        // Limite superior para o número de frequências = nAnt
        // (no pior caso, cada antena usa uma frequência diferente)
        const int nFreq = nAnt;

        // ── Variáveis de decisão (BINÁRIAS) ───────────────────

        // y[k] = frequência k é usada?
        IloIntVarArray y(env, nFreq, 0, 1);
        for (int k = 0; k < nFreq; k++) {
            std::string nome = "y_freq" + std::to_string(k);
            y[k].setName(nome.c_str());
        }

        // x[i][k] = antena i usa frequência k?
        IloArray<IloIntVarArray> x(env, nAnt);
        for (int i = 0; i < nAnt; i++) {
            x[i] = IloIntVarArray(env, nFreq, 0, 1);
            for (int k = 0; k < nFreq; k++) {
                std::string nome = "x_ant" + std::to_string(i)
                                 + "_freq" + std::to_string(k);
                x[i][k].setName(nome.c_str());
            }
        }

        // ── Função objetivo: minimizar frequências usadas ─────
        IloExpr total(env);
        for (int k = 0; k < nFreq; k++)
            total += y[k];
        model.add(IloMinimize(env, total));
        total.end();

        // ── Cada antena usa exatamente uma frequência ─────────
        for (int i = 0; i < nAnt; i++) {
            IloExpr soma(env);
            for (int k = 0; k < nFreq; k++)
                soma += x[i][k];
            std::string nome = "uma_freq_ant" + std::to_string(i);
            model.add(soma == 1).setName(nome.c_str());
            soma.end();
        }

        // ── Sem interferência: antenas adjacentes ≠ frequência ─
        for (const auto& e : edges) {
            for (int k = 0; k < nFreq; k++) {
                std::string nome = "interf_" + std::to_string(e.u)
                                 + "_" + std::to_string(e.v)
                                 + "_f" + std::to_string(k);
                model.add(x[e.u][k] + x[e.v][k] <= 1).setName(nome.c_str());
            }
        }

        // ── Ativação: frequência só está ativa se alguma antena
        //    a usar  →  x[i][k] <= y[k]
        for (int i = 0; i < nAnt; i++) {
            for (int k = 0; k < nFreq; k++) {
                model.add(x[i][k] <= y[k]);
            }
        }

        // ── Quebra de simetria: frequências usadas em ordem ───
        // y[k] >= y[k+1]: se k+1 é usada, k também deve ser
        for (int k = 0; k < nFreq - 1; k++)
            model.add(y[k] >= y[k+1]).setName(
                ("simetria_" + std::to_string(k)).c_str());

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
        std::cout << "  RESULTADO — Problema de Frequência\n";
        std::cout << "============================================\n";
        std::cout << "  Status              : " << cplex.getStatus() << "\n";
        std::cout << "  Frequências mínimas : "
                  << (int)cplex.getObjValue() << "\n";

        // Frequência atribuída a cada antena
        std::cout << "\n  Atribuição de frequências:\n";
        std::vector<int> freqAnt(nAnt, -1);
        for (int i = 0; i < nAnt; i++) {
            for (int k = 0; k < nFreq; k++) {
                if ((int)cplex.getValue(x[i][k])) {
                    freqAnt[i] = k;
                    std::cout << "  Antena " << i
                              << " → Frequência " << k << "\n";
                    break;
                }
            }
        }

        // Verificação: pares interferentes têm frequências distintas?
        std::cout << "\n  Verificação (interferências):\n";
        bool valido = true;
        for (const auto& e : edges) {
            bool conflito = (freqAnt[e.u] == freqAnt[e.v]);
            std::cout << "  Antena " << e.u << " (f=" << freqAnt[e.u]
                      << ") ↔ Antena " << e.v
                      << " (f=" << freqAnt[e.v] << "): "
                      << (conflito ? "CONFLITO!" : "OK") << "\n";
            if (conflito) valido = false;
        }
        std::cout << "\n  Solução válida: " << (valido ? "SIM" : "NAO") << "\n";
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
