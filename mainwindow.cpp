// ============================================================
// mainwindow.cpp
// ============================================================

#include "mainwindow.h"
#include "tiny_language_laxer.h"
#include "tiny_parser.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QStatusBar>
#include <QSplitter>
#include <QTabWidget>
#include <QMessageBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainterPath>
#include <cmath>


// ============================================================
// Constructor
// ============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Tiny Language Compiler");
    resize(1450, 900);

    setupUI();
}

// ============================================================
// Setup UI
// ============================================================

void MainWindow::setupUI()
{
    QWidget* central = new QWidget(this);

    setCentralWidget(central);

    qApp->setStyleSheet(R"(

    QMainWindow{
        background:#0d1117;
    }

    QWidget{
        background:#0d1117;
        color:white;
        font-family:Consolas;
    }

    QLabel{
        color:white;
    }

    QGroupBox{
        border:1px solid #30363d;
        border-radius:10px;
        margin-top:10px;
        font-weight:bold;
        padding-top:15px;
        color:#58a6ff;
        font-size:12pt;
    }

    QTextEdit{
        background:#161b22;
        border:1px solid #30363d;
        border-radius:8px;
        color:white;
        padding:10px;
        font-size:11pt;
    }

    QPushButton{
        background:#238636;
        color:white;
        border:none;
        border-radius:8px;
        padding:10px 20px;
        font-weight:bold;
        font-size:11pt;
    }

    QPushButton:hover{
        background:#2ea043;
    }

    QTableWidget{
        background:#161b22;
        color:white;
        border:1px solid #30363d;
        border-radius:10px;
        gridline-color:#30363d;
    }

    QHeaderView::section{
        background:#21262d;
        color:#58a6ff;
        padding:8px;
        border:none;
        font-weight:bold;
    }

    QTreeWidget{
        background:#161b22;
        color:white;
        border:1px solid #30363d;
        border-radius:10px;
    }

    QTreeWidget::item:selected{
        background:#1f6feb;
    }

    QTabWidget::pane{
        border:none;
    }

    QTabBar::tab{
        background:#161b22;
        color:#8b949e;
        padding:10px 22px;
        border-top-left-radius:8px;
        border-top-right-radius:8px;
    }

    QTabBar::tab:selected{
        background:#1f6feb;
        color:white;
    }

    QListWidget{
        background:#161b22;
        color:white;
        border:none;
    }

    QListWidget::item{
        padding:10px;
    }

    QListWidget::item:selected{
        background:#1f6feb;
        border-radius:5px;
    }

    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    QLabel* title =
        new QLabel(" Tiny Language Compiler Front-End");

    title->setFixedHeight(45);

    title->setStyleSheet(R"(
        background:#161b22;
        color:#58a6ff;
        font-size:16pt;
        font-weight:bold;
        padding-left:15px;
        border-bottom:1px solid #30363d;
    )");

    mainLayout->addWidget(title);

    QTabWidget* tabs = new QTabWidget(this);

    QWidget* reportPage = new QWidget();
    QWidget* lexerPage  = new QWidget();
    QWidget* parserPage = new QWidget();

    tabs->addTab(reportPage, "RE / NFA / DFA");
    tabs->addTab(lexerPage,  "Lexer");
    tabs->addTab(parserPage, "Parser");

    mainLayout->addWidget(tabs);

    setupReportTab(reportPage);
    setupLexerTab(lexerPage);
    setupParserTab(parserPage);

    tokenNames = {
        "Identifier",
        "Number",
        "Operators",
        "String",
        "Comment",
        "Whitespace"
    };

    tokenREs = {
        "letter(letter|digit)*",
        "digit+",
        "+|-|*|/|<|=|:=",
        "\" any char \"",
        "{ any char }",
        "space|tab|newline"
    };

    for(const QString& t : tokenNames)
        tokenList->addItem(t);

    tokenList->setCurrentRow(0);
}

// ============================================================
// Report Tab
// ============================================================

void MainWindow::setupReportTab(QWidget* parent)
{
    QHBoxLayout* layout = new QHBoxLayout(parent);

    tokenList = new QListWidget();

    tokenList->setFixedWidth(200);

    layout->addWidget(tokenList);

    QWidget* rightWidget = new QWidget();

    QVBoxLayout* rightLayout =
        new QVBoxLayout(rightWidget);

    reLabel = new QLabel();

    reLabel->setStyleSheet(R"(
        background:#161b22;
        border:1px solid #30363d;
        border-radius:8px;
        padding:15px;
        font-size:13pt;
        color:#58a6ff;
    )");

    rightLayout->addWidget(reLabel);

    nfaScene = new QGraphicsScene(this);

    nfaView = new QGraphicsView(nfaScene);

    nfaView->setMinimumHeight(250);

    nfaView->setStyleSheet(R"(
        background:#161b22;
        border:1px solid #30363d;
        border-radius:10px;
    )");

    rightLayout->addWidget(nfaView);

    dfaScene = new QGraphicsScene(this);

    dfaView = new QGraphicsView(dfaScene);

    dfaView->setMinimumHeight(250);

    dfaView->setStyleSheet(R"(
        background:#161b22;
        border:1px solid #30363d;
        border-radius:10px;
    )");

    rightLayout->addWidget(dfaView);

    layout->addWidget(rightWidget);

    connect(
        tokenList,
        &QListWidget::currentRowChanged,
        this,
        &MainWindow::onTokenSelected
        );
}

// ============================================================
// Lexer Tab
// ============================================================

void MainWindow::setupLexerTab(QWidget* parent)
{
    QVBoxLayout* layout = new QVBoxLayout(parent);

    QGroupBox* inputGroup =
        new QGroupBox("Source Code");

    QVBoxLayout* inputLayout =
        new QVBoxLayout(inputGroup);

    inputEditor = new QTextEdit();

    inputEditor->setPlaceholderText(
        "Enter Tiny source code here..."
        );

    inputLayout->addWidget(inputEditor);

    QHBoxLayout* btnLayout = new QHBoxLayout();

    analyzeBtn = new QPushButton("Analyze");

    clearBtn = new QPushButton("Clear");

    btnLayout->addWidget(analyzeBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addStretch();

    inputLayout->addLayout(btnLayout);

    layout->addWidget(inputGroup);

    ////////////////////////////////////////////////////////////

    QGroupBox* tokenGroup =
        new QGroupBox("Token Stream");

    QVBoxLayout* tokenLayout =
        new QVBoxLayout(tokenGroup);

    tokenTable = new QTableWidget(0,4);

    tokenTable->setHorizontalHeaderLabels({
        "#",
        "Lexeme",
        "Token Type",
        "Line"
    });

    tokenTable->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);

    tokenTable->verticalHeader()->setVisible(false);

    tokenLayout->addWidget(tokenTable);

    layout->addWidget(tokenGroup);

    connect(
        analyzeBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::onAnalyze
        );

    connect(
        clearBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::onClear
        );
}

// ============================================================
// Parser Tab
// ============================================================

void MainWindow::setupParserTab(QWidget* parent)
{
    QVBoxLayout* layout = new QVBoxLayout(parent);

    QGroupBox* parserGroup =
        new QGroupBox("Parse Tree");

    QVBoxLayout* parserLayout =
        new QVBoxLayout(parserGroup);
    parseScene = new QGraphicsScene(this);
    parseView = new QGraphicsView(parseScene);

    parseView->setStyleSheet(R"(
    background:#161b22;
    border:1px solid #30363d;
    border-radius:10px;
)");



parserLayout->addWidget(parseView);
    layout->addWidget(parserGroup);
}

// ============================================================
// Token Selected
// ============================================================

void MainWindow::onTokenSelected(int index)
{
    if(index < 0 || index >= tokenNames.size())
        return;

    QString token = tokenNames[index];

    reLabel->setText(tokenREs[index]);

    drawNFA(nfaScene, token);
    drawDFA(dfaScene, token);
}

// ============================================================
// Draw State
// ============================================================

void MainWindow::drawState(
    QGraphicsScene* s,
    qreal x,
    qreal y,
    const QString& name,
    bool accept,
    bool start,
    QColor color)
{
    qreal r = 28;

    if(accept)
    {
        s->addEllipse(
            x-r-5,
            y-r-5,
            (r+5)*2,
            (r+5)*2,
            QPen(color,2)
            );
    }

    s->addEllipse(
        x-r,
        y-r,
        r*2,
        r*2,
        QPen(color,2),
        QBrush(QColor("#21262d"))
        );

    QGraphicsTextItem* txt =
        s->addText(name);

    txt->setDefaultTextColor(Qt::white);

    txt->setPos(x-10,y-10);

    if(start)
    {
        s->addLine(
            x-60,
            y,
            x-r,
            y,
            QPen(color,2)
            );
    }
}

// ============================================================
// Draw Arrow
// ============================================================

void MainWindow::drawArrow(
    QGraphicsScene* s,
    qreal x1,
    qreal y1,
    qreal x2,
    qreal y2,
    const QString& label,
    QColor color)
{
    s->addLine(
        x1,
        y1,
        x2,
        y2,
        QPen(color,2)
        );

    QGraphicsTextItem* txt =
        s->addText(label);

    txt->setDefaultTextColor(color);

    txt->setPos((x1+x2)/2,(y1+y2)/2);
}

// ============================================================
// Draw Self Loop
// ============================================================

void MainWindow::drawSelfLoop(
    QGraphicsScene* s,
    qreal x,
    qreal y,
    const QString& label,
    QColor color)
{
    QPainterPath path;

    path.moveTo(x-20,y-25);

    path.cubicTo(
        x-60,
        y-80,
        x+60,
        y-80,
        x+20,
        y-25
        );

    s->addPath(path,QPen(color,2));

    QGraphicsTextItem* txt =
        s->addText(label);

    txt->setDefaultTextColor(color);

    txt->setPos(x-20,y-90);
}

// ============================================================
// Draw NFA
// ============================================================

void MainWindow::drawNFA(
    QGraphicsScene* scene,
    const QString& token)
{
    scene->clear();

    QColor c("#58a6ff");

    drawState(scene,100,120,"q0",false,true,c);

    drawState(scene,350,120,"q1",true,false,c);

    drawArrow(scene,130,120,320,120,token,c);

    drawSelfLoop(scene,350,120,"loop",c);
}

// ============================================================
// Draw DFA
// ============================================================

void MainWindow::drawDFA(
    QGraphicsScene* scene,
    const QString& token)
{
    scene->clear();

    QColor c("#f78166");

    drawState(scene,100,120,"S",false,true,c);

    drawState(scene,350,120,"A",true,false,c);

    drawArrow(scene,130,120,320,120,token,c);
}

void MainWindow::drawTree(NodePtr node, int depth, int &x, int parentX, int parentY)
{
    if (!node) return;

    // حساب المكان الحالي
    int xPos = x * 150;
    int yPos = depth * 100;

    // رسم الخط للأب
    if (depth > 0) {
        parseScene->addLine(parentX + 40, parentY + 40, xPos + 40, yPos, QPen(QColor("#8b949e"), 2));
    }

    // رسم العقدة
    parseScene->addRect(xPos, yPos, 80, 40, QPen(Qt::white), QBrush(QColor("#21262d")));

    QString label = node->value.empty() ? QString::fromStdString(nodeTypeName(node->type)) : QString::fromStdString(node->value);
    QGraphicsTextItem* txt = parseScene->addText(label);
    txt->setDefaultTextColor(Qt::white);
    txt->setPos(xPos + 5, yPos + 8);

    // تحديث الإحداثي الأفقي قبل رسم الأبناء
    int currentX = xPos;
    int currentY = yPos;

    if (node->children.empty()) {
        x++; // لو مفيش أبناء، نحجز مكان للي جنبه
    } else {
        for (auto child : node->children) {
            drawTree(child, depth + 1, x, currentX, currentY);
        }
    }
}// ============================================================
// Analyze
// ============================================================
void MainWindow::drawParseTree(NodePtr root)
{
    parseScene->clear();

    int x = 0;
    drawTree(root, 0, x, 0, 0);
}
void MainWindow::onAnalyze()
{
    std::string src = inputEditor->toPlainText().toStdString();
    Lexer lexer(src);
    std::vector<Token> tokens = lexer.tokenize();

    tokenTable->setRowCount(0);
    for(const auto& tok : tokens) {
        if(tok.type == END_OF_FILE) break;
        int row = tokenTable->rowCount();
        tokenTable->insertRow(row);
        tokenTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(tok.lexeme)));
        tokenTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(Lexer::typeName(tok.type))));
        tokenTable->setItem(row, 3, new QTableWidgetItem(QString::number(tok.line)));
    }

    Parser parser(tokens);
    NodePtr tree = parser.parse();

    parseScene->clear();

    if (tree) {
        int startX = 0; // عرفي المتغير هنا
        drawTree(tree, 0, startX, 0, 0);
    }

    // 4. عرض الأخطاء لو موجودة (مهم جداً للمناقشة)
    if (parser.hasErrors()) {
        QString errorLog;
        for (const auto& err : parser.errors()) {
            errorLog += QString("Line %1: %2\n").arg(err.line).arg(QString::fromStdString(err.message));
        }
        QMessageBox::warning(this, "Syntax Errors", errorLog);
    }
}
// ============================================================
// Clear
// ============================================================

void MainWindow::onClear()
{
    inputEditor->clear();

    tokenTable->setRowCount(0);

    parseScene->clear();
    statusBar()->showMessage("Ready");
}