#include "InterfaceGrafica.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QColor>


InterfaceGrafica::InterfaceGrafica(QWidget *parent)
    : QMainWindow(parent), vm(131072)
{
    configurarLayout();
    setWindowTitle("SIC/XE Virtual Machine");
    
    resize(800, 600);
    
    atualizarRegistradores();
    atualizarMemoria();
}

void InterfaceGrafica::configurarLayout()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    // --- Controles (Linha 0) ---
    btnCarregar = new QPushButton("Carregar Programa");
    btnExecutar = new QPushButton("Executar");
    btnPasso = new QPushButton("Passo");
    
    // Conexões de Slots
    connect(btnCarregar, &QPushButton::clicked, this, &InterfaceGrafica::carregarPrograma_clicked);
    connect(btnExecutar, &QPushButton::clicked, this, &InterfaceGrafica::executar_clicked);
    connect(btnPasso, &QPushButton::clicked, this, &InterfaceGrafica::passo_clicked);

    mainLayout->addWidget(btnCarregar, 0, 0);
    mainLayout->addWidget(btnExecutar, 0, 1);
    mainLayout->addWidget(btnPasso, 0, 2);

    // --- Registradores (Linha 1 e 2) ---
    configurarRegistradores();
    mainLayout->addWidget(new QLabel("Registradores (Hex/Dec):"), 1, 0, 1, 3);
    mainLayout->addWidget(tblRegistradores, 2, 0, 1, 3);

    // --- Memória (Linha 3 e 4) ---
    configurarMemoria();
    mainLayout->addWidget(new QLabel("Memória:"), 3, 0, 1, 3);
    mainLayout->addWidget(tblMemoria, 4, 0, 1, 3);
    
    mainLayout->setRowStretch(4, 1); 
}

void InterfaceGrafica::configurarRegistradores()
{
    tblRegistradores = new QTableWidget(2, 8); 
    tblRegistradores->setHorizontalHeaderLabels({"A", "X", "L", "B", "S", "T", "PC", "SW"});
    tblRegistradores->setVerticalHeaderLabels({"Hex", "Dec"});
    tblRegistradores->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblRegistradores->setMinimumHeight(120);

    for(int row = 0; row < 2; ++row) {
        for(int col = 0; col < 8; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); 
            tblRegistradores->setItem(row, col, item);
        }
    }
}

void InterfaceGrafica::configurarMemoria()
{
    const size_t tamanho_bytes = vm.getMemoria().getTamanhoBytes();
    const size_t palavras = tamanho_bytes / 3;
    
    tblMemoria = new QTableWidget(palavras, 2); 
    tblMemoria->setHorizontalHeaderLabels({"Endereço", "Valor"});
    tblMemoria->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblMemoria->verticalHeader()->setVisible(false);
    
    for (size_t i = 0; i < palavras; ++i) {
        QTableWidgetItem *addrItem = new QTableWidgetItem(QString("0x%1").arg(i * 3, 4, 16, QChar('0')).toUpper());
        addrItem->setFlags(addrItem->flags() & ~Qt::ItemIsEditable);

        QTableWidgetItem *valItem = new QTableWidgetItem("0x000000");
        valItem->setFlags(valItem->flags() & ~Qt::ItemIsEditable);
        
        tblMemoria->setItem(i, 0, addrItem);
        tblMemoria->setItem(i, 1, valItem);
    }
}

// Implementações dos Slots
void InterfaceGrafica::carregarPrograma_clicked()
{
    QString caminhoArquivo = QFileDialog::getOpenFileName(this, "Carregar Programa Binário", "", "Arquivos Binários (*.bin);;Todos os Arquivos (*)");

    if (!caminhoArquivo.isEmpty()) {
        try {
            vm.carregarPrograma(caminhoArquivo.toStdString());
            atualizarRegistradores();
            atualizarMemoria();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Erro de Carregamento", QString("Erro ao carregar o programa: %1").arg(e.what()));
        }
    }
}

void InterfaceGrafica::executar_clicked()
{
    try {
        vm.executar();
        atualizarRegistradores();
        atualizarMemoria();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Erro de Execução", QString("Erro durante a execução: %1").arg(e.what()));
    }
}

void InterfaceGrafica::passo_clicked()
{
    try {
        vm.passo();
        atualizarRegistradores();
        atualizarMemoria();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Erro de Passo", QString("Erro durante o passo: %1").arg(e.what()));
    }
}

// Funções de atualização da UI (inalteradas)
void InterfaceGrafica::atualizarRegistradores()
{
    Registradores& r = vm.getCPU().r;

    std::vector<std::pair<std::int32_t*, int>> regs = {
        {&r.A, 0}, {&r.X, 1}, {&r.L, 2}, {&r.B, 3},
        {&r.S, 4}, {&r.T, 5}, {&r.PC, 6}
    };

    // 1. Atualiza A, X, L, B, S, T, PC
    for (const auto& pair : regs) {
        std::int32_t valor = *pair.first;
        int col = pair.second;
        
        // Valor Hexadecimal (Linha 0)
        QString hexVal = QString("0x%1").arg(valor, 6, 16, QChar('0')).toUpper();
        tblRegistradores->item(0, col)->setText(hexVal);

        // Valor Decimal (Linha 1)
        QString decVal = QString::number(valor);
        tblRegistradores->item(1, col)->setText(decVal);
    }

    // 2. Atualiza SW (Palavra de Status)
    int sw_col = 7;
    QString sw_str;
    switch (r.SW) {
        case EQUAL: sw_str = "EQUAL (0)"; break;
        case BIGGER: sw_str = "BIGGER (1)"; break;
        case SMALLER: sw_str = "SMALLER (-1)"; break;
    }
    tblRegistradores->item(0, sw_col)->setText(sw_str);
    tblRegistradores->item(1, sw_col)->setText(QString::number(r.SW));
}


void InterfaceGrafica::atualizarMemoria()
{
    const std::vector<std::uint8_t>& m_bytes = vm.getMemoria().getMBytes();
    const size_t tamanho_bytes = m_bytes.size();
    const size_t palavras = tamanho_bytes / 3;
    const std::int32_t pc_atual = vm.getCPU().r.PC;
    
    QPalette palette = tblMemoria->palette();
    QColor corFundoPadrao = palette.color(QPalette::Base);
    QColor corDestaque = QColor(170, 200, 255); 


    for (size_t i = 0; i < palavras; ++i) {
        size_t byte_addr = i * 3;
        
        std::uint32_t palavra = 0;
        if (byte_addr + 2 < tamanho_bytes) {
             palavra = (m_bytes[byte_addr] << 16) | 
                       (m_bytes[byte_addr + 1] << 8) | 
                        m_bytes[byte_addr + 2];
        }
        
        QString valStr = QString("0x%1").arg(palavra, 6, 16, QChar('0')).toUpper();
        tblMemoria->item(i, 1)->setText(valStr);

        bool isPC = (byte_addr == pc_atual);
        
        for (int col = 0; col < tblMemoria->columnCount(); ++col) {
            if (isPC) {
                tblMemoria->item(i, col)->setBackground(corDestaque);
            } else {
                tblMemoria->item(i, col)->setBackground(corFundoPadrao); 
            }
        }
    }

    if (pc_atual >= 0 && pc_atual < tamanho_bytes) {
        int row = pc_atual / 3;
        tblMemoria->scrollTo(tblMemoria->model()->index(row, 0));
    }
}