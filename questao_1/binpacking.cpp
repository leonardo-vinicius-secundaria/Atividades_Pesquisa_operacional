/**
 * ============================================================
 *  BIN PACKING — Meta-heurística de Busca Local
 *  Disciplina: Otimização Combinatória / Pesquisa Operacional
 * ============================================================
 *
 *  Estratégia geral:
 *    1. Gera solução inicial gulosa (Best Fit Decreasing — BFD)
 *    2. Refina via Busca Local com vizinhança composta:
 *         • Relocate : move um item de um bin para outro
 *         • Swap     : troca dois itens entre bins distintos
 *       Política de aceitação: First Improvement
 *       (aceita o primeiro vizinho que reduz o nº de bins)
 *    3. Itera até esgotar o tempo limite (passado via CLI)
 *
 *  Uso:
 *    ./binpacking <arquivo_instancia> <tempo_limite_segundos>
 *
 *  Formato do arquivo de entrada:
 *    Linha 1 : n  (número de itens)
 *    Linha 2 : capacidade do bin (normalmente 1.0 ou inteiro)
 *    Linhas  : um tamanho de item por linha
 *
 * ============================================================
 */

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ─────────────────────────────────────────────
//  Tipos e alias
// ─────────────────────────────────────────────
using Clock    = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Seconds  = std::chrono::duration<double>;

// ─────────────────────────────────────────────
//  Estrutura: um Bin
// ─────────────────────────────────────────────
struct Bin {
    std::vector<int> items;   // índices dos itens alocados
    double           load{0.0}; // carga atual (soma dos tamanhos)

    // Verifica se o item de tamanho 'sz' cabe no bin (capacidade unitária)
    bool fits(double sz, double capacity) const {
        return (load + sz) <= capacity + 1e-9; // tolerância numérica
    }

    void addItem(int idx, double sz) {
        items.push_back(idx);
        load += sz;
    }

    void removeItem(int idx, double sz) {
        items.erase(std::remove(items.begin(), items.end(), idx), items.end());
        load -= sz;
    }
};

// ─────────────────────────────────────────────
//  Estrutura: uma Solução
// ─────────────────────────────────────────────
struct Solution {
    std::vector<Bin> bins;

    // ── Função de avaliação ──────────────────────────────────────────────
    // O custo é simplesmente o número de bins não-vazios.
    // Minimizar k = minimizar bins.size() (bins vazios são removidos).
    int evaluate() const {
        int count = 0;
        for (const auto& b : bins)
            if (!b.items.empty()) ++count;
        return count;
    }

    // Remove bins vazios para manter a estrutura enxuta
    void compact() {
        bins.erase(
            std::remove_if(bins.begin(), bins.end(),
                           [](const Bin& b){ return b.items.empty(); }),
            bins.end());
    }
};

// ─────────────────────────────────────────────
//  Leitura da instância
// ─────────────────────────────────────────────
struct Instance {
    int            n;          // número de itens
    double         capacity;   // capacidade de cada bin
    std::vector<double> sizes; // tamanho de cada item
};

Instance readInstance(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Não foi possível abrir o arquivo: " + filename);

    Instance inst;
    file >> inst.n >> inst.capacity;
    inst.sizes.resize(inst.n);
    for (int i = 0; i < inst.n; ++i)
        file >> inst.sizes[i];

    return inst;
}

