#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include "settings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QString* _cameraSNs, CameraSettingConstraints* _cameraSettingConstraints, unsigned int _numCameras, QWidget *parent = nullptr);
    ~SettingsDialog();

public slots:
    void onCamSelectionChanged();
    void onWidthChanged(int _newVal);
    void onHeightChanged(int _newVal);
    void onOffsetXChanged(int _newVal);
    void onOffsetYChanged(int _newVal);
    void onLockToggled(bool _checked);
    void onColorToggled(bool _checked);
    void onBrowse();
    void accept();
    void closeEvent(QCloseEvent* _event);

private:
    Ui::SettingsDialog *ui;

    QString*                    cameraSNs;
    CameraSettingConstraints*   cameraSettingConstraints;

    int                         currentRow = -1;
    CameraSettings              currentCameraSettings;
    CameraSettingConstraints    currentCameraSettingConstraints;
    MiscSettings                miscSettings;

    void updateCameraControls();
};

#endif // SETTINGSDIALOG_H
