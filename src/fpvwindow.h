#ifndef FPVWINDOW_H
#define FPVWINDOW_H

#include <QMainWindow>

#include <exiv2/exiv2.hpp>

#include "uavmanager.h"
#include "uav.h"

#include "uavitemwidget.h"

namespace Ui {
class FPVWindow;
}

class FPVWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::FPVWindow* ui;

    UavManager* uavManager;

    Uav* activeUav;
    UavItemWidget* activeUavItemWidget;


public:
    explicit FPVWindow(QWidget *parent = 0);
    ~FPVWindow();

private slots:
    void on_uavManager_uavRegistered(unsigned int index, Uav* uav);
    void on_uavManager_uavUnregistered(unsigned int index, Uav* uav);
    void on_uavManager_uavReseted();

    void on_uavList_activeUavChanged(Uav* activeUav, UavItemWidget* activeUavItemWidget);

    void on_activeUav_frameReceived(uint8_t* frame, uint width, uint height, uint bytesPerLine);

    void on_screenshotButton_clicked();

private:
    void disconnectActiveUav();
    void connectActiveUav();

};

#endif // FPVWINDOW_H
