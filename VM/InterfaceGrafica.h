#ifndef INTERFACEGRAFICA_H
#define INTERFACEGRAFICA_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QTextEdit>
#include <map>
#include <vector>
#include "Maquina_melhor.h"
#include "CPU.h"
#include "Montador.h"

/*
=========================================================================================
Janela principal, Contém a ligação entre a VM e widgets Qt que exibem o estado.
=========================================================================================
*/
class InterfaceGrafica : public QMainWindow
{
    Q_OBJECT

public:
    /*
    =========================================================================================
    Construtor da interface.
    ==========================================================================================
    */
    explicit InterfaceGrafica(QWidget *parent = nullptr);
    ~InterfaceGrafica() override = default;

    /*
    =========================================================================================
    Slots públicos chamados por eventos de widgets.
    Cada slot corresponde a uma ação do usuário na interface: carregar, executar,reset,
    passo, montador, carregador e ligador
    =========================================================================================
    */
private slots:
    void carregarPrograma_clicked();
    void executar_clicked();
    void passo_clicked();
    void mostrarCodigoFonte_clicked();
    void macros_clicked();
    void montador_clicked();
    void carregador_clicked();
    void ligador_clicked();
    void reset_clicked();

private:
    Maquina vm;
    Montador montador;
    
    /* widgets Qt principais usados pela janela */
    QPushButton *btnCarregar;
    QPushButton *btnExecutar;
    QPushButton *btnPasso;
    QPushButton *btnMacros;
    QPushButton *btnMontador;
    QPushButton *btnCarregador;
    QPushButton *btnLigador;
    QPushButton *btnReset;
    QTableWidget *tblRegistradores;
    QTableWidget *tblMemoria;
    std::string caminhoArquivoAtual;
    
    /* funçoes auxiliares e atualização da UI */
    void configurarLayout();
    void configurarRegistradores();
    void configurarMemoria();
    void atualizarRegistradores();
    void atualizarMemoria();
    void mostrarMensagem(const QString& titulo, const QString& mensagem);
    bool carregarBinario(const std::string& caminho);
    void resetVM();
};

#endif 