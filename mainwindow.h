#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "metrosystem.h" // Assuming MetroSystem is correctly set up for metroFinalData.csv

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QTextEdit;
class QVBoxLayout;
class QPropertyAnimation;
class QGraphicsOpacityEffect;
class QProcess; // For launching Python script
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void findPath();
    // Optional slots for QProcess feedback
    // void onPythonProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    // void onPythonProcessError(QProcess::ProcessError error);

private:
    void setupUi();
    void populateComboBoxes();
    void loadData();

    MetroSystem metroSystem_;

    // UI Elements
    QComboBox *sourceComboBox_;
    QComboBox *destinationComboBox_;
    QComboBox *criteriaComboBox_;
    QPushButton *findPathButton_;
    QTextEdit *outputDisplay_;
    QGraphicsOpacityEffect *outputOpacityEffect_;

    QLabel *sourceLabel_;
    QLabel *destinationLabel_;
    QLabel *criteriaLabel_;
    QLabel *outputLabel_;

    QWidget* controlsWidget_; // To group controls (if using splitter, not strictly needed for simpler layout)
};

#endif // MAINWINDOW_H
