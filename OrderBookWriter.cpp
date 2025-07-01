#include "OrderBookWriter.h"
#include <QObject>

OrderBookWriter::OrderBookWriter(OrderBookManager *manager, QString file, int depth) : 
manager{manager},
depth{depth}
{
    connect(manager, &OrderBookManager::orderBookUpdated, this, &OrderBookWriter::write);
    setFile(file);
};

int OrderBookWriter::getDepth(){
    return depth;
}

void OrderBookWriter::setDepth(int depth){
    this->depth = depth;
}

OrderBookManager * const OrderBookWriter::getManager(){
    return manager;
};

void OrderBookWriter::setManager(OrderBookManager *manager){
    this->manager = manager;
    connect(manager, &OrderBookManager::orderBookUpdated, this, &OrderBookWriter::write);
};

const QFile &OrderBookWriter::getFile(){
    return file;
};

void OrderBookWriter::setFile(QString file){
    this->file.setFileName(file);
    this->file.open(QIODevice::WriteOnly);
    out.setDevice(&(this->file));
};

void OrderBookWriter::write(const OrderBook &orderBook){
    out << orderBook.lastUpdateId << ", " << orderBook.E.count() << ", ";

    out << "\"[";
    auto it = orderBook.asks.begin();
    for (int i = 0; i < depth && it != orderBook.asks.end(); ++i) {
        out << "[" << it->price << "," << it->quantity << "]";
        if (it != std::prev(orderBook.asks.end()) && i != depth-1) {
            out << ", "; // Add a comma if it's not the last element
        }
        it++;
    }
    out << "]\"";

    out << ", "; // Separator between asks and bids

    out << "\"[";
    it = orderBook.bids.begin();
    for (int i = 0; i < depth && it != orderBook.bids.end(); ++i) {
        out << "[" << it->price << "," << it->quantity << "]";
        if (it != std::prev(orderBook.bids.end()) && i != depth-1) {
            out << ", "; // Add a comma if it's not the last element
        }
        it++;
    }
    out << "]\"";

    out << "\n";
    out.flush();
};

