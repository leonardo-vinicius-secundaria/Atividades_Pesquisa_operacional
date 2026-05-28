/*
 * ============================================================
 *  Questão 2 — Programação Linear Inteira (PLI Binária)
 *  Problema de Cobertura (Set Covering) — Escolas em Bairros
 * ============================================================
 *
 *  COMPONENTES DO PROBLEMA DE OTIMIZAÇÃO
 *  ──────────────────────────────────────
 *
 *  Definição do problema:
 *      Uma escola construída em um bairro cobre esse bairro e
 *      todos os seus vizinhos diretos. Toda bairro deve ser
 *      coberto por pelo menos uma escola.
 *
 *  Exemplo de cidade com 6 bairros e sua adjacência:
 *
 *          B1 ── B2 ── B4
 *          |  \   |     |
 *          B3   \ B3   B5 ── B6
 *
 *      Grafo de adjacência (incluindo o próprio nó):
 *          B1: cobre {B1, B2, B3}
 *          B2: cobre {B1, B2, B3, B4}
 *          B3: cobre {B1, B2, B3, B5}
 *          B4: cobre {B2, B4, B5, B6}
 *          B5: cobre {B3, B4, B5, B6}
 *          B6: cobre {B4, B5, B6}
 *
 *  Variáveis de decisão (BINÁRIAS):
 *      y[j] = 1 se escola é construída no bairro j, 0 caso contrário
 *
 *  Função objetivo — Minimizar número de escolas:
 *      min Z = y[0] + y[1] + ... + y[n-1]
 *
 *  Restrições (soluções viáveis):
 *      [Cobertura i]   Σ_{j ∈ N[i]} y[j] >= 1    ∀ bairro i
 *      [Binariedade]   y[j] ∈ {0, 1}             ∀ j
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
        IloModel model(env, "Cobertura_Escolas");

        // ── Dados da cidade ───────────────────────────────────
        // Altere nBairros e a matriz de cobertura conforme o
        // grafo de adjacência do seu enunciado.
        const int nBairros = 6;
        const char* nomeBairro[] = {"B1","B2","B3","B4","B5","B6"};

        // cover[i][j] = 1 se uma escola no bairro j cobre o bairro i
        //               (j é o próprio i ou vizinho de i)
        //
        //         escola em:  B1  B2  B3  B4  B5  B6
        int cover[nBairros][nBairros] = {
            /* B1 cobre: */ { 1,  1,  1,  0,  0,  0 },
            /* B2 cobre: */ { 1,  1,  1,  1,  0,  0 },
            /* B3 cobre: */ { 1,  1,  1,  0,  1,  0 },
            /* B4 cobre: */ { 0,  1,  0,  1,  1,  1 },
            /* B5 cobre: */ { 0,  0,  1,  1,  1,  1 },
            /* B6 cobre: */ { 0,  0,  0,  1,  1,  1 }
        };

        // ── Variáveis de decisão (BINÁRIAS) ───────────────────
        IloIntVarArray y(env, nBairros, 0, 1);
        for (int j = 0; j < nBairros; j++) {
            std::string nome = std::string("y_") + nomeBairro[j];
            y[j].setName(nome.c_str());
        }

        // ── Função objetivo: minimizar escolas construídas ────
        IloExpr total(env);
        for (int j = 0; j < nBairros; j++)
            total += y[j];
        model.add(IloMinimize(env, total));
        total.end();

        // ── Restrições de cobertura ───────────────────────────
        // Para cada bairro i: pelo menos uma escola nos bairros
        // que o cobrem deve ser construída
        for (int i = 0; i < nBairros; i++) {
            IloExpr cobertura(env);
            for (int j = 0; j < nBairros; j++)
                if (cover[i][j])
                    cobertura += y[j];
            std::string nome = std::string("cobertura_") + nomeBairro[i];
            IloRange r = (cobertura >= 1);
            r.setName(nome.c_str());
            model.add(r);
            cobertura.end();
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
        std::cout << "============================================\n";
        std::cout << "  RESULTADO — Cobertura de Escolas\n";
        std::cout << "============================================\n";
        std::cout << "  Status           : " << cplex.getStatus() << "\n";
        std::cout << "  Escolas mínimas  : "
                  << (int)cplex.getObjValue() << "\n";

        std::cout << "\n  Onde construir:\n";
        std::vector<int> escolhidos;
        for (int j = 0; j < nBairros; j++) {
            int val = (int)cplex.getValue(y[j]);
            std::cout << "  " << nomeBairro[j] << ": "
                      << (val ? "CONSTRUIR" : "não") << "\n";
            if (val) escolhidos.push_back(j);
        }

        // Verificação: quais bairros cada escola cobre
        std::cout << "\n  Cobertura resultante:\n";
        for (int j : escolhidos) {
            std::cout << "  Escola em " << nomeBairro[j] << " cobre: ";
            for (int i = 0; i < nBairros; i++)
                if (cover[i][j])
                    std::cout << nomeBairro[i] << " ";
            std::cout << "\n";
        }

        // Verificação: todos os bairros cobertos?
        std::cout << "\n  Verificação de cobertura:\n";
        bool todoCoberto = true;
        for (int i = 0; i < nBairros; i++) {
            bool coberto = false;
            for (int j : escolhidos)
                if (cover[i][j]) { coberto = true; break; }
            std::cout << "  " << nomeBairro[i] << ": "
                      << (coberto ? "coberto" : "NAO COBERTO") << "\n";
            if (!coberto) todoCoberto = false;
        }
        std::cout << "\n  Solução válida: " << (todoCoberto ? "SIM" : "NAO") << "\n";
        std::cout << "============================================\n";

    } catch (IloException &e) {
        std::cerr << "Erro CPLEX: " << e.getMessage() << std::endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}
