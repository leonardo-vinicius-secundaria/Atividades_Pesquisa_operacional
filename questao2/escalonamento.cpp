/*
 * ============================================================
 *  Questão 2 — Programação Linear Inteira (PLI)
 *  Problema do Escalonamento de Horários (Hospital)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Ciclo de trabalho: 4 dias trabalhando + 3 dias descansando = 7 dias
 *  → Há exatamente 7 possíveis dias de início de ciclo (dias 1 a 7)
 *
 *  Variáveis de decisão (INTEIRAS):
 *      x[i] = número de enfermeiras cujo ciclo começa no dia i
 *             (trabalham nos dias i, i+1, i+2, i+3  —  mod 7)
 *      i ∈ {0,...,6}  (0=dia1, ..., 6=dia7)
 *
 *  Uma enfermeira que inicia no dia s está trabalhando no dia d se:
 *      d ∈ { s, (s+1)%7, (s+2)%7, (s+3)%7 }
 *  Equivalentemente, s ∈ { d, (d-1)%7, (d-2)%7, (d-3)%7 }
 *
 *  Função objetivo — Minimizar total de enfermeiras contratadas:
 *      min Z = x[0] + x[1] + x[2] + x[3] + x[4] + x[5] + x[6]
 *
 *  Restrições (soluções viáveis):
 *      [Demanda dia d]  x[d] + x[(d-1)%7] + x[(d-2)%7] + x[(d-3)%7]
 *                       >= demanda[d]         ∀ d ∈ {0,...,6}
 *      [Inteireza]      x[i] ∈ ℤ≥0           ∀ i
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
        IloModel model(env, "Escalonamento_Enfermeiras");

        // ── Demanda por dia (altere conforme o enunciado) ─────
        // Índice: 0=Seg, 1=Ter, 2=Qua, 3=Qui, 4=Sex, 5=Sáb, 6=Dom
        int demanda[] = {3, 4, 4, 3, 4, 5, 6};
        const char* dia[] = {"Seg","Ter","Qua","Qui","Sex","Sab","Dom"};

        // ── Variáveis de decisão (INTEIRAS) ───────────────────
        // x[i] = enfermeiras iniciando ciclo no dia i
        IloIntVarArray x(env, 7, 0, IloIntMax);
        for (int i = 0; i < 7; i++) {
            std::string nome = std::string("x_") + dia[i];
            x[i].setName(nome.c_str());
        }

        // ── Função objetivo: minimizar total de enfermeiras ───
        IloExpr total(env);
        for (int i = 0; i < 7; i++)
            total += x[i];
        model.add(IloMinimize(env, total));
        total.end();

        // ── Restrições de demanda ─────────────────────────────
        // No dia d, trabalham as que iniciaram nos últimos 4 dias:
        //   x[d] + x[(d-1+7)%7] + x[(d-2+7)%7] + x[(d-3+7)%7] >= demanda[d]
        for (int d = 0; d < 7; d++) {
            IloExpr trabalhando(env);
            for (int k = 0; k < 4; k++) {
                int inicio = ((d - k) % 7 + 7) % 7;  // mod seguro para negativos
                trabalhando += x[inicio];
            }
            std::string nome = std::string("demanda_") + dia[d];
            model.add(trabalhando >= demanda[d]).setName(nome.c_str());
            trabalhando.end();
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
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "============================================\n";
        std::cout << "  RESULTADO — Escalonamento de Enfermeiras\n";
        std::cout << "============================================\n";
        std::cout << "  Status           : " << cplex.getStatus() << "\n";
        std::cout << "  Total contratadas: "
                  << (int)cplex.getObjValue() << " enfermeiras\n";

        std::cout << "\n  Início de ciclo por dia:\n";
        for (int i = 0; i < 7; i++) {
            int val = (int)cplex.getValue(x[i]);
            std::cout << "  " << dia[i] << ": " << val
                      << " enfermeira(s) iniciam ciclo"
                         " (trabalham " << dia[i]
                      << ", " << dia[(i+1)%7]
                      << ", " << dia[(i+2)%7]
                      << ", " << dia[(i+3)%7] << ")\n";
        }

        // Verificação: enfermeiras trabalhando por dia
        std::cout << "\n  Verificação — trabalhando por dia:\n";
        std::cout << "  Dia  | Trabalhando | Demanda | OK?\n";
        std::cout << "  ─────────────────────────────────\n";
        for (int d = 0; d < 7; d++) {
            int trabalhando = 0;
            for (int k = 0; k < 4; k++) {
                int inicio = ((d - k) % 7 + 7) % 7;
                trabalhando += (int)cplex.getValue(x[inicio]);
            }
            bool ok = (trabalhando >= demanda[d]);
            std::cout << "  " << dia[d]
                      << "  |     " << std::setw(5) << trabalhando
                      << "   |   " << std::setw(4) << demanda[d]
                      << "  | " << (ok ? "SIM" : "NAO") << "\n";
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
