#pragma once

#include <QtWebSockets/QWebSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <list>
#include <string>
#include <chrono>
#include <QQueue>
#include <QSemaphore>
#include <QThread>
#include <QNetworkAccessManager>

class OrderBookThread : public QThread
{
        Q_OBJECT

    public:
        QWebSocket socket;
        OrderBookThread(QObject *parent = nullptr);
        void run() override;
};

struct Order{
    QString price;
    QString quantity;
};

struct OrderBook
{
    long long lastUpdateId;
    std::chrono::milliseconds E; // Event time
    std::list<Order> bids;
    std::list<Order> asks;
};

class OrderBookManager : public QObject
{
    Q_OBJECT

private:
    QNetworkAccessManager networkManager;
    QString symbol;
    int depth;
    int updateSpeed;
    OrderBook orderBook;
    QQueue<QString> messageQueue;
    QSemaphore hasMessages;

    OrderBookThread *thread;
    friend void OrderBookThread::run();

    void messageHandler(const QString &message);
    void updateOrderBook();

    std::list<Order> fromJsonArray(QJsonArray arr);

public:
    OrderBookManager(QString symbol, int depth = 5000, int updateSpeed = 100);
    ~OrderBookManager() = default;
    bool start();
    bool stop();
    void setSymbol(QString symbol);
    void setDepth(int depth);
    void setUpdateSpeed(int updateSpeed);

signals:
    void messageReceived();
    void orderBookUpdated(const OrderBook &orderBook);
};
