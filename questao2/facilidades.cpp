/*
 * ============================================================
 *  Questão 2 — Programação Linear Inteira Mista (PLIM)
 *  Problema das Facilidades — Facility Location (UFLP)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Variáveis de decisão:
 *      y[i]    ∈ {0,1}   : depósito i é aberto?
 *      x[i][j] ∈ [0,1]   : fração da demanda do cliente j
 *                           atendida pelo depósito i
 *
 *  Função objetivo — Minimizar custo total:
 *      min Z = Σ_i f[i]*y[i]  +  Σ_i Σ_j c[i][j]*x[i][j]
 *               (custo fixo)          (custo de atendimento)
 *
 *  Restrições (soluções viáveis):
 *      [Demanda]    Σ_i x[i][j] = 1          ∀ j  (atendimento total)
 *      [Ativação]   x[i][j] <= y[i]           ∀ i,j (só atende se aberto)
 *      [Binariedade] y[i] ∈ {0,1}
 *      [Fração]     0 <= x[i][j] <= 1
 *
 *  Exemplo: N=3 depósitos, M=4 clientes
 *      Custos fixos: f = {20, 30, 15}
 *      Matriz de custo de atendimento c[depósito][cliente]:
 *              Cl.0  Cl.1  Cl.2  Cl.3
 *      Dep.0 [   5,    4,    3,    7 ]
 *      Dep.1 [   6,    2,    8,    3 ]
 *      Dep.2 [   8,    9,    1,    4 ]
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
        IloModel model(env, "Facilidades_UFLP");

        // ── Dados do problema ─────────────────────────────────
        // Altere N, M, f[] e c[][] conforme o enunciado
        const int N = 3;   // número de depósitos potenciais
        const int M = 4;   // número de clientes

        // Custo fixo de abertura de cada depósito
        double f[] = {20.0, 30.0, 15.0};

        // Custo de atendimento c[depósito i][cliente j]
        double c[N][M] = {
            { 5.0,  4.0,  3.0,  7.0 },   // Depósito 0
            { 6.0,  2.0,  8.0,  3.0 },   // Depósito 1
            { 8.0,  9.0,  1.0,  4.0 }    // Depósito 2
        };

        // ── Variáveis de decisão ──────────────────────────────

        // y[i] ∈ {0,1}: depósito i aberto?
        IloIntVarArray y(env, N, 0, 1);
        for (int i = 0; i < N; i++) {
            std::string nome = "y_dep" + std::to_string(i);
            y[i].setName(nome.c_str());
        }

        // x[i][j] ∈ [0,1]: fração da demanda de j atendida por i
        IloArray<IloNumVarArray> x(env, N);
        for (int i = 0; i < N; i++) {
            x[i] = IloNumVarArray(env, M, 0.0, 1.0, ILOFLOAT);
            for (int j = 0; j < M; j++) {
                std::string nome = "x_dep" + std::to_string(i)
                                 + "_cl" + std::to_string(j);
                x[i][j].setName(nome.c_str());
            }
        }

        // ── Função objetivo: minimizar custo total ────────────
        IloExpr custo(env);
        // Custo fixo de abertura
        for (int i = 0; i < N; i++)
            custo += f[i] * y[i];
        // Custo de atendimento
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                custo += c[i][j] * x[i][j];
        model.add(IloMinimize(env, custo));
        custo.end();

        // ── Restrição de atendimento total da demanda ─────────
        // Cada cliente j deve ter 100% de sua demanda atendida
        for (int j = 0; j < M; j++) {
            IloExpr demanda(env);
            for (int i = 0; i < N; i++)
                demanda += x[i][j];
            std::string nome = "demanda_cl" + std::to_string(j);
            model.add(demanda == 1.0).setName(nome.c_str());
            demanda.end();
        }

        // ── Restrição de ativação: só atende se depósito aberto
        // x[i][j] <= y[i]  ∀ i,j
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                std::string nome = "ativacao_dep" + std::to_string(i)
                                 + "_cl" + std::to_string(j);
                model.add(x[i][j] <= y[i]).setName(nome.c_str());
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
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "============================================\n";
        std::cout << "  RESULTADO — Problema das Facilidades\n";
        std::cout << "============================================\n";
        std::cout << "  Status      : " << cplex.getStatus() << "\n";
        std::cout << "  Custo mínimo: " << cplex.getObjValue() << "\n";

        // Depósitos abertos
        std::cout << "\n  ── Depósitos abertos ────────────────\n";
        double custoFixoTotal = 0.0;
        for (int i = 0; i < N; i++) {
            int aberto = (int)cplex.getValue(y[i]);
            std::cout << "  Depósito " << i << ": "
                      << (aberto ? "ABERTO" : "fechado")
                      << "  (custo fixo: " << f[i] << ")\n";
            if (aberto) custoFixoTotal += f[i];
        }
        std::cout << "  Total custo fixo: " << custoFixoTotal << "\n";

        // Atribuição de clientes
        std::cout << "\n  ── Atribuição de clientes ───────────\n";
        double custoAtend = 0.0;
        for (int j = 0; j < M; j++) {
            std::cout << "  Cliente " << j << " atendido por: ";
            for (int i = 0; i < N; i++) {
                double val = cplex.getValue(x[i][j]);
                if (val > 1e-6) {
                    std::cout << "Dep." << i
                              << " (" << val*100.0 << "%, custo "
                              << c[i][j]*val << ")  ";
                    custoAtend += c[i][j] * val;
                }
            }
            std::cout << "\n";
        }
        std::cout << "  Total custo atendimento: " << custoAtend << "\n";

        std::cout << "\n  ── Decomposição do custo ────────────\n";
        std::cout << "  Custo fixo (abertura) : " << custoFixoTotal << "\n";
        std::cout << "  Custo de atendimento  : " << custoAtend     << "\n";
        std::cout << "  Custo total           : "
                  << custoFixoTotal + custoAtend << "\n";
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