// ─────────────────────────────────────────────
//  Solução Inicial — Best Fit Decreasing (BFD)
// ─────────────────────────────────────────────
// Ordena os itens em ordem decrescente de tamanho e insere cada item
// no bin com maior carga que ainda o comporte (melhor encaixe).
// BFD produz soluções iniciais significativamente melhores que First Fit.
Solution buildInitialSolution(const Instance& inst) {
    // Ordena índices por tamanho decrescente
    std::vector<int> order(inst.n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(),
              [&](int a, int b){ return inst.sizes[a] > inst.sizes[b]; });

    Solution sol;

    for (int idx : order) {
        double sz = inst.sizes[idx];

        // Procura o bin com maior carga que ainda comporte o item (Best Fit)
        int    bestBin  = -1;
        double bestLoad = -1.0;
        for (int b = 0; b < (int)sol.bins.size(); ++b) {
            if (sol.bins[b].fits(sz, inst.capacity) &&
                sol.bins[b].load > bestLoad) {
                bestLoad = sol.bins[b].load;
                bestBin  = b;
            }
        }

        if (bestBin == -1) {
            // Nenhum bin disponível: abre um novo
            Bin newBin;
            newBin.addItem(idx, sz);
            sol.bins.push_back(newBin);
        } else {
            sol.bins[bestBin].addItem(idx, sz);
        }
    }

    return sol;
}

// ─────────────────────────────────────────────
//  Busca Local — Vizinhança Relocate + Swap
//  Política: First Improvement
// ─────────────────────────────────────────────
//
// (c) ESTRATÉGIA DE BUSCA LOCAL
// ─────────────────────────────
// Vizinhança composta por dois movimentos:
//
//   1. RELOCATE: seleciona um item em um bin e tenta movê-lo para outro
//      bin existente. Se o bin de origem ficar vazio após a mudança,
//      o número de bins diminui → melhora garantida.
//
//   2. SWAP: seleciona um item em bin A e outro em bin B e os troca.
//      Útil para reorganizar cargas e viabilizar futuros relocates.
//
// Geração dos vizinhos:
//   Para cada par (bin_i, bin_j) e cada combinação de itens, testa-se
//   o movimento. Como a política é First Improvement, o algoritmo
//   aceita e aplica imediatamente o primeiro movimento que reduza
//   o número de bins, reiniciando a varredura.
//
// Aceitação:
//   Um movimento é aceito se e somente se reduz o valor da função
//   objetivo (número de bins). Movimentos neutros ou piores são
//   rejeitados (busca local estrita — sem perturbação aleatória aqui).

bool localSearch(Solution& sol, const Instance& inst) {
    bool improved = false;
    int  nbins    = (int)sol.bins.size();

    // ── Relocate ──────────────────────────────────────────────────────
    // Tenta mover cada item de um bin para outro bin, priorizando
    // esvaziar bins (o que reduz k diretamente).
    for (int src = 0; src < nbins && !improved; ++src) {
        if (sol.bins[src].items.empty()) continue;

        // Copia local para iterar com segurança durante possíveis remoções
        std::vector<int> items_src = sol.bins[src].items;

        for (int itemIdx : items_src) {
            double sz = inst.sizes[itemIdx];

            for (int dst = 0; dst < nbins && !improved; ++dst) {
                if (dst == src) continue;
                if (sol.bins[dst].fits(sz, inst.capacity)) {
                    // Aplica o movimento
                    sol.bins[src].removeItem(itemIdx, sz);
                    sol.bins[dst].addItem(itemIdx, sz);

                    // Verifica se o bin de origem ficou vazio → melhora
                    if (sol.bins[src].items.empty()) {
                        sol.compact();
                        improved = true;
                        nbins    = (int)sol.bins.size();
                    } else {
                        // Desfaz: não houve redução de bins
                        sol.bins[dst].removeItem(itemIdx, sz);
                        sol.bins[src].addItem(itemIdx, sz);
                    }
                }
            }
        }
    }

    // ── Swap ──────────────────────────────────────────────────────────
    // Troca um item de bin_i por um item de bin_j, depois verifica
    // se algum relocate fica viável (verificação indireta via nova
    // chamada no loop externo). Aqui o swap é aplicado se viabilizar
    // um bin vazio imediato — caso raro mas possível para itens pequenos.
    if (!improved) {
        for (int i = 0; i < nbins && !improved; ++i) {
            for (int j = i + 1; j < nbins && !improved; ++j) {
                std::vector<int> items_i = sol.bins[i].items;
                std::vector<int> items_j = sol.bins[j].items;

                for (int ai : items_i) {
                    for (int bj : items_j) {
                        double sza = inst.sizes[ai];
                        double szb = inst.sizes[bj];

                        // Testa viabilidade da troca
                        double newLoad_i = sol.bins[i].load - sza + szb;
                        double newLoad_j = sol.bins[j].load - szb + sza;

                        if (newLoad_i <= inst.capacity + 1e-9 &&
                            newLoad_j <= inst.capacity + 1e-9) {
                            // Aplica swap
                            sol.bins[i].removeItem(ai, sza);
                            sol.bins[j].removeItem(bj, szb);
                            sol.bins[i].addItem(bj, szb);
                            sol.bins[j].addItem(ai, sza);

                            // Verifica se algum bin ficou vazio
                            if (sol.bins[i].items.empty() ||
                                sol.bins[j].items.empty()) {
                                sol.compact();
                                improved = true;
                                nbins    = (int)sol.bins.size();
                            } else {
                                // Desfaz swap (não houve melhora direta)
                                sol.bins[i].removeItem(bj, szb);
                                sol.bins[j].removeItem(ai, sza);
                                sol.bins[i].addItem(ai, sza);
                                sol.bins[j].addItem(bj, szb);
                            }

                            if (improved) break;
                        }
                    }
                    if (improved) break;
                }
            }
        }
    }

    return improved;
}

