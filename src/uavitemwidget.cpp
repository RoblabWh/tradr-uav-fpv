#include "uavitemwidget.h"
#include "ui_uavitemwidget.h"

UavItemWidget::UavItemWidget(Uav* uav, QWidget *parent) : QWidget(parent)
{    
    this->ui = new Ui::UavItemWidget();
    this->ui->setupUi(this);

    this->uav = uav;

    this->ui->lblID->setText(QString::number(this->uav->getID()));
    this->ui->lblName->setText(this->uav->getName());
    this->ui->lblModel->setText(this->uav->getModel());

    this->ui->buttonStartVideostream->setEnabled(!(this->uav->isLiveStreamOn()));
    this->ui->buttonStopVideostream->setEnabled(this->uav->isLiveStreamOn());

    connect(this->ui->buttonStartVideostream, SIGNAL(clicked()), this, SLOT(on_buttonStartVideostream_clicked()), Qt::UniqueConnection);
    connect(this->ui->buttonStopVideostream, SIGNAL(clicked()), this, SLOT(on_buttonStopVideostream_clicked()), Qt::UniqueConnection);
    connect(this->ui->buttonShowVideostream, SIGNAL(clicked()), this, SLOT(on_buttonShowVideostream_clicked()), Qt::UniqueConnection);

    connect(this->uav, SIGNAL(videostreamStateChanged(bool)), this, SLOT(on_uav_videostreamStateChanged(bool)), Qt::UniqueConnection);
}

UavItemWidget::~UavItemWidget()
{
    delete ui;
}

void UavItemWidget::setSelection(bool isSelected)
{
    if (isSelected)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, Qt::blue);
        this->setPalette(pal);
    }
    else
    {
        QPalette pal;
        pal.setColor(QPalette::Background, Qt::gray);
        this->setPalette(pal);
    }
}

void UavItemWidget::on_buttonStartVideostream_clicked()
{
    this->uav->startLiveStream();
}

void UavItemWidget::on_buttonStopVideostream_clicked()
{
    this->uav->stopLiveStream();
}

void UavItemWidget::on_buttonShowVideostream_clicked()
{
    emit activeUavChanged(this->uav, this);
}

void UavItemWidget::on_uav_videostreamStateChanged(bool isLiveStreamOn)
{
    this->ui->buttonStartVideostream->setEnabled(!isLiveStreamOn);
    this->ui->buttonStopVideostream->setEnabled(isLiveStreamOn);
}
