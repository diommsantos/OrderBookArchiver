#include "OrderBookManager.h"
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <iostream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QEventLoop>

OrderBookThread::OrderBookThread(QObject *parent) : QThread(parent) {}

void OrderBookThread::run()
{   
    OrderBookManager *obm = static_cast<OrderBookManager *>(parent());
    obm->socket.open(QUrl("wss://stream.binance.com:9443/ws/" + obm->symbol.toLower() + "@depth" +
        (obm->updateSpeed == 100 ? "@100ms" : "")));
    exec();

}

OrderBookManager::OrderBookManager(QString symbol, int depth, int updateSpeed) :
thread(this),
networkManager(this), 
symbol{symbol},
updateSpeed{updateSpeed},
depth{depth}
{
    socket.moveToThread(&thread);
    QObject::connect(&socket, &QWebSocket::textMessageReceived, this, &OrderBookManager::messageHandler, Qt::DirectConnection);

}

void OrderBookManager::updateOrderBook(){
    while(hasMessages.available()){
        QJsonDocument json = QJsonDocument::fromJson(messageQueue.dequeue().toUtf8());
        qDebug() << "U is: " << json["U"].toInteger() << "u is: " << json["u"].toInteger() << "and lastUpdateId is: "<< orderBook.lastUpdateId;
        if(json["u"].toInteger() < orderBook.lastUpdateId)
            return;
        if(json["U"].toInteger() > orderBook.lastUpdateId + 1){
            qCritical() << "Something went wrong, U is greater than lastUpdateId";
            return;
        }

        std::list<Order> asks = fromJsonArray(json["a"].toArray());
        std::list<Order> bids = fromJsonArray(json["b"].toArray());

        //goes through each ask received on the update and finds where to insert/delete in the orderBook each updated ask
        for(auto ask : asks){
            auto it = std::lower_bound(orderBook.asks.begin(), orderBook.asks.end(), ask,
                [](Order a, Order ask){
                    return a.price.toDouble() < ask.price.toDouble();
                });
            if(it != orderBook.asks.end() && (*it).price == ask.price){
                if(ask.quantity.toDouble() == 0)
                    it = orderBook.asks.erase(it);
                else
                    *it = ask;
            }else if(ask.quantity.toDouble() != 0){
                it = orderBook.asks.insert(it, ask);
            }
        }

        for(auto bid : bids){
            auto it = std::lower_bound(orderBook.bids.begin(), orderBook.bids.end(), bid,
                [](Order a, Order bid){
                    return a.price.toDouble() > bid.price.toDouble();
                });
            if(it != orderBook.bids.end() && (*it).price == bid.price){
                if(bid.quantity.toDouble() == 0)
                    it = orderBook.bids.erase(it);
                else
                    *it = bid;
            }else if(bid.quantity.toDouble() != 0){
                it = orderBook.bids.insert(it, bid);
            }
        }

        orderBook.E = std::chrono::milliseconds(json["E"].toInteger());
        orderBook.lastUpdateId = json["u"].toInteger();
        hasMessages.acquire();
        emit orderBookUpdated(orderBook);
    }     
}

std::list<Order> OrderBookManager::fromJsonArray(QJsonArray arr)
{
    std::list<Order> v;
    for(auto value : arr){
        QJsonArray pair = value.toArray();
        Order order{pair[0].toString(), pair[1].toString()};
        v.push_back(order);
    }
    return v;
}

void OrderBookManager::start()
{   
    thread.start();

    hasMessages.acquire();
    QJsonDocument jsonOrderBook = QJsonDocument::fromJson(messageQueue.head().toUtf8()); hasMessages.release();
    orderBook.lastUpdateId = jsonOrderBook["U"].toInteger();

    QEventLoop loop;
    QNetworkRequest request(QUrl("https://api.binance.com/api/v3/depth?symbol=" + symbol + "&limit=" + QString::number(depth)));
    connect(&networkManager, &QNetworkAccessManager::finished, [&jsonOrderBook](QNetworkReply *reply){
        QByteArray response = reply->readAll();
        jsonOrderBook = QJsonDocument::fromJson(response);
        reply->deleteLater();
    });
    connect(&networkManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    do{
        QNetworkReply *reply = networkManager.get(request);
        loop.exec();
    }while(jsonOrderBook["lastUpdateId"].toInteger() < orderBook.lastUpdateId);

    //Remove messages with u <= lasUpdateId
    orderBook.lastUpdateId = jsonOrderBook["lastUpdateId"].toInteger();
    for(int i = 0; i < messageQueue.size(); i++){
        QJsonDocument json = QJsonDocument::fromJson(messageQueue[i].toUtf8());
        if(json["u"].toInteger() <= orderBook.lastUpdateId)
            messageQueue.removeAt(i); i--; hasMessages.acquire();
    }
    
    orderBook.asks = fromJsonArray(jsonOrderBook["asks"].toArray());
    orderBook.bids = fromJsonArray(jsonOrderBook["bids"].toArray());

    QObject::connect(this, &OrderBookManager::messageReceived, this, &OrderBookManager::updateOrderBook, Qt::QueuedConnection);
    
}

void OrderBookManager::stop()
{   
    thread.quit();
    thread.wait();
    hasMessages.acquire(hasMessages.available());
    QObject::disconnect(this, &OrderBookManager::messageReceived, this, &OrderBookManager::updateOrderBook);
}

void OrderBookManager::messageHandler(const QString &message)
{
    qDebug() << "hasMessages is:" << hasMessages.available();
    messageQueue.enqueue(message);
    hasMessages.release();
    emit messageReceived();
    qDebug() << "Message received!";
    
}