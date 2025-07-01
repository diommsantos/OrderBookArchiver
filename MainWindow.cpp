#include "MainWindow.h"

Archiver::Archiver(QString symbol, QString file, int depth, int updateSpeed) :
symbol{symbol},
file{file},
depth{depth},
updateSpeed{updateSpeed},
manager{new OrderBookManager(symbol, 5000, updateSpeed)},
writer{new OrderBookWriter(manager, file, depth)},
isRunning{false}
{
    writer->setManager(manager);
}

Archiver::~Archiver()
{
    delete manager;
    delete writer;
}

void Archiver::setFile(QString file){
    writer->setFile(file);
}

void Archiver::setSymbol(QString symbol){
    manager->setSymbol(symbol);
}

void Archiver::setDepth(int depth){
    writer->setDepth(depth);
}

void Archiver::setUpdateSpeed(int updateSpeed){
    manager->setUpdateSpeed(updateSpeed);
}

bool Archiver::start(){
    return isRunning = manager->start();
}

bool Archiver::stop(){
    isRunning = !(manager->stop());
    return !isRunning;
}

bool Archiver::running(){
    return isRunning;
}

DefaultArchiverWidget::DefaultArchiverWidget(QWidget *parent) : 
QWidget(parent)
{
    archiver = new Archiver(defaultSymbol, defaultFile, defaultDepth, defaultUpdateSpeed);
    horizontalLayout = new QHBoxLayout(this);
    symbolLabel = new QLabel("Symbol: ", this);
    horizontalLayout->addWidget(symbolLabel);
    symbolLineEdit = new QLineEdit(defaultSymbol ,this);
    horizontalLayout->addWidget(symbolLineEdit);
    fileButton = new QPushButton(this);
    fileButton->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    horizontalLayout->addWidget(fileButton);
    fileLineEdit = new QLineEdit(defaultFile, this);
    fileLineEdit->setReadOnly(true);
    horizontalLayout ->addWidget(fileLineEdit);
    depthLabel = new QLabel("Depth: ", this);
    horizontalLayout->addWidget(depthLabel);
    depthComboBox = new QComboBox(this);
    depthComboBox->addItems({"25", "50", "100", "500", "5000"});
    horizontalLayout->addWidget(depthComboBox);
    updateSpeedLabel = new QLabel("Update Speed: ", this);
    horizontalLayout->addWidget(updateSpeedLabel);
    updateSpeedComboBox = new QComboBox(this);
    updateSpeedComboBox->addItems({"100", "1000"});
    horizontalLayout->addWidget(updateSpeedComboBox);
    msLabel = new QLabel("ms", this);
    horizontalLayout->addWidget(msLabel, 0, Qt::AlignLeft);
    sButton = new QPushButton(this);
    sButton->setText("Start");
    sButton->setFixedWidth(40);
    horizontalLayout->addWidget(sButton);
    horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);
    connect(symbolLineEdit, &QLineEdit::editingFinished, [&](){archiver->setSymbol(symbolLineEdit->text());});
    connect(fileButton, &QPushButton::clicked, this, [&](){
        QString fileName = QFileDialog::getSaveFileName(this, "Save as...", 
            symbolLineEdit->text()+"-orderBook-"+QDateTime::currentDateTimeUtc().toString("dd-MM-yyyy-HH'h'mm'm'")+".csv", 
            "Comma-separated values (*.csv)");
        fileLineEdit->setText(fileName);
        archiver->setFile(fileName);
    });
    connect(depthComboBox, &QComboBox::currentTextChanged, this, [&](const QString &depth){archiver->setDepth(depth.toInt());});
    connect(updateSpeedComboBox, &QComboBox::currentTextChanged, this, [&](const QString &updateSpeed){archiver->setUpdateSpeed(updateSpeed.toInt());});
    connect(sButton, &QPushButton::clicked, this, [&](){
        if(!(archiver->running())){
            if(archiver->start())
                sButton->setText("Stop");
        }else{
            if(archiver->stop())
                sButton->setText("Start");
        }
    });
};

DefaultArchiverWidget::~DefaultArchiverWidget(){
    delete archiver;
};

