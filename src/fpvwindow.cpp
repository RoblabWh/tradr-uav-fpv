#include "fpvwindow.h"
#include "ui_fpvwindow.h"

FPVWindow::FPVWindow(QWidget* parent) : QMainWindow(parent)
{
    this->ui = new Ui::FPVWindow;
    this->ui->setupUi(this);

    this->uavManager = new UavManager();

    connect(this->uavManager, SIGNAL(uavRegistered(uint, Uav*)), this, SLOT(on_uavManager_uavRegistered(uint, Uav*)), Qt::UniqueConnection);
    connect(this->uavManager, SIGNAL(uavUnregistered(uint, Uav*)), this, SLOT(on_uavManager_uavUnregistered(uint, Uav*)), Qt::UniqueConnection);
    connect(this->uavManager, SIGNAL(uavReseted()), this, SLOT(on_uavManager_uavReseted()), Qt::UniqueConnection);

    std::cout << "reset activeUAV" << endl;
    this->activeUav = nullptr;
    this->activeUavItemWidget = nullptr;

    this->uavManager->update();
}

FPVWindow::~FPVWindow()
{
    delete ui;
}

void FPVWindow::on_uavManager_uavRegistered(unsigned int index, Uav* uav)
{
    std::cout << "register new uav" << std::endl;

    QListWidgetItem* item = new QListWidgetItem();
    UavItemWidget* widget = new UavItemWidget(uav);
    connect(widget, SIGNAL(activeUavChanged(Uav*, UavItemWidget*)), this, SLOT(on_uavList_activeUavChanged(Uav*, UavItemWidget*)), Qt::UniqueConnection);
    item->setSizeHint(widget->minimumSize());
    this->ui->listUav->addItem(item);
    this->ui->listUav->setItemWidget(item, widget);
}

void FPVWindow::on_uavManager_uavUnregistered(unsigned int index, Uav* uav)
{
    std::cout << "remove uav number: " << index << std::endl;

    if (uav == this->activeUav)
    {
        std::cout << "reset activeUAV" << endl;
        this->activeUav = nullptr;
        this->activeUavItemWidget = nullptr;
    }
    this->ui->listUav->removeItemWidget(this->ui->listUav->takeItem(index));
}

void FPVWindow::on_uavManager_uavReseted()
{
    std::cout << "clear all uavs" << std::endl;

    std::cout << "reset activeUAV" << endl;
    this->activeUav = nullptr;
    this->activeUavItemWidget = nullptr;

    this->ui->listUav->clear();
}

void FPVWindow::on_uavList_activeUavChanged(Uav* activeUav, UavItemWidget* activeUavItemWidget)
{

    this->disconnectActiveUav();

    this->activeUav = activeUav;
    this->activeUavItemWidget = activeUavItemWidget;

    this->connectActiveUav();

}

void FPVWindow::on_activeUav_frameReceived(uint8_t* frame, uint width, uint height, uint bytesPerLine)
{
    //std::cout << "rendering..." << std::endl;


    unsigned char* newFrame = new unsigned char[width * height * 3];

    for (int i = 0; i < width * height * 3; i = i + 3)
    {
        newFrame[i] = frame[i + 2];
        newFrame[i + 1] = frame[i + 1];
        newFrame[i + 2] = frame[i];
    }

    this->ui->lblFPV->setPixmap((QPixmap::fromImage(QImage(newFrame, width, height, bytesPerLine, QImage::Format_RGB888))).scaled(this->ui->lblFPV->width(), this->ui->lblFPV->height(), Qt::KeepAspectRatio));

    delete newFrame;


    //this->ui->lblFPV->setPixmap((QPixmap::fromImage(QImage(frame, width, height, bytesPerLine, QImage::Format_RGB888))).scaled(this->ui->lblFPV->width(), this->ui->lblFPV->height(), Qt::KeepAspectRatio));
}

void FPVWindow::disconnectActiveUav()
{
    if (this->activeUav != NULL && this->activeUavItemWidget != NULL)
    {
        disconnect(this->activeUav, SIGNAL(frameReceived(uint8_t*, uint, uint, uint)), this, SLOT(on_activeUav_frameReceived(uint8_t*, uint, uint, uint)));
        this->activeUavItemWidget->setSelection(false);
    }
}

void FPVWindow::connectActiveUav()
{
    if (this->activeUav != NULL && this->activeUavItemWidget != NULL)
    {
        bool isConnected = connect(this->activeUav, SIGNAL(frameReceived(uint8_t*, uint, uint, uint)), this, SLOT(on_activeUav_frameReceived(uint8_t*, uint, uint, uint)), Qt::UniqueConnection);
        this->activeUavItemWidget->setSelection(true);

        if (isConnected)
        {
            std::cout << "display connected" << std::endl;
        }
    }
}

void FPVWindow::on_screenshotButton_clicked()
{
    const QPixmap* pix = this->ui->lblFPV->pixmap();
    if (pix != NULL)
    {
        pix->save("test.jpg", "JPG");
    }

    Exiv2::ExifData exifData;

    /*
     * float degrees = floor(lon);
float minutes = (lon - degrees) * 60;

sprintf(bufflon,"Longitude: %.0f degrees, %f minutes\n", degrees, minutes)

*/
    exifData["Exif.Image.Model"] = "Test 1";

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open("test.jpg");
    assert(image.get() != 0);
    image->setExifData(exifData);
    image->writeMetadata();
}
