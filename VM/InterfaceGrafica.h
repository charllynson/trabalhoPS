#ifndef INTERFACEGRAFICA_H
#define INTERFACEGRAFICA_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <map>
#include <vector>
#include "Maquina_melhor.h" 
#include "CPU.h"

// Usamos QMainWindow como a classe base da GUI
class InterfaceGrafica : public QMainWindow
{
    Q_OBJECT // Macro obrigatória para classes que definem signals e slots

public:
    explicit InterfaceGrafica(QWidget *parent = nullptr);
    ~InterfaceGrafica() override = default;

private slots:
    // Slots para as funções de controle
    void carregarPrograma_clicked();
    void executar_clicked();
    void passo_clicked();

private:
    Maquina vm; // Instância da máquina virtual

    // Componentes da Interface
    QPushButton *btnCarregar;
    QPushButton *btnExecutar;
    QPushButton *btnPasso;
    QTableWidget *tblRegistradores;
    QTableWidget *tblMemoria;

    // Funções de configuração e atualização
    void configurarLayout();
    void configurarRegistradores();
    void configurarMemoria();
    void atualizarRegistradores();
    void atualizarMemoria();
};

#endif // INTERFACEGRAFICA_H