ArchiverWidget::ArchiverWidget(QWidget *parent) :
QWidget(parent)
{
    horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setContentsMargins(0,0,0,0);
    defaultArchiverWidget =  new DefaultArchiverWidget(this);
    horizontalLayout->addWidget(defaultArchiverWidget);
    //horizontalLayout->addSpacerItem(new QSpacerItem(20, 0));
    closeButton = new QPushButton(this);
    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    horizontalLayout->addWidget(closeButton);
    horizontalLayout->setSizeConstraint(QLayout::SetFixedSize);
    connect(closeButton, &QPushButton::clicked, this, &ArchiverWidget::closeButtonClicked);
};

AddButton::AddButton(QWidget *parent) : 
QPushButton(parent)
{
};

void AddButton::paintEvent(QPaintEvent *event){
    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect(offset + lineWidth / 2.0, offset + lineWidth / 2.0, radius*2 - lineWidth, radius*2 - lineWidth);
    painter.setPen(QPen(palette().color(QPalette::ButtonText), lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect);

    painter.drawLine(QPoint(width() / 2, offset + lineWidth + height() / 10), QPoint(width() / 2, height() - (offset + lineWidth + height() / 10)));
    painter.drawLine(QPoint(offset + lineWidth + width()/10 , height() / 2), QPoint(width() - (offset + lineWidth + width()/10), height() / 2));
};

void AddButton::resizeEvent(QResizeEvent *event){
    setMask(generateMask());
    lineWidth = width() / 8;
    QPushButton::resizeEvent(event);
};

QBitmap AddButton::generateMask(){
    int w = width();
    int h = height();
    radius = std::min(w, h) / 2 - offset;
    QBitmap bmp(size());
    bmp.clear();
    QPainter p(&bmp);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::color1));
    p.setBrush(QBrush(Qt::color1));
    int diameter = radius*2;
    p.drawEllipse(offset, offset, diameter, diameter);
    p.end();
    return bmp;
};    


MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent)
{
    resize(800, 600);
    scrollArea = new QScrollArea(this);
    setCentralWidget(scrollArea);
    scrollWidget = new QWidget(scrollArea);
    scrollWidget->setFixedWidth(780);
    verticalLayout = new QVBoxLayout(scrollWidget);
    QMargins margins = verticalLayout->contentsMargins();
    margins.setLeft(0);
    margins.setRight(0);
    verticalLayout->setContentsMargins(margins);
    
    defaultArchiverWidgetWrapper = new QWidget(scrollWidget);
    defaultArchiverWidgetWrapper->setFixedSize(scrollWidget->width(), 42);
    QHBoxLayout *wrapperLayout = new QHBoxLayout(defaultArchiverWidgetWrapper);
    defaultArchiverWidget = new DefaultArchiverWidget(defaultArchiverWidgetWrapper);
    wrapperLayout->addWidget(defaultArchiverWidget, 0, Qt::AlignCenter);
    wrapperLayout->activate();
    verticalLayout->addWidget(defaultArchiverWidgetWrapper, 0, Qt::AlignLeft);
    verticalLayout->addItem(new QSpacerItem(0, 30));
    addButton = new AddButton(scrollWidget);
    addButton->setFixedSize(40, 40);
    verticalLayout->addWidget(addButton, 0, Qt::AlignHCenter | Qt::AlignTop);
    defaultArchiverWidgetWrapper->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    connect(addButton, &AddButton::clicked, [this](bool checked){addArchiverWidget();});
};

void MainWindow::addArchiverWidget(){
    QWidget *archiverWidgetWrapper = new QWidget(scrollWidget);
    archiverWidgetWrapper->setFixedSize(defaultArchiverWidgetWrapper->width(), defaultArchiverWidgetWrapper->height());
    ArchiverWidget *archiverWidget = new ArchiverWidget(archiverWidgetWrapper);
    archiverWidget->move(defaultArchiverWidget->x(), defaultArchiverWidget->y());
    verticalLayout->insertWidget(verticalLayout->count()-2, archiverWidgetWrapper, 0, Qt::AlignLeft);
    connect(archiverWidget, &ArchiverWidget::closeButtonClicked, this, [this](){
        QObject *archiverWidgetWrapper = sender()->parent();
        int index = verticalLayout->indexOf((const QWidget *)archiverWidgetWrapper);
        QWidget *wrapperAtIndex = verticalLayout->itemAt(index)->widget();
        verticalLayout->removeWidget(wrapperAtIndex);
        wrapperAtIndex->deleteLater();
    });
};