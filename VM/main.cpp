#include "MontadorSemVibecode.h"
#include "Ligador.h"
#include "Maquina_melhor.h"
#include <iomanip>

int main(int argc, char *argv[]) {

  auto montador = MontadorSemVibecode();
  auto maquina  = Maquina(16384);

  // ── Teste multi-módulo ──────────────────────────────────
  // Montar módulo A (programa principal)
  montador.montar("../data/teste_modA.txt", "./teste_modA_result.txt");
  // Montar módulo B (sub-rotina)
  montador.montar("../data/teste_modB.txt", "./teste_modB_result.txt");

  // Mostrar os arquivos objeto gerados
  std::cout << "\n===== ARQUIVO OBJETO: teste_modA_result.txt =====\n";
  {
    std::ifstream f("./teste_modA_result.txt");
    std::string line;
    while (std::getline(f, line)) std::cout << line << "\n";
  }
  std::cout << "\n===== ARQUIVO OBJETO: teste_modB_result.txt =====\n";
  {
    std::ifstream f("./teste_modB_result.txt");
    std::string line;
    while (std::getline(f, line)) std::cout << line << "\n";
  }

  // Linkar os dois módulos com PROGADDR = 0x0000
  Ligador ligador;
  if (ligador.LigadorRelocador(
        {"./teste_modA_result.txt", "./teste_modB_result.txt"}, 0x0000)) {

    std::cout << "\n===== IMAGEM DE MEMÓRIA LINKADA =====\n";
    // Imprime os primeiros 0x20 bytes (cobre PROGA + PROGB)
    for (int i = 0; i < 0x20; i++) {
      if (i % 16 == 0)
        std::cout << std::hex << std::setw(4) << std::setfill('0') << i << ": ";
      std::cout << std::hex << std::setw(2) << std::setfill('0')
                << (int)ligador.memoria[i] << " ";
      if (i % 16 == 15) std::cout << "\n";
    }
    std::cout << std::dec << "\n";

    ligador.CarregadorAbsoluto(maquina);
    std::cout << "Carregamento concluído com sucesso.\n";
  } else {
    std::cerr << "Ligação falhou. Carregador não executado.\n";
  }

  maquina.executar();
}