#ifndef UAVITEMWIDGET_H
#define UAVITEMWIDGET_H

#include <QWidget>

#include "uav.h"

namespace Ui {
class UavItemWidget;
}

class UavItemWidget : public QWidget
{
    Q_OBJECT

signals:
    void activeUavChanged(Uav* activeUav, UavItemWidget* activeUavItemWidget);

public:
    explicit UavItemWidget(Uav* uav, QWidget *parent = 0);
    ~UavItemWidget();

    void setSelection(bool isSelected);

private:
    Ui::UavItemWidget* ui;

    Uav* uav;

private slots:
    void on_buttonStartVideostream_clicked();
    void on_buttonStopVideostream_clicked();
    void on_buttonShowVideostream_clicked();

    void on_uav_videostreamStateChanged(bool isLiveStreamOn);
};

#endif // UAVITEMWIDGET_H
