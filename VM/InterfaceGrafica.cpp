#include "InterfaceGrafica.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QColor>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QFile>
#include <QTextStream>

/*
=========================================================================================
Configura o layout e inicializa as visualizações dos registradores 
e da memória do estado da VM
=========================================================================================
*/
InterfaceGrafica::InterfaceGrafica(QWidget *parent)
    : QMainWindow(parent), vm(131072)
{
    configurarLayout();

    {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53,53,53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(42,42,42));
        darkPalette.setColor(QPalette::AlternateBase, QColor(66,66,66));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53,53,53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42,130,218));
        darkPalette.setColor(QPalette::Highlight, QColor(42,130,218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(darkPalette);
        qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }

    setWindowTitle("SIC/XE Virtual Machine");

    resize(1000, 700);

    atualizarRegistradores();
    atualizarMemoria();
}

void InterfaceGrafica::configurarLayout()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    /*
    =========================================================================================
    Area de botões de execução da VM (Carregar, Executar, Passo, Reset)
    =========================================================================================
    */
    QHBoxLayout *controlesLayout = new QHBoxLayout();

    btnCarregar = new QPushButton("Carregar");
    btnExecutar = new QPushButton("Executar");
    btnPasso = new QPushButton("Passo");
    btnReset = new QPushButton("Reset");

    connect(btnCarregar, &QPushButton::clicked, this, &InterfaceGrafica::carregarPrograma_clicked);
    connect(btnExecutar, &QPushButton::clicked, this, &InterfaceGrafica::executar_clicked);
    connect(btnPasso, &QPushButton::clicked, this, &InterfaceGrafica::passo_clicked);
    connect(btnReset, &QPushButton::clicked, this, &InterfaceGrafica::reset_clicked);

    controlesLayout->addStretch();
    controlesLayout->addWidget(btnCarregar);
    controlesLayout->addWidget(btnExecutar);
    controlesLayout->addWidget(btnPasso);
    controlesLayout->addWidget(btnReset);

    mainLayout->addLayout(controlesLayout, 0, 0, 1, 1);

    /*
    =========================================================================================
    area de controle dos registradores, da memória e dos botões de ação
    (macros/montador/carregador/ligador) 
    =========================================================================================
    */
    QVBoxLayout *leftLayout = new QVBoxLayout();

    configurarRegistradores();
    QGroupBox *gbRegistradores = new QGroupBox("Registradores (Hex/Dec)");
    QHBoxLayout *regHBox = new QHBoxLayout();
    regHBox->addWidget(tblRegistradores);

    QWidget *buttonsWidget = new QWidget();

    btnMacros = new QPushButton("Macros");
    btnMontador = new QPushButton("Montador");
    btnCarregador = new QPushButton("Carregador");
    btnLigador = new QPushButton("Ligador");

    QList<QPushButton*> actionButtons = {btnMacros, btnMontador, btnCarregador, btnLigador};
    for (QPushButton* btn : actionButtons) {
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        btn->setFixedSize(80, 80);
        QFont font = btn->font();
        font.setPointSize(10);
        btn->setFont(font);
        btn->setStyleSheet(
            "QPushButton {"
            " background-color: #CFCFCF;"
            " color: #111111;"
            " border: none;"
            " border-radius: 12px;"
            "}" 
            "QPushButton:hover { background-color: #BFBFBF; }"
            "QPushButton:pressed { background-color: #AFAFAF; }"
        );
        btn->setFlat(false);
    }

    QHBoxLayout *hButtons = new QHBoxLayout();
    hButtons->setSpacing(10);
    hButtons->setContentsMargins(4,4,4,4);
    hButtons->addStretch();
    hButtons->addWidget(btnMacros);
    hButtons->addWidget(btnMontador);
    hButtons->addWidget(btnCarregador);
    hButtons->addWidget(btnLigador);
    hButtons->addStretch();

    QVBoxLayout *vCenter = new QVBoxLayout();
    vCenter->addStretch();
    vCenter->addLayout(hButtons);
    vCenter->addStretch();
    buttonsWidget->setLayout(vCenter);

    regHBox->addWidget(buttonsWidget);
    regHBox->setAlignment(buttonsWidget, Qt::AlignVCenter);
    gbRegistradores->setLayout(regHBox);

        gbRegistradores->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        int regRowHeight = 22; // pixels per register row
        int headerHeight = 28;
        int totalRegHeight = headerHeight + (regRowHeight * 8);
    gbRegistradores->setFixedHeight(totalRegHeight);
    tblRegistradores->setMinimumWidth(160);
    tblRegistradores->setMaximumWidth(220);
    for (int r = 0; r < 8; ++r) tblRegistradores->setRowHeight(r, regRowHeight);

    leftLayout->addWidget(gbRegistradores);


    configurarMemoria();
    QGroupBox *gbMemoria = new QGroupBox("Memória");
    QVBoxLayout *memLayout = new QVBoxLayout();
    memLayout->addWidget(tblMemoria);
    gbMemoria->setLayout(memLayout);
    leftLayout->addWidget(gbMemoria);
    
    mainLayout->addLayout(leftLayout, 1, 0, 1, 1);

    // Conectar ações (apenas Montador terá funcionalidade completa)
    connect(btnMacros, &QPushButton::clicked, this, &InterfaceGrafica::macros_clicked);
    connect(btnMontador, &QPushButton::clicked, this, &InterfaceGrafica::montador_clicked);
    connect(btnCarregador, &QPushButton::clicked, this, &InterfaceGrafica::carregador_clicked);
    connect(btnLigador, &QPushButton::clicked, this, &InterfaceGrafica::ligador_clicked);
}

