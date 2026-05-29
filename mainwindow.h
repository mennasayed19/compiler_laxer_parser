#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QTabWidget>
#include <QListWidget>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "parse_node.h"
#include <QGraphicsScene>
#include <QGraphicsView>



class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAnalyze();
    void onClear();
    void onTokenSelected(int index);

private:
    void drawParseTree(NodePtr root);

    void setupUI();
    void setupLexerTab(QWidget* parent);
    void setupParserTab(QWidget* parent);
    void setupReportTab(QWidget* parent);
    void setupAnalyzerTab(QWidget* parent);
    QString tokenColor(const QString& type);
void drawTree(NodePtr node, int depth, int &x, int parentX = 0, int parentY = 0);
    void drawNFA(QGraphicsScene* scene, const QString& token);
    void drawDFA(QGraphicsScene* scene, const QString& token);
    void drawArrow(QGraphicsScene* s, qreal x1, qreal y1, qreal x2, qreal y2,
                   const QString& label, QColor color);
    void drawState(QGraphicsScene* s, qreal x, qreal y, const QString& name,
                   bool accept, bool start, QColor color);
    void drawSelfLoop(QGraphicsScene* s, qreal x, qreal y,
                      const QString& label, QColor color);

    // Analyzer
    QTextEdit*    inputEditor;
    QTableWidget* tokenTable;
    QLabel*       statusLabel;
    QPushButton*  analyzeBtn;
    QPushButton*  clearBtn;
    QTreeWidget*  parseTreeWidget;
    QGraphicsScene* parseScene;
    QGraphicsView* parseView;
    // Report
    QListWidget*    tokenList;
    QLabel*         reLabel;
    QGraphicsView*  nfaView;
    QGraphicsView*  dfaView;
    QGraphicsScene* nfaScene;
    QGraphicsScene* dfaScene;

    QStringList tokenNames;
    QStringList tokenREs;
};

#endif