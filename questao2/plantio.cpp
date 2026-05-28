/*
 * ============================================================
 *  Questão 2 — Programação Linear (PL)
 *  Problema do Plantio (Cooperativa Agrícola)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Variáveis de decisão:
 *      x[i][j] = acres da cultura j plantados na fazenda i
 *      i ∈ {0,1,2}  →  Fazendas 1, 2, 3
 *      j ∈ {0,1,2}  →  Milho, Arroz, Feijão
 *
 *  Dados das fazendas:
 *      Fazenda | Área (acres) | Água (litros)
 *      ─────────────────────────────────────
 *         1    |     400      |    1800
 *         2    |     650      |    2200
 *         3    |     350      |     950
 *
 *  Dados das culturas:
 *      Cultura | Área global (acres) | Água (L/acre) | Lucro (R$/acre)
 *      ──────────────────────────────────────────────────────────────
 *      Milho   |        660          |      5.5      |    5000
 *      Arroz   |        880          |      4.0      |    4000
 *      Feijão  |        400          |      3.5      |    1800
 *
 *  Função objetivo — Maximizar lucro total:
 *      max Z = Σ_i Σ_j lucro[j] * x[i][j]
 *
 *  Restrições (soluções viáveis):
 *      [Área por fazenda]    Σ_j x[i][j] <= areaMax[i]           ∀ i
 *      [Água por fazenda]    Σ_j agua[j]*x[i][j] <= aguaMax[i]   ∀ i
 *      [Área global/cultura] Σ_i x[i][j] <= areaGlobal[j]        ∀ j
 *      [Equidade]            Σ_j x[0][j]   Σ_j x[1][j]   Σ_j x[2][j]
 *                            ──────────── = ──────────── = ────────────
 *                                400            650            350
 *      [Não-negatividade]    x[i][j] >= 0                       ∀ i,j
 *
 * ============================================================
 */

#include <ilcplex/ilocplex.h>
#include <iostream>
#include <iomanip>
#include <string>

ILOSTLBEGIN

int main() {
    IloEnv env;

    try {
        IloModel model(env, "Problema_Plantio");

        const int nFazendas = 3;
        const int nCulturas = 3;

        // ── Dados das fazendas ────────────────────────────────
        double areaMax[] = {400.0, 650.0, 350.0};
        double aguaMax[] = {1800.0, 2200.0, 950.0};

        // ── Dados das culturas: Milho, Arroz, Feijão ─────────
        double areaGlobal[] = {660.0, 880.0, 400.0};
        double agua[]       = {5.5,   4.0,   3.5};
        double lucro[]      = {5000.0, 4000.0, 1800.0};

        const char* nomeFazenda[] = {"F1", "F2", "F3"};
        const char* nomeCultura[] = {"Milho ", "Arroz ", "Feijao"};

        // ── Variáveis de decisão ──────────────────────────────
        // x[i][j] = acres da cultura j na fazenda i
        IloArray<IloNumVarArray> x(env, nFazendas);
        for (int i = 0; i < nFazendas; i++) {
            x[i] = IloNumVarArray(env, nCulturas, 0.0, IloInfinity, ILOFLOAT);
            for (int j = 0; j < nCulturas; j++) {
                std::string nome = std::string("x_") + nomeFazenda[i]
                                 + "_" + nomeCultura[j];
                x[i][j].setName(nome.c_str());
            }
        }

        // ── Função objetivo: maximizar lucro total ────────────
        IloExpr obj(env);
        for (int i = 0; i < nFazendas; i++)
            for (int j = 0; j < nCulturas; j++)
                obj += lucro[j] * x[i][j];
        model.add(IloMaximize(env, obj));
        obj.end();

        // ── Restrições de área por fazenda ────────────────────
        for (int i = 0; i < nFazendas; i++) {
            IloExpr areaUsada(env);
            for (int j = 0; j < nCulturas; j++)
                areaUsada += x[i][j];
            std::string nome = std::string("area_") + nomeFazenda[i];
            model.add(areaUsada <= areaMax[i]).setName(nome.c_str());
            areaUsada.end();
        }

        // ── Restrições de água por fazenda ────────────────────
        for (int i = 0; i < nFazendas; i++) {
            IloExpr aguaUsada(env);
            for (int j = 0; j < nCulturas; j++)
                aguaUsada += agua[j] * x[i][j];
            std::string nome = std::string("agua_") + nomeFazenda[i];
            model.add(aguaUsada <= aguaMax[i]).setName(nome.c_str());
            aguaUsada.end();
        }

        // ── Restrições de área global por cultura ─────────────
        for (int j = 0; j < nCulturas; j++) {
            IloExpr totalCultura(env);
            for (int i = 0; i < nFazendas; i++)
                totalCultura += x[i][j];
            std::string nome = std::string("global_") + nomeCultura[j];
            model.add(totalCultura <= areaGlobal[j]).setName(nome.c_str());
            totalCultura.end();
        }

        // ── Restrição de equidade (mesma proporção por fazenda)
        //    F1_usada / 400 = F2_usada / 650  =>  650*F1 = 400*F2
        //    F2_usada / 650 = F3_usada / 350  =>  350*F2 = 650*F3
        {
            IloExpr area0(env), area1(env), area2(env);
            for (int j = 0; j < nCulturas; j++) {
                area0 += x[0][j];
                area1 += x[1][j];
                area2 += x[2][j];
            }
            model.add(650.0 * area0 == 400.0 * area1).setName("equidade_F1_F2");
            model.add(350.0 * area1 == 650.0 * area2).setName("equidade_F2_F3");
            area0.end(); area1.end(); area2.end();
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
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "============================================\n";
        std::cout << "  RESULTADO — Problema do Plantio\n";
        std::cout << "============================================\n";
        std::cout << "  Status      : " << cplex.getStatus() << "\n";
        std::cout << "  Lucro máximo: R$ " << cplex.getObjValue() << "\n";

        std::cout << "\n  Tabela de Plantio (acres):\n";
        std::cout << "  Fazenda | Milho    | Arroz    | Feijão   |"
                     " Total    | Disp.\n";
        std::cout << "  ──────────────────────────────────────────"
                     "────────────────\n";

        for (int i = 0; i < nFazendas; i++) {
            double total = 0.0;
            std::cout << "     " << nomeFazenda[i] << "   |";
            for (int j = 0; j < nCulturas; j++) {
                double val = cplex.getValue(x[i][j]);
                total += val;
                std::cout << " " << std::setw(8) << val << " |";
            }
            std::cout << " " << std::setw(8) << total << " |"
                      << "  " << areaMax[i] << "\n";
        }

        std::cout << "\n  Proporção utilizada por fazenda (equidade):\n";
        for (int i = 0; i < nFazendas; i++) {
            double total = 0.0;
            for (int j = 0; j < nCulturas; j++)
                total += cplex.getValue(x[i][j]);
            std::cout << "  " << nomeFazenda[i] << ": "
                      << total / areaMax[i] * 100.0 << "%\n";
        }

        std::cout << "\n  Área total por cultura:\n";
        for (int j = 0; j < nCulturas; j++) {
            double total = 0.0;
            for (int i = 0; i < nFazendas; i++)
                total += cplex.getValue(x[i][j]);
            std::cout << "  " << nomeCultura[j] << ": " << total
                      << " / " << areaGlobal[j] << " acres\n";
        }
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
