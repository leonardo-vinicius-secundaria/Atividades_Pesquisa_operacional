/**
 * ============================================================
 *  BIN PACKING — Meta-heurística de Busca Local + ILS
 *  Formato: OR-Library / binpack1
 *
 *  Pipeline:
 *    1. Solução inicial: Best Fit Decreasing (BFD)
 *    2. Busca Local (Relocate + Swap neutro — First Improvement)
 *    3. ILS: ao atingir ótimo local, perturba e reinicia
 *       (exploração no tempo disponível)
 *
 *  Uso:
 *    ./binpacking2 <arquivo> <tempo_s> [índice]
 *    Índice 0-based. Omitir = resolve todas.
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
#include <stdexcept>
#include <string>
#include <vector>

using Clock     = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Seconds   = std::chrono::duration<double>;

// ─────────────────────────────────────────────
//  Bin
// ─────────────────────────────────────────────
struct Bin {
    std::vector<int> items;
    int              load{0};

    bool fits(int sz, int cap) const { return load + sz <= cap; }
    void addItem   (int i, int sz) { items.push_back(i); load += sz; }
    void removeItem(int i, int sz) {
        items.erase(std::remove(items.begin(), items.end(), i), items.end());
        load -= sz;
    }
};

// ─────────────────────────────────────────────
//  Solução
// ─────────────────────────────────────────────
struct Solution {
    std::vector<Bin> bins;

    // Função objetivo: minimizar k = número de bins não-vazios
    int evaluate() const {
        int c = 0; for (const auto& b : bins) if (!b.items.empty()) ++c;
        return c;
    }

    void compact() {
        bins.erase(
            std::remove_if(bins.begin(), bins.end(),
                           [](const Bin& b){ return b.items.empty(); }),
            bins.end());
    }
};

// ─────────────────────────────────────────────
//  Instância
// ─────────────────────────────────────────────
struct Instance {
    std::string      name;
    int              n, capacity, k_opt;
    std::vector<int> sizes;
};

// ─────────────────────────────────────────────
//  Leitura — formato OR-Library / binpack
// ─────────────────────────────────────────────
std::vector<Instance> readFile(const std::string& fn) {
    std::ifstream f(fn);
    if (!f) throw std::runtime_error("Não foi possível abrir: " + fn);
    int T; f >> T;
    std::vector<Instance> v; v.reserve(T);
    for (int t = 0; t < T; ++t) {
        Instance inst;
        f >> inst.name >> inst.capacity >> inst.n >> inst.k_opt;
        inst.sizes.resize(inst.n);
        for (int i = 0; i < inst.n; ++i) f >> inst.sizes[i];
        v.push_back(std::move(inst));
    }
    return v;
}

// ─────────────────────────────────────────────
//  Solução inicial — Best Fit Decreasing
// ─────────────────────────────────────────────
Solution buildBFD(const Instance& inst) {
    std::vector<int> ord(inst.n); std::iota(ord.begin(), ord.end(), 0);
    std::sort(ord.begin(), ord.end(),
              [&](int a, int b){ return inst.sizes[a] > inst.sizes[b]; });
    Solution sol;
    for (int idx : ord) {
        int sz = inst.sizes[idx];
        int bestB = -1, bestL = -1;
        for (int b = 0; b < (int)sol.bins.size(); ++b)
            if (sol.bins[b].fits(sz, inst.capacity) && sol.bins[b].load > bestL)
                { bestL = sol.bins[b].load; bestB = b; }
        if (bestB < 0) { Bin nb; nb.addItem(idx,sz); sol.bins.push_back(nb); }
        else             sol.bins[bestB].addItem(idx,sz);
    }
    return sol;
}

// ─────────────────────────────────────────────
//  Busca Local — Relocate + Swap (First Improvement)
//
//  Duas fases:
//  Fase 1 — Relocate puro: aceita apenas se esvazia bin (Δf = -1)
//  Fase 2 — Swap neutro  : troca itens entre bins para VIABILIZAR
//            futuros relocates (aceita se melhora carga sem piorar f;
//            se esvaziar bin, melhor ainda)
// ─────────────────────────────────────────────
bool localSearch(Solution& sol, const Instance& inst) {
    bool improved = false;
    int  nb       = (int)sol.bins.size();

    // ── Fase 1: Relocate (melhora estrita) ────────────────────────────
    for (int src = 0; src < nb && !improved; ++src) {
        if (sol.bins[src].items.empty()) continue;
        auto snap = sol.bins[src].items;

        for (int item : snap) {
            int sz = inst.sizes[item];
            for (int dst = 0; dst < nb && !improved; ++dst) {
                if (dst == src || !sol.bins[dst].fits(sz, inst.capacity)) continue;
                sol.bins[src].removeItem(item, sz);
                sol.bins[dst].addItem(item, sz);
                if (sol.bins[src].items.empty()) {
                    sol.compact(); improved = true; nb = (int)sol.bins.size();
                } else {
                    sol.bins[dst].removeItem(item, sz);
                    sol.bins[src].addItem(item, sz);
                }
            }
        }
    }

    // ── Fase 2: Swap neutro — reorganiza cargas ────────────────────────
    // Aceita troca se pelo menos um bin fica mais "vazio" E a troca é
    // viável. Esvaziar bin aceita imediatamente (melhora estrita).
    if (!improved) {
        for (int i = 0; i < nb && !improved; ++i) {
            for (int j = i+1; j < nb && !improved; ++j) {
                auto si = sol.bins[i].items;
                auto sj = sol.bins[j].items;
                for (int ai : si) {
                    for (int bj : sj) {
                        int sza = inst.sizes[ai], szb = inst.sizes[bj];
                        int li  = sol.bins[i].load - sza + szb;
                        int lj  = sol.bins[j].load - szb + sza;
                        if (li > inst.capacity || lj > inst.capacity) continue;

                        // Aplica swap
                        sol.bins[i].removeItem(ai,sza); sol.bins[j].removeItem(bj,szb);
                        sol.bins[i].addItem(bj,szb);    sol.bins[j].addItem(ai,sza);

                        if (sol.bins[i].items.empty() || sol.bins[j].items.empty()) {
                            // Bin esvaziado — melhora real
                            sol.compact(); improved = true; nb = (int)sol.bins.size();
                        } else if (li < sol.bins[i].load || lj < sol.bins[j].load) {
                            // Swap neutro aceito: libera espaço para futuros relocates
                            // (não desfaz — mantém a reconfiguração)
                            improved = false; // não conta como melhora de f, mas continua
                        } else {
                            // Desfaz
                            sol.bins[i].removeItem(bj,szb); sol.bins[j].removeItem(ai,sza);
                            sol.bins[i].addItem(ai,sza);    sol.bins[j].addItem(bj,szb);
                        }
                        if (improved) break;
                    }
                    if (improved) break;
                }
            }
        }
    }

    return improved;
}

// ─────────────────────────────────────────────
//  Perturbação ILS — move aleatoriamente p itens
//  entre bins distintos (sem respeitar melhora)
// ─────────────────────────────────────────────
void perturb(Solution& sol, const Instance& inst,
             std::mt19937& rng, int strength = 3) {
    int nb = (int)sol.bins.size();
    if (nb < 2) return;

    std::uniform_int_distribution<int> binDist(0, nb-1);

    for (int k = 0; k < strength; ++k) {
        // Escolhe bin fonte não-vazio
        int src = binDist(rng);
        int tries = 0;
        while (sol.bins[src].items.empty() && ++tries < nb*2)
            src = binDist(rng);
        if (sol.bins[src].items.empty()) continue;

        // Escolhe item aleatório no bin fonte
        std::uniform_int_distribution<int> itemDist(
            0, (int)sol.bins[src].items.size()-1);
        int itemPos = itemDist(rng);
        int item    = sol.bins[src].items[itemPos];
        int sz      = inst.sizes[item];

        // Tenta inserir em bin destino aleatório
        int dst = binDist(rng);
        int t2  = 0;
        while ((dst == src || !sol.bins[dst].fits(sz, inst.capacity)) && ++t2 < nb*2)
            dst = binDist(rng);
        if (dst == src || !sol.bins[dst].fits(sz, inst.capacity)) {
            // Abre bin novo para garantir viabilidade
            Bin nb2; nb2.addItem(item, sz);
            sol.bins[src].removeItem(item, sz);
            sol.bins.push_back(nb2);
        } else {
            sol.bins[src].removeItem(item, sz);
            sol.bins[dst].addItem(item, sz);
        }
    }
    sol.compact();
}

// ─────────────────────────────────────────────
//  Resolve uma instância (BFD + BL + ILS)
// ─────────────────────────────────────────────
struct Result {
    std::string name;
    int         k_found, k_opt, iters, ils_restarts;
    double      gap_pct, elapsed;
    Solution    sol;
};

Result solve(const Instance& inst, double timeLimit, unsigned seed = 42) {
    TimePoint start = Clock::now();
    std::mt19937 rng(seed);

    // Solução inicial BFD
    Solution best = buildBFD(inst);
    best.compact();
    int bestK = best.evaluate();

    Solution curr = best;
    int iter = 0, restarts = 0;

    while (true) {
        double elapsed = Seconds(Clock::now() - start).count();
        if (elapsed >= timeLimit) break;

        bool imp = localSearch(curr, inst);
        ++iter;

        if (imp) {
            int k = curr.evaluate();
            if (k < bestK) { bestK = k; best = curr; }
        } else {
            // Ótimo local → perturba (ILS) se ainda há tempo
            double el2 = Seconds(Clock::now() - start).count();
            if (el2 >= timeLimit) break;

            // Perturbação: intensidade cresce levemente com reinícios
            int strength = 3 + (restarts % 5);
            curr = best;  // reinicia do melhor encontrado
            perturb(curr, inst, rng, strength);
            ++restarts;
        }
    }

    double elapsed = Seconds(Clock::now() - start).count();
    double gap     = 100.0 * (bestK - inst.k_opt) / (double)inst.k_opt;
    return { inst.name, bestK, inst.k_opt, iter, restarts, gap, elapsed, best };
}

// ─────────────────────────────────────────────
//  Impressão detalhada
// ─────────────────────────────────────────────
void printDetail(const Result& r, const Instance& inst) {
    std::cout << "\n  ── Detalhe dos bins: " << r.name << " ──\n";
    for (int b = 0; b < (int)r.sol.bins.size(); ++b) {
        const Bin& bin = r.sol.bins[b];
        std::cout << "  Bin " << std::setw(3) << b+1
                  << " [" << std::setw(3) << bin.load
                  << "/" << inst.capacity << "] ";
        for (int idx : bin.items) std::cout << inst.sizes[idx] << " ";
        std::cout << "\n";
    }
}

// ─────────────────────────────────────────────
//  Tabela resumo
// ─────────────────────────────────────────────
void printTable(const std::vector<Result>& res) {
    std::cout << "\n";
    std::cout << "╔══════════════╦═════════╦═══════╦═══════════╦══════════╦════════╦══════════╗\n";
    std::cout << "║  Instância   ║ k_found ║ k_opt ║   Gap(%)  ║ Tempo(s) ║  Iters ║ Restarts ║\n";
    std::cout << "╠══════════════╬═════════╬═══════╬═══════════╬══════════╬════════╬══════════╣\n";

    int sf = 0, so = 0; double st = 0;
    for (const auto& r : res) {
        std::string tag = (r.k_found == r.k_opt) ? " ✓" : "  ";
        std::cout << "║ " << std::left  << std::setw(12) << r.name
                  << " ║ " << std::right << std::setw(7) << r.k_found
                  << " ║ "  << std::setw(5) << r.k_opt
                  << " ║ "  << std::setw(8) << std::fixed << std::setprecision(2)
                  << r.gap_pct << "%" << tag
                  << " ║ "  << std::setw(8) << std::setprecision(4) << r.elapsed
                  << " ║ "  << std::setw(6) << r.iters
                  << " ║ "  << std::setw(8) << r.ils_restarts << " ║\n";
        sf += r.k_found; so += r.k_opt; st += r.elapsed;
    }
    double avg = 100.0*(sf-so)/(double)so;
    int otimos = 0;
    for (const auto& r : res) if (r.k_found == r.k_opt) ++otimos;
    std::cout << "╠══════════════╩═════════╩═══════╩═══════════╩══════════╩════════╩══════════╣\n";
    std::cout << std::fixed;
    std::cout << "║ Ótimos encontrados: " << std::setw(2) << otimos << "/" << res.size()
              << "  |  k_total=" << sf << " (opt=" << so << ")"
              << "  |  gap médio=" << std::setprecision(2) << avg << "%"
              << "  |  t=" << std::setprecision(3) << st << "s  ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════════╝\n\n";
}

// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <arquivo> <tempo_s> [índice]\n";
        return EXIT_FAILURE;
    }
    const std::string fn   = argv[1];
    const double      tlim = std::stod(argv[2]);
    const int         solo = (argc >= 4) ? std::stoi(argv[3]) : -1;

    std::vector<Instance> insts;
    try { insts = readFile(fn); }
    catch (const std::exception& e) { std::cerr << e.what() << "\n"; return EXIT_FAILURE; }

    std::cout << "Arquivo : " << fn << " | " << insts.size()
              << " instâncias | tlim=" << tlim << "s\n";
    std::cout << std::string(70, '─') << "\n";

    int i0 = (solo >= 0) ? solo     : 0;
    int i1 = (solo >= 0) ? solo + 1 : (int)insts.size();

    // Distribui o tempo igualmente entre as instâncias
    double tpp = (i1 - i0 > 0) ? tlim / (i1 - i0) : tlim;

    std::vector<Result> results;
    for (int i = i0; i < i1; ++i) {
        const Instance& inst = insts[i];
        std::cout << "[" << std::setw(2) << i << "] "
                  << std::left << std::setw(12) << inst.name
                  << " n=" << inst.n << " cap=" << inst.capacity
                  << " k*=" << inst.k_opt << " ... " << std::flush;

        Result r = solve(inst, tpp, 42u + (unsigned)i);
        results.push_back(r);
        std::cout << "k=" << r.k_found
                  << (r.k_found == r.k_opt ? " ✓" : "  ")
                  << " gap=" << std::fixed << std::setprecision(1) << r.gap_pct << "%"
                  << " t="   << std::setprecision(4) << r.elapsed  << "s"
                  << " restarts=" << r.ils_restarts << "\n";
    }

    printTable(results);

    // Detalhe se instância única
    if (solo >= 0 && !results.empty())
        printDetail(results[0], insts[solo]);

    return EXIT_SUCCESS;
}