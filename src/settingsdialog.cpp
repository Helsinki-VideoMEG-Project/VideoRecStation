#include <QFileDialog>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"


SettingsDialog::SettingsDialog(QString* _cameraSNs, CameraSettingConstraints* _cameraSettingConstraints, unsigned int _numCameras, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint| Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    cameraSNs = _cameraSNs;
    cameraSettingConstraints = _cameraSettingConstraints;
    miscSettings = Settings::getInstance().getMiscSettings();

    for(unsigned int i=0; i<_numCameras; i++)
        ui->camerasList->addItem(cameraSNs[i]);

    ui->pathEdit->setText(miscSettings.storagePath);
}


SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::onCamSelectionChanged()
{
    CameraSettings cameraSettingsCopy;
    // If camera selection hasn't changed, do nothing
    if (currentRow == ui->camerasList->currentRow())
        return;

    if (currentRow != -1) {
        // Save settings for the previous camera
        Settings::getInstance().setCameraSettings(cameraSNs[currentRow], currentCameraSettings);
    }

    // Load settings for the new camera
    currentRow = ui->camerasList->currentRow();
    currentCameraSettingConstraints = cameraSettingConstraints[currentRow];
    currentCameraSettings = Settings::getInstance().getCameraSettings(cameraSNs[currentRow]);
    cameraSettingsCopy = currentCameraSettings;

    ui->widthSpinBox->setValue(cameraSettingsCopy.width);
    ui->heightSpinBox->setValue(cameraSettingsCopy.height);
    ui->offsetXSpinBox->setValue(cameraSettingsCopy.offsetX);
    ui->offsetYSpinBox->setValue(cameraSettingsCopy.offsetY);
    ui->lockCheckBox->setChecked(cameraSettingsCopy.useTrigger);
    if (cameraSettingsCopy.color)
        ui->colorRadioButton->setChecked(true);
    else
        ui->bwRadioButton->setChecked(true);

    updateCameraControls();
}


void SettingsDialog::updateCameraControls()
{
    // Assume that the values in the controls are valid.
    // Update control's labels, ranges/enable/disable statuses

    unsigned int minWidth = 8;
    unsigned int minHeight = 8;
    unsigned int maxWidth = currentCameraSettingConstraints.maxWidth - ui->offsetXSpinBox->value();
    unsigned int maxHeight = currentCameraSettingConstraints.maxHeight - ui->offsetYSpinBox->value();

    unsigned int minOffsetX = 0;
    unsigned int minOffsetY = 0;
    unsigned int maxOffsetX = currentCameraSettingConstraints.maxWidth - ui->widthSpinBox->value();
    unsigned int maxOffsetY = currentCameraSettingConstraints.maxHeight - ui->heightSpinBox->value();

    // Enable all the widgets
    for (int i = 0; i < ui->gridLayout->count(); ++i) {
        QWidget *widget = ui->gridLayout->itemAt(i)->widget();
        if (widget) {
            widget->setEnabled(true);
        }
    }

    ui->colorRadioButton->setEnabled(true);
    ui->bwRadioButton->setEnabled(true);

    // Update the ranges
    ui->widthSpinBox->setMinimum(minWidth);
    ui->widthSpinBox->setMaximum(maxWidth);
    ui->heightSpinBox->setMinimum(minHeight);
    ui->heightSpinBox->setMaximum(maxHeight);

    ui->offsetXSpinBox->setMinimum(minOffsetX);
    ui->offsetXSpinBox->setMaximum(maxOffsetX);
    ui->offsetYSpinBox->setMinimum(minOffsetY);
    ui->offsetYSpinBox->setMaximum(maxOffsetY);

    // Update the labels
    ui->widthLabel->setText(QString("Width (%1--%2):").arg(minWidth).arg(maxWidth));
    ui->heightLabel->setText(QString("Height (%1--%2):").arg(minHeight).arg(maxHeight));
    ui->offsetXLabel->setText(QString("Offset X (%1--%2):").arg(minOffsetX).arg(maxOffsetX));
    ui->offsetYLabel->setText(QString("Offset Y (%1--%2):").arg(minOffsetY).arg(maxOffsetY));
    
    // Enable/disable the controls
    ui->widthLabel->setEnabled(maxWidth > minWidth);
    ui->widthSpinBox->setEnabled(maxWidth > minWidth);
    ui->widthLabelPx->setEnabled(maxWidth > minWidth);
    
    ui->heightLabel->setEnabled(maxHeight > minHeight);
    ui->heightSpinBox->setEnabled(maxHeight > minHeight);
    ui->heightLabelPx->setEnabled(maxHeight > minHeight);
    
    ui->offsetXLabel->setEnabled(maxOffsetX > minOffsetX);
    ui->offsetXSpinBox->setEnabled(maxOffsetX > minOffsetX);
    ui->offsetXLabelPx->setEnabled(maxOffsetX > minOffsetX);
    
    ui->offsetYLabel->setEnabled(maxOffsetY > minOffsetY);
    ui->offsetYSpinBox->setEnabled(maxOffsetY > minOffsetY);
    ui->offsetYLabelPx->setEnabled(maxOffsetY > minOffsetY);
}


void SettingsDialog::onWidthChanged(int _newVal)
{
    currentCameraSettings.width = _newVal;
    updateCameraControls();
}


void SettingsDialog::onHeightChanged(int _newVal)
{
    currentCameraSettings.height = _newVal;
    updateCameraControls();
}


void SettingsDialog::onOffsetXChanged(int _newVal)
{
    currentCameraSettings.offsetX = _newVal;
    updateCameraControls();
}


void SettingsDialog::onOffsetYChanged(int _newVal)
{
    currentCameraSettings.offsetY = _newVal;
    updateCameraControls();
}


void SettingsDialog::onLockToggled(bool _checked)
{
    currentCameraSettings.useTrigger = _checked;
}


void SettingsDialog::onColorToggled(bool _checked)
{
    currentCameraSettings.color = _checked;
}


void SettingsDialog::accept()
{
    if (currentRow != -1)
        Settings::getInstance().setCameraSettings(cameraSNs[currentRow], currentCameraSettings);

    Settings::getInstance().setMiscSettings(miscSettings);
        
    QDialog::accept();
}


void SettingsDialog::onBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Directory"),
        ui->pathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!dir.isEmpty()) {
        ui->pathEdit->setText(dir);
        miscSettings.storagePath = dir;
    }
}


void SettingsDialog::closeEvent(QCloseEvent* _event)
{
    _event->ignore(); // Ignore the close event to prevent the window from closing
}
