/*
 * ============================================================
 *  Questão 2 — Programação Linear (PL)
 *  Problema da Dieta (Composto Alimentar de Custo Mínimo)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Variáveis de decisão:
 *      x[i] = quantidade do ingrediente i utilizada (i = 1..6)
 *
 *  Tabela de composição:
 *      Ingrediente | Vit. A | Vit. C | Preço
 *      ──────────────────────────────────────
 *           1      |   1    |   0    |  35
 *           2      |   0    |   1    |  30
 *           3      |   2    |   3    |  60
 *           4      |   2    |   1    |  50
 *           5      |   1    |   3    |  27
 *           6      |   2    |   2    |  22
 *
 *  Função objetivo — Minimizar custo:
 *      min Z = 35x1 + 30x2 + 60x3 + 50x4 + 27x5 + 22x6
 *
 *  Restrições (soluções viáveis):
 *      1*x1 + 0*x2 + 2*x3 + 2*x4 + 1*x5 + 2*x6 >= 9   (Vitamina A)
 *      0*x1 + 1*x2 + 3*x3 + 1*x4 + 3*x5 + 2*x6 >= 19  (Vitamina C)
 *      x[i] >= 0  para todo i                           (não-negatividade)
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
        IloModel model(env, "Problema_Dieta");

        // ── Variáveis de decisão ──────────────────────────────
        // x[0] = ingrediente 1, ..., x[5] = ingrediente 6
        IloNumVarArray x(env, 6, 0.0, IloInfinity, ILOFLOAT);
        x[0].setName("x1"); x[1].setName("x2"); x[2].setName("x3");
        x[3].setName("x4"); x[4].setName("x5"); x[5].setName("x6");

        // ── Dados ─────────────────────────────────────────────
        // Custo de cada ingrediente
        IloNumArray custo(env, 6,  35.0, 30.0, 60.0, 50.0, 27.0, 22.0);

        // Vitamina A por unidade de cada ingrediente
        IloNumArray vitA(env, 6,    1.0,  0.0,  2.0,  2.0,  1.0,  2.0);

        // Vitamina C por unidade de cada ingrediente
        IloNumArray vitC(env, 6,    0.0,  1.0,  3.0,  1.0,  3.0,  2.0);

        // ── Função objetivo: minimizar custo total ────────────
        model.add(IloMinimize(env, IloScalProd(custo, x)));

        // ── Restrições ────────────────────────────────────────
        model.add(IloScalProd(vitA, x) >= 9.0) .setName("vitaminaA");
        model.add(IloScalProd(vitC, x) >= 19.0).setName("vitaminaC");

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
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "========================================\n";
        std::cout << "  RESULTADO — Problema da Dieta\n";
        std::cout << "========================================\n";
        std::cout << "  Status      : " << cplex.getStatus()      << "\n";
        std::cout << "  Custo mínimo: " << cplex.getObjValue()    << "\n";
        std::cout << "  ── Quantidades ──────────────────────\n";

        const char* nomes[] = {"x1","x2","x3","x4","x5","x6"};
        for (int i = 0; i < 6; i++) {
            double val = cplex.getValue(x[i]);
            if (val > 1e-6)
                std::cout << "  " << nomes[i] << " (ingrediente "
                          << i+1 << "): " << val << "\n";
        }

        std::cout << "  ── Vitaminas obtidas ────────────────\n";

        // Calcula vitaminas obtidas (verificação de viabilidade)
        double totalA = 0.0, totalC = 0.0;
        for (int i = 0; i < 6; i++) {
            double val = cplex.getValue(x[i]);
            totalA += vitA[i] * val;
            totalC += vitC[i] * val;
        }
        std::cout << "  Vitamina A: " << totalA << " (mínimo: 9)\n";
        std::cout << "  Vitamina C: " << totalC << " (mínimo: 19)\n";
        std::cout << "========================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