// ─────────────────────────────────────────────
//  Impressão da solução final
// ─────────────────────────────────────────────
void printSolution(const Solution& sol, const Instance& inst, double elapsed) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║         SOLUÇÃO FINAL — BIN PACKING      ║\n";
    std::cout << "╠══════════════════════════════════════════╣\n";
    std::cout << "║  Bins utilizados  : " << std::setw(5) << sol.evaluate()
              << "                  ║\n";
    std::cout << "║  Tempo de execução: " << std::fixed << std::setprecision(3)
              << std::setw(8) << elapsed << " s             ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";

    // Detalhe de cada bin
    for (int b = 0; b < (int)sol.bins.size(); ++b) {
        const Bin& bin = sol.bins[b];
        std::cout << "Bin " << std::setw(3) << (b + 1)
                  << "  [carga = " << std::fixed << std::setprecision(4)
                  << bin.load << " / " << inst.capacity << "]  itens: ";
        for (int idx : bin.items)
            std::cout << idx << "(" << inst.sizes[idx] << ") ";
        std::cout << "\n";
    }
    std::cout << std::endl;
}

// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {
    // ── Parâmetros de linha de comando ────────────────────────────────
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0]
                  << " <arquivo_instancia> <tempo_limite_s>\n";
        return EXIT_FAILURE;
    }

    const std::string filename   = argv[1];
    const double      timeLimit  = std::stod(argv[2]);  // segundos

    // ── Leitura da instância ──────────────────────────────────────────
    Instance inst;
    try {
        inst = readInstance(filename);
    } catch (const std::exception& e) {
        std::cerr << "Erro ao ler instância: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Instância carregada: " << inst.n
              << " itens, capacidade = " << inst.capacity << "\n";

    // ── Solução inicial — BFD ─────────────────────────────────────────
    TimePoint start  = Clock::now();
    Solution  best   = buildInitialSolution(inst);
    best.compact();

    int bestK = best.evaluate();
    std::cout << "Solução inicial (BFD): " << bestK << " bins\n";

    // ── Loop principal de busca local ─────────────────────────────────
    // (d) CRITÉRIO DE PARADA: tempo limite fornecido via CLI
    int iter = 0;
    while (true) {
        double elapsed =
            Seconds(Clock::now() - start).count();
        if (elapsed >= timeLimit) break;

        // Tenta melhorar a solução corrente
        bool improved = localSearch(best, inst);
        ++iter;

        if (improved) {
            bestK = best.evaluate();
            std::cout << "  [iter " << iter << " | t=" << std::fixed
                      << std::setprecision(2) << elapsed
                      << "s] Melhora: " << bestK << " bins\n";
        } else {
            // Ótimo local atingido — interrompe (sem perturbação)
            std::cout << "  [iter " << iter
                      << "] Ótimo local atingido.\n";
            break;
        }
    }

    // ── Impressão final ───────────────────────────────────────────────
    double totalElapsed = Seconds(Clock::now() - start).count();
    printSolution(best, inst, totalElapsed);

    std::cout << "Função objetivo (bins): " << best.evaluate() << "\n";
    std::cout << "Iterações de busca local: " << iter << "\n";
    std::cout << "Tempo total: " << std::fixed << std::setprecision(3)
              << totalElapsed << " s\n";

    return EXIT_SUCCESS;
}

