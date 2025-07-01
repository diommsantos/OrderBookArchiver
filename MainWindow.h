#pragma once

#include <QMainWindow>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QBitmap>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QComboBox>
#include "OrderBookManager.h"
#include "OrderBookWriter.h"

struct Archiver{
protected:
    bool isRunning;
public:
    QString symbol;
    QString file; 
    int depth;
    int updateSpeed;

    OrderBookManager *manager;
    OrderBookWriter *writer;


    Archiver(QString symbol, QString file, int depth, int updateSpeed);
    ~Archiver();
    void setSymbol(QString symbol);
    void setFile(QString file);
    void setDepth(int depth);
    void setUpdateSpeed(int updateSpeed);
    bool start();
    bool stop();
    bool running();
};

class DefaultArchiverWidget : public QWidget {
    Q_OBJECT
protected:
    const QString defaultSymbol{"BTCUSDT"};
    const QString defaultFile{defaultSymbol + "-orderBook-"+QDateTime::currentDateTimeUtc().toString("dd-MM-yyyy-HH'h'mm'm'")+".csv"}; 
    int defaultDepth{25};
    int defaultUpdateSpeed{100};
    Archiver *archiver;
    QHBoxLayout *horizontalLayout;
    QLabel *symbolLabel;
    QLineEdit *symbolLineEdit;
    QFileDialog *fileDialog;
    QPushButton *fileButton;
    QLineEdit *fileLineEdit;
    QLabel *depthLabel;
    QComboBox *depthComboBox;
    QLabel *updateSpeedLabel;
    QComboBox *updateSpeedComboBox;
    QLabel *msLabel;
    QPushButton *sButton;
public:
    DefaultArchiverWidget(QWidget *parent = nullptr);
    ~DefaultArchiverWidget();
};

class ArchiverWidget : public QWidget
{
    Q_OBJECT
protected:
    DefaultArchiverWidget *defaultArchiverWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *closeButton;

public:
    ArchiverWidget(QWidget *parent = nullptr);

signals:
    void closeButtonClicked(bool checked);
};

class AddButton : public QPushButton {
private:
    int lineWidth;
    int offset = 2;
    int radius;
    QBitmap generateMask();
protected:
    void resizeEvent(QResizeEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

public:
    AddButton(QWidget *parent = nullptr);
};

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    QScrollArea *scrollArea;
    QWidget *scrollWidget;
    QVBoxLayout *verticalLayout;
    DefaultArchiverWidget *defaultArchiverWidget;
    QWidget *defaultArchiverWidgetWrapper;
    AddButton *addButton;
    void addArchiverWidget();
    void removeArchiverWidget(int index);

public:
    MainWindow(QWidget *parent = nullptr);
};