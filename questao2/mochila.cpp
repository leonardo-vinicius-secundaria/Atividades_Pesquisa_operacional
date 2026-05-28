/*
 * ============================================================
 *  Questão 2 — Programação Linear Inteira (PLI Binária)
 *  Problema da Mochila 0/1 (Knapsack)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Variáveis de decisão (BINÁRIAS):
 *      x[i] = 1 se o item i é colocado na mochila
 *      x[i] = 0 caso contrário
 *
 *  Função objetivo — Maximizar valor total:
 *      max Z = Σ_i v[i] * x[i]
 *
 *  Restrição de capacidade:
 *      Σ_i w[i] * x[i] <= W
 *
 *  Não-negatividade / binariedade:
 *      x[i] ∈ {0, 1}
 *
 *  Exemplo com n=5 itens e capacidade W=10:
 *      Item | Valor (v) | Peso (w)
 *      ─────────────────────────────
 *        0  |     6     |    2
 *        1  |    10     |    5
 *        2  |    12     |    5
 *        3  |     7     |    3
 *        4  |     3     |    1
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
        IloModel model(env, "Mochila_01");

        // ── Dados do problema ─────────────────────────────────
        // Altere n, W, v[] e w[] conforme o enunciado
        const int n = 5;
        const int W = 10;   // capacidade máxima da mochila

        int v[] = {  6, 10, 12,  7,  3 };   // valores
        int w[] = {  2,  5,  5,  3,  1 };   // pesos

        // ── Variáveis de decisão (BINÁRIAS) ───────────────────
        IloIntVarArray x(env, n, 0, 1);
        for (int i = 0; i < n; i++) {
            std::string nome = "x_" + std::to_string(i);
            x[i].setName(nome.c_str());
        }

        // ── Função objetivo: maximizar valor total ────────────
        IloExpr valor(env);
        for (int i = 0; i < n; i++)
            valor += v[i] * x[i];
        model.add(IloMaximize(env, valor));
        valor.end();

        // ── Restrição de capacidade ───────────────────────────
        IloExpr peso(env);
        for (int i = 0; i < n; i++)
            peso += w[i] * x[i];
        model.add(peso <= W).setName("capacidade");
        peso.end();

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
        std::cout << "  RESULTADO — Problema da Mochila 0/1\n";
        std::cout << "============================================\n";
        std::cout << "  Status        : " << cplex.getStatus() << "\n";
        std::cout << "  Valor máximo  : " << (int)cplex.getObjValue() << "\n";

        std::cout << "\n  Item | Valor | Peso | Selecionado?\n";
        std::cout << "  ────────────────────────────────────\n";

        int pesoTotal = 0, valorTotal = 0;
        for (int i = 0; i < n; i++) {
            int sel = (int)cplex.getValue(x[i]);
            std::cout << "  " << std::setw(4) << i
                      << " | " << std::setw(5) << v[i]
                      << " | " << std::setw(4) << w[i]
                      << " |   " << (sel ? "SIM" : "nao") << "\n";
            if (sel) {
                pesoTotal  += w[i];
                valorTotal += v[i];
            }
        }

        std::cout << "  ────────────────────────────────────\n";
        std::cout << "  Peso total : " << pesoTotal
                  << " / " << W << "\n";
        std::cout << "  Valor total: " << valorTotal << "\n";
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
