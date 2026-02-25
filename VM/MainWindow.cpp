#include "MainWindow.h"

// ════════════════════════════════════════════════════════════
//  Construtor
// ════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("SIC/XE – Montador · Ligador · Máquina Virtual");
    resize(1200, 750);
    setupUI();

    maquina = std::make_unique<Maquina>(16384);

    // Redirecionar log da máquina para o painel
    maquina->logCallback = [this](const std::string& msg) {
        appendLog(QString::fromStdString(msg));
    };

    atualizarRegistradores();
}

// ════════════════════════════════════════════════════════════
//  Layout
// ════════════════════════════════════════════════════════════

void MainWindow::setupUI() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    QFont mono("Monospace", 10);
    mono.setStyleHint(QFont::Monospace);

    // ── Painel esquerdo: fonte + módulos ──────────────────
    auto* grpFonte = new QGroupBox("Código Fonte");
    auto* layFonte = new QVBoxLayout(grpFonte);

    lblArquivoAtual = new QLabel("Nenhum arquivo aberto");
    editorFonte = new QPlainTextEdit;
    editorFonte->setFont(mono);
    editorFonte->setReadOnly(true);

    auto* layBtnFonte = new QHBoxLayout;
    btnAbrir     = new QPushButton("Abrir");
    btnAddModulo = new QPushButton("+ Módulo");
    btnRemModulo = new QPushButton("− Módulo");
    layBtnFonte->addWidget(btnAbrir);
    layBtnFonte->addWidget(btnAddModulo);
    layBtnFonte->addWidget(btnRemModulo);

    listaModulos = new QListWidget;
    listaModulos->setMaximumHeight(100);

    layFonte->addWidget(lblArquivoAtual);
    layFonte->addWidget(editorFonte, 1);
    layFonte->addLayout(layBtnFonte);
    layFonte->addWidget(new QLabel("Módulos a linkar:"));
    layFonte->addWidget(listaModulos);

    // ── Painel central: registradores + memória ───────────
    auto* grpRegs = new QGroupBox("Registradores");
    auto* layRegs = new QVBoxLayout(grpRegs);

    tabelaRegistradores = new QTableWidget(8, 2);
    tabelaRegistradores->setHorizontalHeaderLabels({"Reg", "Valor"});
    tabelaRegistradores->horizontalHeader()->setStretchLastSection(true);
    tabelaRegistradores->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabelaRegistradores->setFont(mono);
    tabelaRegistradores->setMaximumHeight(280);
    tabelaRegistradores->verticalHeader()->setVisible(false);
    layRegs->addWidget(tabelaRegistradores);

    auto* grpMem = new QGroupBox("Memória");
    auto* layMem = new QVBoxLayout(grpMem);

    auto* layMemAddr = new QHBoxLayout;
    layMemAddr->addWidget(new QLabel("Endereço (hex):"));
    editMemAddr = new QLineEdit("0000");
    editMemAddr->setMaximumWidth(80);
    layMemAddr->addWidget(editMemAddr);
    auto* btnIrMem = new QPushButton("Ir");
    layMemAddr->addWidget(btnIrMem);
    layMemAddr->addStretch();

    tabelaMemoria = new QTableWidget;
    tabelaMemoria->setFont(mono);
    tabelaMemoria->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabelaMemoria->setColumnCount(17); // addr + 16 bytes
    QStringList hdrMem = {"Endereço"};
    for (int i = 0; i < 16; ++i)
        hdrMem << formatHex(i, 2);
    tabelaMemoria->setHorizontalHeaderLabels(hdrMem);
    tabelaMemoria->verticalHeader()->setVisible(false);

    layMem->addLayout(layMemAddr);
    layMem->addWidget(tabelaMemoria, 1);

    auto* rightTop = new QVBoxLayout;
    rightTop->addWidget(grpRegs);
    rightTop->addWidget(grpMem, 1);

    // ── Painel inferior: log + objeto ─────────────────────
    auto* grpLog = new QGroupBox("Log de Execução");
    auto* layLog = new QVBoxLayout(grpLog);
    logExecucao = new QPlainTextEdit;
    logExecucao->setFont(mono);
    logExecucao->setReadOnly(true);
    logExecucao->setMaximumHeight(180);
    layLog->addWidget(logExecucao);

    auto* grpObj = new QGroupBox("Arquivo Objeto");
    auto* layObj = new QVBoxLayout(grpObj);
    viewObjeto = new QPlainTextEdit;
    viewObjeto->setFont(mono);
    viewObjeto->setReadOnly(true);
    viewObjeto->setMaximumHeight(180);
    layObj->addWidget(viewObjeto);

    // ── Botões de ação ────────────────────────────────────
    auto* grpAcoes = new QGroupBox("Ações");
    auto* layAcoes = new QVBoxLayout(grpAcoes);

    auto* layPA = new QHBoxLayout;
    layPA->addWidget(new QLabel("PROGADDR (hex):"));
    editProgAddr = new QLineEdit("0000");
    editProgAddr->setMaximumWidth(80);
    layPA->addWidget(editProgAddr);
    layPA->addStretch();

    btnMontar   = new QPushButton("1. Montar");
    btnLinkar   = new QPushButton("2. Linkar + Carregar");
    btnExecutar = new QPushButton("▶ Executar Tudo");
    btnPasso    = new QPushButton("⏭ Passo");
    btnReset    = new QPushButton("↻ Reset");

    btnLinkar->setEnabled(false);
    btnExecutar->setEnabled(false);
    btnPasso->setEnabled(false);

    layAcoes->addLayout(layPA);
    layAcoes->addWidget(btnMontar);
    layAcoes->addWidget(btnLinkar);
    layAcoes->addWidget(btnExecutar);
    layAcoes->addWidget(btnPasso);
    layAcoes->addWidget(btnReset);
    layAcoes->addStretch();

    // ── Montagem geral ────────────────────────────────────
    auto* mainLayout = new QHBoxLayout(central);

    auto* leftCol = new QVBoxLayout;
    leftCol->addWidget(grpFonte, 1);

    auto* midCol = new QVBoxLayout;
    midCol->addLayout(rightTop, 1);

    auto* bottomSplit = new QHBoxLayout;
    bottomSplit->addWidget(grpObj, 1);
    bottomSplit->addWidget(grpLog, 1);

    auto* centerAll = new QVBoxLayout;
    centerAll->addLayout(midCol, 1);
    centerAll->addLayout(bottomSplit);

    mainLayout->addLayout(leftCol, 2);
    mainLayout->addLayout(centerAll, 3);
    mainLayout->addWidget(grpAcoes);

    // ── Conexões ──────────────────────────────────────────
    connect(btnAbrir,     &QPushButton::clicked, this, &MainWindow::onAbrirFonte);
    connect(btnAddModulo, &QPushButton::clicked, this, &MainWindow::onAdicionarModulo);
    connect(btnRemModulo, &QPushButton::clicked, this, &MainWindow::onRemoverModulo);
    connect(btnMontar,    &QPushButton::clicked, this, &MainWindow::onMontar);
    connect(btnLinkar,    &QPushButton::clicked, this, &MainWindow::onLinkar);
    connect(btnExecutar,  &QPushButton::clicked, this, &MainWindow::onExecutarTudo);
    connect(btnPasso,     &QPushButton::clicked, this, &MainWindow::onPasso);
    connect(btnReset,     &QPushButton::clicked, this, &MainWindow::onReset);
    connect(btnIrMem,     &QPushButton::clicked, this, &MainWindow::atualizarMemoria);
    connect(editMemAddr,  &QLineEdit::returnPressed, this, &MainWindow::atualizarMemoria);
}

