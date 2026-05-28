/*
 * ============================================================
 *  Questão 2 — Programação Linear (PL)
 *  Problema da Ração (Empresa de Alimentos Caninos)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Variáveis de decisão:
 *      x1 = quantidade (kg) de AMGS (All Mega Giga Suprema) produzida
 *      x2 = quantidade (kg) de RE  (Ração das Estrelas) produzida
 *
 *  Função objetivo — Maximizar Lucro (Receita − Custo matérias-primas):
 *
 *      Receita      :  20*x1 + 30*x2
 *      Custo cereais:   1 * (5*x1 + 2*x2)
 *      Custo carne  :   4 * (1*x1 + 4*x2)
 *
 *      Lucro = (20 - 5 - 4)*x1 + (30 - 2 - 16)*x2
 *            = 11*x1 + 12*x2
 *
 *  Restrições (soluções viáveis):
 *      5*x1 + 2*x2 <= 30.000   (disponibilidade de cereais)
 *      1*x1 + 4*x2 <= 10.000   (disponibilidade de carne)
 *      x1 >= 0,  x2 >= 0       (não-negatividade)
 *
 * ============================================================
 */

#include <ilcplex/ilocplex.h>
#include <iostream>
#include <iomanip>

ILOSTLBEGIN

int main() {
    IloEnv env;

    try {
        IloModel model(env, "Racao_Canina");

        // ── Variáveis de decisão ──────────────────────────────
        IloNumVar x1(env, 0.0, IloInfinity, ILOFLOAT, "AMGS");
        IloNumVar x2(env, 0.0, IloInfinity, ILOFLOAT, "RE");

        // ── Função objetivo: maximizar lucro ──────────────────
        model.add(IloMaximize(env, 11.0 * x1 + 12.0 * x2));

        // ── Restrições ────────────────────────────────────────
        model.add(5.0 * x1 + 2.0 * x2 <= 30000.0).setName("cereais");
        model.add(1.0 * x1 + 4.0 * x2 <= 10000.0).setName("carne");

        // ── Resolve ───────────────────────────────────────────
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); // suprime log interno do CPLEX

        if (!cplex.solve()) {
            std::cerr << "Solução não encontrada. Status: "
                      << cplex.getStatus() << std::endl;
            env.end();
            return 1;
        }

        // ── Resultados ────────────────────────────────────────
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "========================================\n";
        std::cout << "  RESULTADO — Problema da Ração\n";
        std::cout << "========================================\n";
        std::cout << "  Status    : " << cplex.getStatus()        << "\n";
        std::cout << "  Lucro max : R$ " << cplex.getObjValue()   << "\n";
        std::cout << "  AMGS (x1) : "   << cplex.getValue(x1)
                  << " kg\n";
        std::cout << "  RE   (x2) : "   << cplex.getValue(x2)
                  << " kg\n";
        std::cout << "========================================\n";

        // ── Consumo de recursos (verificação) ─────────────────
        double cereais_usados = 5.0 * cplex.getValue(x1)
                              + 2.0 * cplex.getValue(x2);
        double carne_usada    = 1.0 * cplex.getValue(x1)
                              + 4.0 * cplex.getValue(x2);
        std::cout << "  Cereais usados : " << cereais_usados
                  << " / 30000 kg\n";
        std::cout << "  Carne usada    : " << carne_usada
                  << " / 10000 kg\n";
        std::cout << "========================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
