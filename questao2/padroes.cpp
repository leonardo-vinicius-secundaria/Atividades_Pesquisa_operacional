/*
 * ============================================================
 *  Questão 2 — Programação Linear Inteira (PLI)
 *  Problema dos Padrões (Fábrica de Latinhas)
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Padrões de impressão:
 *      Padrão | Folha | Corpos | Tampas | Tempo
 *      ─────────────────────────────────────────
 *         1   | Tam 1 |   1    |   7    |  2 s
 *         2   | Tam 2 |   2    |   3    |  3 s
 *         3   | Tam 1 |   0    |   9    |  2 s
 *         4   | Tam 1 |   4    |   4    |  1 s
 *
 *  Variáveis de decisão (INTEIRAS):
 *      x1,x2,x3,x4 >= 0  : vezes que cada padrão é executado
 *      L           >= 0  : latinhas completas montadas e vendidas
 *      Sc          >= 0  : sobra de corpos não utilizados
 *      St          >= 0  : sobra de tampas não utilizadas
 *
 *  Função objetivo — Maximizar lucro:
 *      max Z = 50*L - 5*Sc - 3*St
 *
 *  Restrições (soluções viáveis):
 *      [Folhas Tam 1]    x1 + x3 + x4 <= 200
 *      [Folhas Tam 2]    x2 <= 90
 *      [Tempo total]     2x1 + 3x2 + 2x3 + x4 <= 100
 *      [Balanço corpos]  x1 + 2x2 + 4x4 = L + Sc
 *      [Balanço tampas]  7x1 + 3x2 + 9x3 + 4x4 = 2L + St
 *      [Inteireza]       todas as variáveis ∈ ℤ≥0
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
        IloModel model(env, "Padroes_Latinhas");

        // ── Variáveis de decisão (INTEIRAS) ───────────────────
        IloIntVar x1(env, 0, IloIntMax, "x1_pad1");
        IloIntVar x2(env, 0, IloIntMax, "x2_pad2");
        IloIntVar x3(env, 0, IloIntMax, "x3_pad3");
        IloIntVar x4(env, 0, IloIntMax, "x4_pad4");
        IloIntVar L (env, 0, IloIntMax, "L_latinhas");
        IloIntVar Sc(env, 0, IloIntMax, "Sc_sobra_corpo");
        IloIntVar St(env, 0, IloIntMax, "St_sobra_tampa");

        // ── Função objetivo: maximizar lucro ──────────────────
        model.add(IloMaximize(env, 50*L - 5*Sc - 3*St));

        // ── Restrições de estoque ─────────────────────────────
        model.add(x1 + x3 + x4 <= 200).setName("folhas_tam1");
        model.add(x2           <=  90).setName("folhas_tam2");

        // ── Restrição de tempo ────────────────────────────────
        model.add(2*x1 + 3*x2 + 2*x3 + x4 <= 100).setName("tempo");

        // ── Balanço de corpos: produzidos = usados + sobra ────
        //    1x1 + 2x2 + 0x3 + 4x4 = L + Sc
        model.add(x1 + 2*x2 + 4*x4 == L + Sc).setName("balanco_corpos");

        // ── Balanço de tampas: produzidas = 2*usadas + sobra ─
        //    7x1 + 3x2 + 9x3 + 4x4 = 2L + St
        model.add(7*x1 + 3*x2 + 9*x3 + 4*x4 == 2*L + St).setName("balanco_tampas");

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
        int vx1 = (int)cplex.getValue(x1);
        int vx2 = (int)cplex.getValue(x2);
        int vx3 = (int)cplex.getValue(x3);
        int vx4 = (int)cplex.getValue(x4);
        int vL  = (int)cplex.getValue(L);
        int vSc = (int)cplex.getValue(Sc);
        int vSt = (int)cplex.getValue(St);

        std::cout << "============================================\n";
        std::cout << "  RESULTADO — Problema dos Padrões\n";
        std::cout << "============================================\n";
        std::cout << "  Status      : " << cplex.getStatus() << "\n";
        std::cout << "  Lucro máximo: " << (int)cplex.getObjValue() << " u.\n";

        std::cout << "\n  ── Padrões executados ───────────────\n";
        std::cout << "  Padrão 1 (Tam1 | 1C 7T | 2s): " << vx1 << "x\n";
        std::cout << "  Padrão 2 (Tam2 | 2C 3T | 3s): " << vx2 << "x\n";
        std::cout << "  Padrão 3 (Tam1 | 0C 9T | 2s): " << vx3 << "x\n";
        std::cout << "  Padrão 4 (Tam1 | 4C 4T | 1s): " << vx4 << "x\n";

        std::cout << "\n  ── Produção e montagem ──────────────\n";
        int corposProd = vx1 + 2*vx2 + 4*vx4;
        int tampasProd = 7*vx1 + 3*vx2 + 9*vx3 + 4*vx4;
        std::cout << "  Corpos produzidos : " << corposProd << "\n";
        std::cout << "  Tampas produzidas : " << tampasProd << "\n";
        std::cout << "  Latinhas montadas : " << vL  << "\n";
        std::cout << "  Sobra de corpos   : " << vSc << " (custo: "
                  << 5*vSc << " u.)\n";
        std::cout << "  Sobra de tampas   : " << vSt << " (custo: "
                  << 3*vSt << " u.)\n";

        std::cout << "\n  ── Uso de recursos ──────────────────\n";
        int folhas1 = vx1 + vx3 + vx4;
        int folhas2 = vx2;
        int tempo   = 2*vx1 + 3*vx2 + 2*vx3 + vx4;
        std::cout << "  Folhas Tam 1: " << folhas1 << " / 200\n";
        std::cout << "  Folhas Tam 2: " << folhas2 << " / 90\n";
        std::cout << "  Tempo total : " << tempo   << " / 100 s\n";

        std::cout << "\n  ── Decomposição do lucro ────────────\n";
        std::cout << "  Receita (50 x " << vL  << "): +" << 50*vL  << " u.\n";
        std::cout << "  Custo corpos (5 x " << vSc << "):  -" << 5*vSc << " u.\n";
        std::cout << "  Custo tampas (3 x " << vSt << "):  -" << 3*vSt << " u.\n";
        std::cout << "  Lucro total         :  "
                  << 50*vL - 5*vSc - 3*vSt << " u.\n";
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