/*
 * ============================================================
 *  ANÁLISE DA ABORDAGEM
 * ============================================================
 *
 *  (a) REPRESENTAÇÃO DA SOLUÇÃO
 *  ─────────────────────────────
 *  • `Instance`: armazena n, capacidade e vetor de tamanhos.
 *  • `Bin`     : vetor de índices dos itens + carga acumulada (double).
 *  • `Solution`: vetor de Bins. O número de bins k = bins.size().
 *  A representação direta por bins (ao invés de "item → bin_id")
 *  facilita tanto a avaliação quanto a aplicação de movimentos de
 *  vizinhança, pois relocates e swaps operam diretamente nos vetores
 *  de cada bin.
 *
 *  (b) FUNÇÃO DE AVALIAÇÃO
 *  ────────────────────────
 *  f(sol) = número de bins não-vazios.
 *  Custo O(k). O objetivo é minimizar f. Bins esvaziados são
 *  removidos via `compact()`, mantendo o vetor compacto e a
 *  avaliação O(1) após compact (basta bins.size()).
 *
 *  (c) ESTRATÉGIA DE BUSCA LOCAL — ver comentários no código.
 *
 *  (d) CRITÉRIO DE PARADA — ver comentários no código.
 *
 *  COMPLEXIDADE APROXIMADA
 *  ────────────────────────
 *  Relocate: O(k² · n_max) por iteração, onde n_max é o maior nº de
 *  itens por bin. No pior caso O(k · n).
 *  Swap    : O(k² · n_max²) por iteração.
 *  Total por iteração: O(k² · n²) no pior caso.
 *  Número de iterações: limitado pelo tempo ou pelo nº de melhorias
 *  possíveis (no máximo k − k* onde k* é o ótimo).
 *
 *  VANTAGENS
 *  ──────────
 *  • Simples de implementar e entender.
 *  • BFD fornece ponto de partida de alta qualidade.
 *  • Convergência rápida em instâncias de médio porte.
 *  • Determinístico — resultados reproduzíveis.
 *
 *  LIMITAÇÕES
 *  ──────────
 *  • Fica preso em ótimos locais com facilidade.
 *  • Sem mecanismo de escape (sem perturbação / diversificação).
 *  • Para instâncias grandes, cada iteração pode ser cara.
 *
 *  MELHORIAS FUTURAS
 *  ──────────────────
 *  1. ILS (Iterated Local Search): perturbar a solução ao atingir
 *     ótimo local e reiniciar a busca local.
 *  2. Simulated Annealing: aceitar soluções piores com probabilidade
 *     decrescente, escapando de ótimos locais.
 *  3. Tabu Search: manter lista de movimentos proibidos para evitar
 *     ciclos e forçar exploração.
 *  4. VNS (Variable Neighborhood Search): alternar automaticamente
 *     entre vizinhanças quando uma fica sem melhoras.
 *  5. Paralelismo: explorar vizinhanças em threads distintas (OpenMP).
 * ============================================================
 */
