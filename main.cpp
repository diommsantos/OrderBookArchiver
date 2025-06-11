#include <QCoreApplication>
#include "OrderBookManager.h"
#include <QTimer>
#include <fstream>
#include <QFile>
#include <QTextStream>

QFile orderbookFile("orderbook.csv");

void writeOrderBookUpdate(const OrderBook &orderBook){
    QTextStream out{&orderbookFile};
    out << orderBook.lastUpdateId << ", " << orderBook.E.count() << ", ";

    constexpr int MAX_LEVELS = 30;
    out << "\"[";
    auto it = orderBook.asks.begin();
    for (int i = 0; i < MAX_LEVELS && it != orderBook.asks.end(); ++i) {
        out << "[" << it->price << "," << it->quantity << "]";
        if (it != std::prev(orderBook.asks.end()) && i != MAX_LEVELS-1) {
            out << ", "; // Add a comma if it's not the last element
        }
        it++;
    }
    out << "]\"";

    out << ", "; // Separator between asks and bids

    // Write the bids to the CSV file
    out << "\"[";
    it = orderBook.bids.begin();
    for (int i = 0; i < MAX_LEVELS && it != orderBook.bids.end(); ++i) {
        out << "[" << it->price << "," << it->quantity << "]";
        if (it != std::prev(orderBook.bids.end()) && i != MAX_LEVELS-1) {
            out << ", "; // Add a comma if it's not the last element
        }
        it++;
    }
    out << "]\"";

    // Add a separator for clarity
    out << "\n";

    // Ensure the data is flushed to the file
    out.flush();
}
    

int main(int argc, char *argv[])
{   
    QCoreApplication a(argc, argv);
    orderbookFile.open(QIODeviceBase::WriteOnly);
    OrderBookManager manager("BTCUSDT", 100, 1000);
    
    QObject::connect(&manager, &OrderBookManager::orderBookUpdated, &writeOrderBookUpdate);
    manager.start();

    //QWebSocket socket;
    //socket.close();
    return a.exec();
    
}