void InterfaceGrafica::configurarRegistradores()
{
    tblRegistradores = new QTableWidget(8, 2);
    tblRegistradores->setVerticalHeaderLabels({"A", "X", "L", "B", "S", "T", "PC", "SW"});
    tblRegistradores->setHorizontalHeaderLabels({"Hex", "Dec"});
    tblRegistradores->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblRegistradores->setMinimumWidth(220);

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 2; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            tblRegistradores->setItem(row, col, item);
        }
    }
}

/*
=========================================================================================
configurarMemoria()
Cria a QTableWidget que representa a memória da VM. Cada linha corresponde a uma
palavra de 3 bytes em formato nixpbe, exibindo o endereço e os valores atuais. 
=========================================================================================
*/
void InterfaceGrafica::configurarMemoria()
{
    const size_t tamanho_bytes = vm.getMemoria().getTamanhoBytes();
    const size_t palavras = tamanho_bytes / 3;
    
    tblMemoria = new QTableWidget(palavras, 2); 
    tblMemoria->setHorizontalHeaderLabels({"Endereço", "Valores"});
    tblMemoria->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblMemoria->verticalHeader()->setVisible(false);
            connect(btnCarregador, &QPushButton::clicked, this, &InterfaceGrafica::carregador_clicked);
    for (size_t i = 0; i < palavras; ++i) {
        QTableWidgetItem *addrItem = new QTableWidgetItem(QString::number(i * 3).rightJustified(4, '0'));
        addrItem->setFlags(addrItem->flags() & ~Qt::ItemIsEditable);

        QTableWidgetItem *valItem = new QTableWidgetItem("0x000000");
        valItem->setFlags(valItem->flags() & ~Qt::ItemIsEditable);
        
        tblMemoria->setItem(i, 0, addrItem);
        tblMemoria->setItem(i, 1, valItem);
    }
}

/*
=========================================================================================
Abre uma caixa nos arquivos para o usuário selecionar um arquivo binário e tenta
carregá-lo na VM
--- depois do processador de macros preciso ver qual formato vamos escolher para entrada 
=========================================================================================
*/
void InterfaceGrafica::carregarPrograma_clicked()
{
    QString filtro = "Arquivos Binário (*.obj *.bin);;Todos os Arquivos (*)";
    QString titulo = "Carregar Programa Binário";

    QString caminhoArquivo = QFileDialog::getOpenFileName(this, titulo, "", filtro);
    if (!caminhoArquivo.isEmpty()) {
        caminhoArquivoAtual = caminhoArquivo.toStdString();
        bool sucesso = carregarBinario(caminhoArquivoAtual);
        if (sucesso) {
            atualizarRegistradores();
            atualizarMemoria();
        }
    }
}

