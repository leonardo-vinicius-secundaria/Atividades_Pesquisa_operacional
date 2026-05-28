/*
 * ============================================================
 *  Questão 2 — Programação Linear (PL)
 *  Problema do Transporte (Balanceado)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Variáveis de decisão:
 *      x[i][j] = unidades transportadas da fábrica i para o depósito j
 *      i ∈ {0,1,2}  →  Fábricas 1, 2, 3
 *      j ∈ {0,1,2}  →  Depósitos 1, 2, 3
 *
 *  Oferta das fábricas: {120, 80, 80}  (total = 280)
 *  Demanda dos depósitos: {150, 70, 60} (total = 280)
 *  → Problema balanceado (oferta total = demanda total)
 *
 *  Matriz de custos c[i][j]:
 *            D1   D2   D3
 *      F1  [  8    5    6 ]
 *      F2  [ 15   10   12 ]
 *      F3  [  3    9   10 ]
 *
 *  Função objetivo — Minimizar custo total de transporte:
 *      min Z = Σ_i Σ_j c[i][j] * x[i][j]
 *
 *  Restrições (soluções viáveis):
 *      [Oferta]     Σ_j x[i][j] = oferta[i]    ∀ i  (igualdade — balanceado)
 *      [Demanda]    Σ_i x[i][j] = demanda[j]   ∀ j  (igualdade — balanceado)
 *      [Não-neg.]   x[i][j] >= 0               ∀ i,j
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
        IloModel model(env, "Problema_Transporte");

        const int nFab  = 3;   // fábricas
        const int nDep  = 3;   // depósitos

        // ── Dados ─────────────────────────────────────────────
        double oferta[]  = {120.0, 80.0, 80.0};
        double demanda[] = {150.0, 70.0, 60.0};

        // Matriz de custos c[fabrica][deposito]
        double c[nFab][nDep] = {
            {  8.0,  5.0,  6.0 },   // Fábrica 1
            { 15.0, 10.0, 12.0 },   // Fábrica 2
            {  3.0,  9.0, 10.0 }    // Fábrica 3
        };

        // ── Variáveis de decisão ──────────────────────────────
        IloArray<IloNumVarArray> x(env, nFab);
        for (int i = 0; i < nFab; i++) {
            x[i] = IloNumVarArray(env, nDep, 0.0, IloInfinity, ILOFLOAT);
            for (int j = 0; j < nDep; j++) {
                std::string nome = "x_F" + std::to_string(i+1)
                                 + "_D" + std::to_string(j+1);
                x[i][j].setName(nome.c_str());
            }
        }

        // ── Função objetivo: minimizar custo total ────────────
        IloExpr obj(env);
        for (int i = 0; i < nFab; i++)
            for (int j = 0; j < nDep; j++)
                obj += c[i][j] * x[i][j];
        model.add(IloMinimize(env, obj));
        obj.end();

        // ── Restrições de oferta (igualdade — balanceado) ─────
        for (int i = 0; i < nFab; i++) {
            IloExpr enviado(env);
            for (int j = 0; j < nDep; j++)
                enviado += x[i][j];
            std::string nome = "oferta_F" + std::to_string(i+1);
            model.add(enviado == oferta[i]).setName(nome.c_str());
            enviado.end();
        }

        // ── Restrições de demanda (igualdade — balanceado) ────
        for (int j = 0; j < nDep; j++) {
            IloExpr recebido(env);
            for (int i = 0; i < nFab; i++)
                recebido += x[i][j];
            std::string nome = "demanda_D" + std::to_string(j+1);
            model.add(recebido == demanda[j]).setName(nome.c_str());
            recebido.end();
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
        std::cout << "  RESULTADO — Problema do Transporte\n";
        std::cout << "============================================\n";
        std::cout << "  Status      : " << cplex.getStatus() << "\n";
        std::cout << "  Custo mínimo: " << cplex.getObjValue() << "\n";

        std::cout << "\n  Plano de transporte (unidades):\n";
        std::cout << "         |   D1     |   D2     |   D3     |  Enviado / Oferta\n";
        std::cout << "  ────────────────────────────────────────────────────────\n";

        for (int i = 0; i < nFab; i++) {
            double enviado = 0.0;
            std::cout << "  F" << i+1 << "      |";
            for (int j = 0; j < nDep; j++) {
                double val = cplex.getValue(x[i][j]);
                enviado += val;
                std::cout << " " << std::setw(8) << val << " |";
            }
            std::cout << "  " << enviado << " / " << oferta[i] << "\n";
        }

        std::cout << "  ────────────────────────────────────────────────────────\n";
        std::cout << "  Recebido|";
        for (int j = 0; j < nDep; j++) {
            double recebido = 0.0;
            for (int i = 0; i < nFab; i++)
                recebido += cplex.getValue(x[i][j]);
            std::cout << " " << std::setw(8) << recebido << " |";
        }
        std::cout << "\n  Demanda |";
        for (int j = 0; j < nDep; j++)
            std::cout << " " << std::setw(8) << demanda[j] << " |";
        std::cout << "\n";

        // Custo por rota (apenas rotas ativas)
        std::cout << "\n  Rotas ativas:\n";
        for (int i = 0; i < nFab; i++) {
            for (int j = 0; j < nDep; j++) {
                double val = cplex.getValue(x[i][j]);
                if (val > 1e-6) {
                    std::cout << "  F" << i+1 << " → D" << j+1
                              << ": " << val << " un. × R$"
                              << c[i][j] << " = R$"
                              << val * c[i][j] << "\n";
                }
            }
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