// ════════════════════════════════════════════════════════════
//  Slots
// ════════════════════════════════════════════════════════════

void MainWindow::onAbrirFonte() {
    QString path = QFileDialog::getOpenFileName(this, "Abrir fonte SIC/XE",
                                                 QString(), "Fontes (*.txt *.asm);;Todos (*)");
    if (path.isEmpty()) return;

    std::ifstream f(path.toStdString());
    if (!f.is_open()) return;

    std::string contents((std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>());
    editorFonte->setPlainText(QString::fromStdString(contents));
    lblArquivoAtual->setText(path);

    // Adicionar automaticamente à lista de módulos se ainda não estiver
    std::string stdPath = path.toStdString();
    bool ja_tem = false;
    for (const auto& s : fontes) if (s == stdPath) { ja_tem = true; break; }
    if (!ja_tem) {
        fontes.push_back(stdPath);
        listaModulos->addItem(path);
    }
}

void MainWindow::onAdicionarModulo() {
    QStringList paths = QFileDialog::getOpenFileNames(this, "Adicionar módulos",
                                                       QString(), "Fontes (*.txt *.asm);;Todos (*)");
    for (const auto& p : paths) {
        std::string sp = p.toStdString();
        bool ja_tem = false;
        for (const auto& s : fontes) if (s == sp) { ja_tem = true; break; }
        if (!ja_tem) {
            fontes.push_back(sp);
            listaModulos->addItem(p);
        }
    }
}

void MainWindow::onRemoverModulo() {
    auto* item = listaModulos->currentItem();
    if (!item) return;
    std::string sp = item->text().toStdString();
    fontes.erase(std::remove(fontes.begin(), fontes.end(), sp), fontes.end());
    delete listaModulos->takeItem(listaModulos->currentRow());
}

void MainWindow::onMontar() {
    if (fontes.empty()) {
        QMessageBox::warning(this, "Aviso", "Nenhum arquivo fonte adicionado.");
        return;
    }

    logExecucao->clear();
    viewObjeto->clear();
    objetos.clear();

    for (const auto& fonte : fontes) {
        // Gerar caminho do .obj ao lado do fonte
        std::filesystem::path p(fonte);
        std::string objPath = p.stem().string() + "_obj.txt";

        appendLog(QString("Montando: %1 → %2\n")
                  .arg(QString::fromStdString(fonte),
                       QString::fromStdString(objPath)));

        bool ok = montador.montar(fonte, objPath);
        if (!ok) {
            appendLog("  ERRO na montagem!\n");
            QMessageBox::critical(this, "Erro", "Falha ao montar " +
                                  QString::fromStdString(fonte));
            return;
        }
        appendLog("  ✔ Montagem OK\n");
        objetos.push_back(objPath);

        // Mostrar conteúdo do .obj
        std::ifstream f(objPath);
        std::string line;
        viewObjeto->appendPlainText("── " + QString::fromStdString(p.filename().string()) + " ──");
        while (std::getline(f, line))
            viewObjeto->appendPlainText(QString::fromStdString(line));
        viewObjeto->appendPlainText("");
    }

    btnLinkar->setEnabled(true);
    appendLog("Montagem concluída. Pronto para linkar.\n");
}

void MainWindow::onLinkar() {
    if (objetos.empty()) {
        QMessageBox::warning(this, "Aviso", "Nenhum arquivo objeto disponível. Monte primeiro.");
        return;
    }

    bool ok;
    uint32_t progaddr = editProgAddr->text().toUInt(&ok, 16);
    if (!ok) {
        QMessageBox::warning(this, "Aviso", "PROGADDR inválido.");
        return;
    }

    appendLog(QString("Linkando %1 módulo(s) em PROGADDR=0x%2...\n")
              .arg(objetos.size())
              .arg(progaddr, 4, 16, QChar('0')));

    ligador = Ligador(); // reset
    bool linkOk = ligador.LigadorRelocador(objetos, progaddr);
    if (!linkOk) {
        appendLog("Ligação falhou!\n");
        QMessageBox::critical(this, "Erro", "Falha na ligação.");
        return;
    }

    appendLog("✔ Ligação OK\n");

    // Carregar na máquina
    maquina = std::make_unique<Maquina>(16384);
    maquina->logCallback = [this](const std::string& msg) {
        appendLog(QString::fromStdString(msg));
    };

    ligador.CarregadorAbsoluto(*maquina);
    programaCarregado = true;
    programaTerminou  = false;

    appendLog("✔ Programa carregado na máquina.\n");

    atualizarRegistradores();
    atualizarMemoria();

    btnExecutar->setEnabled(true);
    btnPasso->setEnabled(true);
}

void MainWindow::onExecutarTudo() {
    if (!programaCarregado || programaTerminou) return;

    appendLog("── Executando ──\n");
    maquina->executar();
    programaTerminou = true;

    atualizarRegistradores();
    atualizarMemoria();
    appendLog("── Execução finalizada ──\n");

    btnPasso->setEnabled(false);
    btnExecutar->setEnabled(false);
}

void MainWindow::onPasso() {
    if (!programaCarregado || programaTerminou) return;

    StepResult r = maquina->passo();
    atualizarRegistradores();
    atualizarMemoria();

    if (r != StepResult::OK) {
        programaTerminou = true;
        btnPasso->setEnabled(false);
        btnExecutar->setEnabled(false);
        appendLog("── Programa parou ──\n");
    }
}

void MainWindow::onReset() {
    maquina = std::make_unique<Maquina>(16384);
    maquina->logCallback = [this](const std::string& msg) {
        appendLog(QString::fromStdString(msg));
    };

    programaCarregado = false;
    programaTerminou  = false;
    logExecucao->clear();
    viewObjeto->clear();
    objetos.clear();

    btnLinkar->setEnabled(false);
    btnExecutar->setEnabled(false);
    btnPasso->setEnabled(false);

    atualizarRegistradores();
    atualizarMemoria();
    appendLog("Reset completo.\n");
}

// ════════════════════════════════════════════════════════════
//  Helpers
// ════════════════════════════════════════════════════════════

void MainWindow::atualizarRegistradores() {
    struct RegInfo { const char* nome; int32_t val; };
    RegInfo regs[] = {
        {"A",  maquina->cpu.r.A},
        {"X",  maquina->cpu.r.X},
        {"L",  maquina->cpu.r.L},
        {"B",  maquina->cpu.r.B},
        {"S",  maquina->cpu.r.S},
        {"T",  maquina->cpu.r.T},
        {"PC", (int32_t)maquina->cpu.r.PC},
        {"SW", (int32_t)maquina->cpu.r.SW},
    };

    tabelaRegistradores->setRowCount(8);
    for (int i = 0; i < 8; ++i) {
        tabelaRegistradores->setItem(i, 0, new QTableWidgetItem(regs[i].nome));
        tabelaRegistradores->setItem(i, 1, new QTableWidgetItem(
            formatHex(static_cast<uint32_t>(regs[i].val), 6)));
    }
}

void MainWindow::atualizarMemoria() {
    bool ok;
    uint32_t base = editMemAddr->text().toUInt(&ok, 16);
    if (!ok) base = 0;
    base &= ~0xF; // alinhar para 16

    int rows = 16; // mostrar 16 linhas = 256 bytes
    tabelaMemoria->setRowCount(rows);

    for (int r = 0; r < rows; ++r) {
        uint32_t addr = base + r * 16;
        tabelaMemoria->setItem(r, 0, new QTableWidgetItem(formatHex(addr, 4)));

        for (int c = 0; c < 16; ++c) {
            uint32_t a = addr + c;
            uint8_t val = 0;
            if (a < maquina->memoria.getTamanhoBytes())
                val = static_cast<uint8_t>(maquina->memoria.read(a));
            auto* item = new QTableWidgetItem(formatHex(val, 2));

            // Destacar endereço do PC
            if (a == maquina->cpu.r.PC) {
                item->setBackground(QColor(255, 255, 120)); // amarelo
            }

            tabelaMemoria->setItem(r, c + 1, item);
        }
    }

    tabelaMemoria->resizeColumnsToContents();
}

void MainWindow::appendLog(const QString& msg) {
    logExecucao->moveCursor(QTextCursor::End);
    logExecucao->insertPlainText(msg);
    logExecucao->moveCursor(QTextCursor::End);
}

QString MainWindow::formatHex(uint32_t val, int w) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex
        << std::setw(w) << std::setfill('0') << val;
    return QString::fromStdString(oss.str());
}