/*
=========================================================================================
carregarBinario()
delega a leitura do binário e trata exceções.
=========================================================================================
*/
bool InterfaceGrafica::carregarBinario(const std::string& caminho)
{
    try {
        vm.carregarPrograma(caminho);
        mostrarMensagem("Sucesso", "Arquivo carregado com sucesso!");
        return true;
    } catch (const std::exception& e) {
        mostrarMensagem("Erro", QString("Erro ao carregar arquivo: %1").arg(e.what()));
        return false;
    }
}
/*
=========================================================================================
Abre um arquivo fonte assembly, chama o montador para gerar o objeto na memória
e grava temporariamente o binário antes de fazer o carregamento para a VM
=========================================================================================
*/
void InterfaceGrafica::montador_clicked()
{
    QString filtro = "Arquivos Fonte (*.asm *.txt);;Todos os Arquivos (*)";
    QString titulo = "Montador - Abrir Fonte";

    QString caminhoArquivo = QFileDialog::getOpenFileName(this, titulo, "", filtro);
    if (caminhoArquivo.isEmpty()) return;

    try {
        std::vector<uint8_t> codigoObjeto = montador.montar(caminhoArquivo.toStdString());
        if (codigoObjeto.empty()) {
            mostrarMensagem("Erro", "Falha na montagem - código objeto vazio.");
            return;
        }

        QString tempPath = QDir::tempPath() + "/sic_temp.obj";
        QFile tempFile(tempPath);
        if (!tempFile.open(QIODevice::WriteOnly)) {
            mostrarMensagem("Erro", "Não foi possível criar arquivo temporário.");
            return;
        }
        tempFile.write(reinterpret_cast<const char*>(codigoObjeto.data()), codigoObjeto.size());
        tempFile.close();

        vm.carregarPrograma(tempPath.toStdString());
        mostrarMensagem("Sucesso", QString("Montagem e carregamento concluídos: %1 bytes").arg(codigoObjeto.size()));
        atualizarRegistradores();
        atualizarMemoria();
    } catch (const std::exception& e) {
        mostrarMensagem("Erro", QString("Erro durante montagem: %1").arg(e.what()));
    }
}

/*
=========================================================================================
base para os proximaos checkpoints
=========================================================================================
*/
void InterfaceGrafica::macros_clicked() { }
void InterfaceGrafica::carregador_clicked() { }
void InterfaceGrafica::ligador_clicked() { }

void InterfaceGrafica::reset_clicked()
{
    resetVM();
}

/*
=========================================================================================
zera toda a memória e reinicializa os registradores para um estado limpo
=========================================================================================
*/
void InterfaceGrafica::resetVM()
{
    std::size_t tamanho = vm.getMemoria().getTamanhoBytes();
    for (std::size_t i = 0; i < tamanho; ++i) {
        vm.getMemoria().setByte(i, 0);
    }

    auto& r = vm.getCPU().r;
    r.A = 0;
    r.X = 0;
    r.L = 0;
    r.B = 0;
    r.S = 0;
    r.T = 0;
    r.PC = 0;
    r.SW = EQUAL;

    atualizarRegistradores();
    atualizarMemoria();
}


/*
=========================================================================================
execução completa e passo a passo
=========================================================================================
*/
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

/*
=========================================================================================
mostrarCodigoFonte_clicked()
usado quando a ação de mostrar o código-fonte não está disponível
=========================================================================================
*/
void InterfaceGrafica::mostrarCodigoFonte_clicked()
{
    mostrarMensagem("Informação", "Mostrar código fonte não implementado neste estado.");
}

/*
=========================================================================================
mostrarMensagem()
exibição de diálogos de informação ao usuário.
=========================================================================================
*/
void InterfaceGrafica::mostrarMensagem(const QString& titulo, const QString& mensagem)
{
    QMessageBox::information(this, titulo, mensagem);
}

/*
=========================================================================================
atualizarRegistradores()
le os valores atuais dos registradores da `Maquina` e atualiza a tabela da interface.
Exibe cada registrador em hexadecimal e decimal; trata SW (status word) separadamente.
=========================================================================================
*/
void InterfaceGrafica::atualizarRegistradores()
{
    Registradores& r = vm.getCPU().r;
    std::vector<std::int32_t*> regs = {&r.A, &r.X, &r.L, &r.B, &r.S, &r.T, &r.PC};

    for (size_t row = 0; row < regs.size(); ++row) {
        std::int32_t valor = *regs[row];
        QString hexVal = QString("0x%1").arg(valor, 6, 16, QChar('0')).toUpper();
        QString decVal = QString::number(valor);
        tblRegistradores->item((int)row, 0)->setText(hexVal);
        tblRegistradores->item((int)row, 1)->setText(decVal);
    }

    int sw_row = 7;
    QString sw_str;
    switch (r.SW) {
        case EQUAL: sw_str = "EQUAL (0)"; break;
        case BIGGER: sw_str = "BIGGER (1)"; break;
        case SMALLER: sw_str = "SMALLER (-1)"; break;
    }
    tblRegistradores->item(sw_row, 0)->setText(sw_str);
    tblRegistradores->item(sw_row, 1)->setText(QString::number(r.SW));
}

/*
=========================================================================================
atualizarMemoria()
Atualiza a visualização da memória a partir dos bytes da Maquina e converte palavras 
de 3 bytes em valores numéricos
=========================================================================================
*/
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
        
        QString valStr = QString::number(palavra).rightJustified(6, '0');
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

    if (pc_atual >= 0 && pc_atual < (std::int32_t)tamanho_bytes) {
        int row = pc_atual / 3;
        tblMemoria->scrollTo(tblMemoria->model()->index(row, 0));
    }
}