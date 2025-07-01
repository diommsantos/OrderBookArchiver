#pragma once

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include "OrderBookManager.h"

class OrderBookWriter : public QObject
{
    Q_OBJECT
protected:
    int depth; 
    QFile file;
    QTextStream out;
    OrderBookManager *manager;
protected slots:
    void write(const OrderBook &orderBook);
public:
    OrderBookWriter(OrderBookManager *manager, QString file, int depth);
    ~OrderBookWriter() = default;
    
    int getDepth();
    void setDepth(int depth);
    OrderBookManager * const getManager();
    void setManager(OrderBookManager *manager);
    const QFile &getFile();
    void setFile(QString file);

};