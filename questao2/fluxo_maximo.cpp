/*
 * ============================================================
 *  Questão 2 — Programação Linear (PL)
 *  Problema do Fluxo Máximo
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Nós da rede:
 *      0 = s (origem)
 *      1 = v1, 2 = v2, 3 = v3, 4 = v4, 5 = v5, 6 = v6
 *      7 = t (destino/sorvedouro)
 *
 *  Arcos direcionados e capacidades:
 *      s→v1:5  s→v2:4  s→v3:6
 *      v1→v2:4  v1→v4:6  v1→v5:5
 *      v2→v3:3  v2→v5:4
 *      v3→v5:6  v3→v6:5
 *      v4→t:5
 *      v5→v4:5  v5→t:3
 *      v6→v5:7  v6→t:6
 *
 *  Variáveis de decisão:
 *      x[a] = fluxo no arco a  (0 <= x[a] <= capacidade[a])
 *
 *  Função objetivo — Maximizar fluxo total que sai da origem:
 *      max Z = x[s→v1] + x[s→v2] + x[s→v3]
 *
 *  Restrições (soluções viáveis):
 *      [Capacidade]    0 <= x[a] <= cap[a]          ∀ arco a
 *      [Conservação]   Σ entrada(v) = Σ saída(v)    ∀ v intermediário
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
        IloModel model(env, "Fluxo_Maximo");

        // ── Nós ───────────────────────────────────────────────
        // 0=s, 1=v1, 2=v2, 3=v3, 4=v4, 5=v5, 6=v6, 7=t
        const int nNos = 8;
        const int S    = 0;
        const int T    = 7;

        const char* nomeNo[] = {"s","v1","v2","v3","v4","v5","v6","t"};

        // ── Arcos: {origem, destino, capacidade} ──────────────
        struct Arco { int de, para; double cap; };

        std::vector<Arco> arcos = {
            // Saindo de s
            {0, 1, 5.0},   // s → v1
            {0, 2, 4.0},   // s → v2
            {0, 3, 6.0},   // s → v3
            // Saindo de v1
            {1, 2, 4.0},   // v1 → v2
            {1, 4, 6.0},   // v1 → v4
            {1, 5, 5.0},   // v1 → v5
            // Saindo de v2
            {2, 3, 3.0},   // v2 → v3
            {2, 5, 4.0},   // v2 → v5
            // Saindo de v3
            {3, 5, 6.0},   // v3 → v5
            {3, 6, 5.0},   // v3 → v6
            // Saindo de v4
            {4, 7, 5.0},   // v4 → t
            // Saindo de v5
            {5, 4, 5.0},   // v5 → v4
            {5, 7, 3.0},   // v5 → t
            // Saindo de v6
            {6, 5, 7.0},   // v6 → v5
            {6, 7, 6.0}    // v6 → t
        };

        int nArcos = (int)arcos.size();

        // ── Variáveis de decisão ──────────────────────────────
        // x[a] = fluxo no arco a, limitado pela capacidade
        IloNumVarArray x(env, nArcos);
        for (int a = 0; a < nArcos; a++) {
            std::string nome = "x_" + std::string(nomeNo[arcos[a].de])
                             + "_" + std::string(nomeNo[arcos[a].para]);
            // lower=0, upper=capacidade (restringe diretamente no bound)
            x[a] = IloNumVar(env, 0.0, arcos[a].cap, ILOFLOAT, nome.c_str());
        }

        // ── Função objetivo: maximizar fluxo que sai de s ─────
        IloExpr obj(env);
        for (int a = 0; a < nArcos; a++)
            if (arcos[a].de == S)
                obj += x[a];
        model.add(IloMaximize(env, obj));
        obj.end();

        // ── Restrições de conservação de fluxo ───────────────
        // Para cada nó intermediário (nem s nem t):
        //     Σ x[arcos que chegam] = Σ x[arcos que saem]
        for (int v = 0; v < nNos; v++) {
            if (v == S || v == T) continue;

            IloExpr entrada(env), saida(env);
            for (int a = 0; a < nArcos; a++) {
                if (arcos[a].para == v) entrada += x[a];
                if (arcos[a].de   == v) saida   += x[a];
            }
            std::string nome = "conserv_" + std::string(nomeNo[v]);
            model.add(entrada == saida).setName(nome.c_str());
            entrada.end(); saida.end();
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
        std::cout << "  RESULTADO — Problema do Fluxo Máximo\n";
        std::cout << "============================================\n";
        std::cout << "  Status        : " << cplex.getStatus() << "\n";
        std::cout << "  Fluxo máximo  : " << cplex.getObjValue() << "\n";

        // Verificação pelo lado do destino
        double fluxoT = 0.0;
        for (int a = 0; a < nArcos; a++)
            if (arcos[a].para == T)
                fluxoT += cplex.getValue(x[a]);
        std::cout << "  Fluxo em t    : " << fluxoT
                  << "  (verificação)\n";

        std::cout << "\n  Fluxo por arco (fluxo / capacidade):\n";
        std::cout << "  ─────────────────────────────────────\n";
        for (int a = 0; a < nArcos; a++) {
            double val = cplex.getValue(x[a]);
            std::cout << "  " << std::setw(3) << nomeNo[arcos[a].de]
                      << " → " << std::setw(3) << nomeNo[arcos[a].para]
                      << " :  " << std::setw(5) << val
                      << " / " << arcos[a].cap;
            if (val >= arcos[a].cap - 1e-6)
                std::cout << "  [SATURADO]";
            std::cout << "\n";
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
