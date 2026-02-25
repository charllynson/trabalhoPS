#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFont>
#include <QTimer>
#include <QListWidget>

#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>

#include "MontadorSemVibecode.h"
#include "Ligador.h"
#include "Maquina_melhor.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAbrirFonte();
    void onAdicionarModulo();
    void onRemoverModulo();
    void onMontar();
    void onLinkar();
    void onExecutarTudo();
    void onPasso();
    void onReset();

private:
    // ── Widgets ───────────────────────────────
    // Painel esquerdo: editor + módulos
    QPlainTextEdit* editorFonte;
    QListWidget*    listaModulos;       // lista de arquivos .txt adicionados
    QLabel*         lblArquivoAtual;

    // Painel central: registradores + memória
    QTableWidget*   tabelaRegistradores;
    QTableWidget*   tabelaMemoria;
    QLineEdit*      editMemAddr;        // endereço inicial p/ visualizar memória

    // Painel inferior: log + obj
    QPlainTextEdit* logExecucao;
    QPlainTextEdit* viewObjeto;

    // Botões
    QPushButton* btnAbrir;
    QPushButton* btnAddModulo;
    QPushButton* btnRemModulo;
    QPushButton* btnMontar;
    QPushButton* btnLinkar;
    QPushButton* btnExecutar;
    QPushButton* btnPasso;
    QPushButton* btnReset;

    QLineEdit* editProgAddr;

    // ── Backend ───────────────────────────────
    MontadorSemVibecode montador;
    Ligador             ligador;
    std::unique_ptr<Maquina> maquina;

    std::vector<std::string> fontes;     // caminhos dos .txt
    std::vector<std::string> objetos;    // caminhos dos .obj gerados
    bool programaCarregado = false;
    bool programaTerminou  = false;

    // ── Helpers ───────────────────────────────
    void atualizarRegistradores();
    void atualizarMemoria();
    void appendLog(const QString& msg);
    void setupUI();
    QString formatHex(uint32_t val, int w);
};
