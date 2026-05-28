/*
 * ============================================================
 *  Questão 2 — Programação Linear (PL)
 *  Problema da Empresa de Tintas
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Insumos disponíveis para compra:
 *      SolA  : 30% SEC + 70% COR  →  R$ 1,50 / litro
 *      SolB  : 60% SEC + 40% COR  →  R$ 1,00 / litro
 *      SEC   : 100% SEC           →  R$ 4,00 / litro
 *      COR   : 100% COR           →  R$ 6,00 / litro
 *
 *  Tintas a produzir:
 *      SR (secagem rápida) : 1000 litros | ≥25% SEC, ≥50% COR
 *      SN (secagem normal) :  250 litros | ≥20% SEC, ≥50% COR
 *
 *  Variáveis de decisão (litros de cada insumo por tinta):
 *      aSR, bSR, sSR, cSR  →  SolA/SolB/SEC/COR alocados para SR
 *      aSN, bSN, sSN, cSN  →  SolA/SolB/SEC/COR alocados para SN
 *
 *  Função objetivo — Minimizar custo total de compra:
 *      min Z = 1.5(aSR+aSN) + 1.0(bSR+bSN) + 4.0(sSR+sSN) + 6.0(cSR+cSN)
 *
 *  Restrições (soluções viáveis):
 *      [Volume SR]     aSR + bSR + sSR + cSR = 1000
 *      [Volume SN]     aSN + bSN + sSN + cSN =  250
 *      [SEC em SR]     0.3*aSR + 0.6*bSR + sSR        >= 250  (25% de 1000)
 *      [COR em SR]     0.7*aSR + 0.4*bSR        + cSR >= 500  (50% de 1000)
 *      [SEC em SN]     0.3*aSN + 0.6*bSN + sSN        >=  50  (20% de  250)
 *      [COR em SN]     0.7*aSN + 0.4*bSN        + cSN >= 125  (50% de  250)
 *      [Não-neg.]      todas as variáveis >= 0
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
        IloModel model(env, "Empresa_Tintas");

        // ── Variáveis de decisão ──────────────────────────────
        // Insumos alocados para SR
        IloNumVar aSR(env, 0.0, IloInfinity, ILOFLOAT, "SolA_SR");
        IloNumVar bSR(env, 0.0, IloInfinity, ILOFLOAT, "SolB_SR");
        IloNumVar sSR(env, 0.0, IloInfinity, ILOFLOAT, "SEC_SR");
        IloNumVar cSR(env, 0.0, IloInfinity, ILOFLOAT, "COR_SR");

        // Insumos alocados para SN
        IloNumVar aSN(env, 0.0, IloInfinity, ILOFLOAT, "SolA_SN");
        IloNumVar bSN(env, 0.0, IloInfinity, ILOFLOAT, "SolB_SN");
        IloNumVar sSN(env, 0.0, IloInfinity, ILOFLOAT, "SEC_SN");
        IloNumVar cSN(env, 0.0, IloInfinity, ILOFLOAT, "COR_SN");

        // ── Função objetivo: minimizar custo total ────────────
        IloExpr custo(env);
        custo += 1.5 * (aSR + aSN);   // SolA
        custo += 1.0 * (bSR + bSN);   // SolB
        custo += 4.0 * (sSR + sSN);   // SEC puro
        custo += 6.0 * (cSR + cSN);   // COR puro
        model.add(IloMinimize(env, custo));
        custo.end();

        // ── Restrições de volume de produção ─────────────────
        model.add(aSR + bSR + sSR + cSR == 1000.0).setName("volume_SR");
        model.add(aSN + bSN + sSN + cSN ==  250.0).setName("volume_SN");

        // ── Restrições de qualidade — SR (1000 litros) ────────
        // SEC >= 25% → conteúdo de SEC >= 250 litros
        model.add(0.3*aSR + 0.6*bSR + sSR >= 250.0).setName("SEC_SR_min");
        // COR >= 50% → conteúdo de COR >= 500 litros
        model.add(0.7*aSR + 0.4*bSR + cSR >= 500.0).setName("COR_SR_min");

        // ── Restrições de qualidade — SN (250 litros) ─────────
        // SEC >= 20% → conteúdo de SEC >= 50 litros
        model.add(0.3*aSN + 0.6*bSN + sSN >= 50.0).setName("SEC_SN_min");
        // COR >= 50% → conteúdo de COR >= 125 litros
        model.add(0.7*aSN + 0.4*bSN + cSN >= 125.0).setName("COR_SN_min");

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
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "============================================\n";
        std::cout << "  RESULTADO — Empresa de Tintas\n";
        std::cout << "============================================\n";
        std::cout << "  Status      : " << cplex.getStatus() << "\n";
        std::cout << "  Custo mínimo: R$ " << cplex.getObjValue() << "\n";

        // Totais comprados de cada insumo
        double totalSolA = cplex.getValue(aSR) + cplex.getValue(aSN);
        double totalSolB = cplex.getValue(bSR) + cplex.getValue(bSN);
        double totalSEC  = cplex.getValue(sSR) + cplex.getValue(sSN);
        double totalCOR  = cplex.getValue(cSR) + cplex.getValue(cSN);

        std::cout << "\n  ── Compras totais ───────────────────\n";
        std::cout << "  SolA : " << totalSolA << " L  (R$ "
                  << 1.5 * totalSolA << ")\n";
        std::cout << "  SolB : " << totalSolB << " L  (R$ "
                  << 1.0 * totalSolB << ")\n";
        std::cout << "  SEC  : " << totalSEC  << " L  (R$ "
                  << 4.0 * totalSEC  << ")\n";
        std::cout << "  COR  : " << totalCOR  << " L  (R$ "
                  << 6.0 * totalCOR  << ")\n";

        std::cout << "\n  ── Alocação por tinta ───────────────\n";
        std::cout << "  Insumo |   SR (1000 L)  |   SN (250 L)\n";
        std::cout << "  ─────────────────────────────────────\n";
        std::cout << "  SolA   | " << std::setw(14) << cplex.getValue(aSR)
                  << " | " << std::setw(12) << cplex.getValue(aSN) << "\n";
        std::cout << "  SolB   | " << std::setw(14) << cplex.getValue(bSR)
                  << " | " << std::setw(12) << cplex.getValue(bSN) << "\n";
        std::cout << "  SEC    | " << std::setw(14) << cplex.getValue(sSR)
                  << " | " << std::setw(12) << cplex.getValue(sSN) << "\n";
        std::cout << "  COR    | " << std::setw(14) << cplex.getValue(cSR)
                  << " | " << std::setw(12) << cplex.getValue(cSN) << "\n";

        // Verificação das proporções obtidas
        double secEmSR = 0.3*cplex.getValue(aSR) + 0.6*cplex.getValue(bSR)
                       + cplex.getValue(sSR);
        double corEmSR = 0.7*cplex.getValue(aSR) + 0.4*cplex.getValue(bSR)
                       + cplex.getValue(cSR);
        double secEmSN = 0.3*cplex.getValue(aSN) + 0.6*cplex.getValue(bSN)
                       + cplex.getValue(sSN);
        double corEmSN = 0.7*cplex.getValue(aSN) + 0.4*cplex.getValue(bSN)
                       + cplex.getValue(cSN);

        std::cout << "\n  ── Verificação de qualidade ─────────\n";
        std::cout << "  SR — SEC: " << secEmSR/1000.0*100.0
                  << "% (min 25%)  COR: " << corEmSR/1000.0*100.0
                  << "% (min 50%)\n";
        std::cout << "  SN — SEC: " << secEmSN/250.0*100.0
                  << "% (min 20%)  COR: " << corEmSN/250.0*100.0
                  << "% (min 50%)\n";
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
