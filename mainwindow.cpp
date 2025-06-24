#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QStringList>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>
#include <QFont>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QProcess>         // <<<< For launching Python
#include <QJsonDocument>    // <<<< For creating JSON for Python
#include <QJsonObject>      // <<<<
#include <QJsonArray>       // <<<<

// getLineColor function (can be shared or duplicated if not in a common utility)
QString getLineColor(const QString& lineName) {
    QString lowerLineName = lineName.toLower();
    if (lowerLineName.contains("blue")) return "blue";
    if (lowerLineName.contains("red")) return "red";
    if (lowerLineName.contains("green")) return "green";
    if (lowerLineName.contains("yellow")) return "darkgoldenrod";
    if (lowerLineName.contains("pink")) return "deeppink";
    if (lowerLineName.contains("magenta")) return "magenta";
    if (lowerLineName.contains("orange")) return "orange";
    if (lowerLineName.contains("aqua")) return "darkcyan";
    if (lowerLineName.contains("violet") || lowerLineName.contains("voilet")) return "darkviolet";
    if (lowerLineName.contains("gray") || lowerLineName.contains("grey")) return "gray";
    if (lowerLineName.contains("rapid")) return "saddlebrown";
    return "black";
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
    loadData();
    connect(findPathButton_, &QPushButton::clicked, this, &MainWindow::findPath);
}

MainWindow::~MainWindow() {
    // QObject parent-child mechanism will clean up most widgets.
}

void MainWindow::setupUi() {
    setWindowTitle("Metro Route Finder"); // Simplified title

    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // Using a single GridLayout for simplicity for now
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    sourceLabel_ = new QLabel("Source Station:", this);
    sourceComboBox_ = new QComboBox(this);
    sourceComboBox_->setEditable(false);
    sourceComboBox_->setMinimumWidth(250);

    destinationLabel_ = new QLabel("Destination Station:", this);
    destinationComboBox_ = new QComboBox(this);
    destinationComboBox_->setEditable(false);
    destinationComboBox_->setMinimumWidth(250);

    criteriaLabel_ = new QLabel("Optimize By:", this);
    criteriaComboBox_ = new QComboBox(this);
    criteriaComboBox_->addItem("Least Stops", 1);
    criteriaComboBox_->addItem("Least Cost", 2);
    criteriaComboBox_->addItem("Least Time", 3);
    criteriaComboBox_->setMinimumWidth(250);

    findPathButton_ = new QPushButton("Find Route", this);

    outputLabel_ = new QLabel("Route Details:", this);
    outputDisplay_ = new QTextEdit(this);
    outputDisplay_->setReadOnly(true);
    outputDisplay_->setMinimumHeight(200); // Good height for text output
    outputDisplay_->setFont(QFont("Arial", 10));

    outputOpacityEffect_ = new QGraphicsOpacityEffect(this);
    outputDisplay_->setGraphicsEffect(outputOpacityEffect_);
    outputOpacityEffect_->setOpacity(1.0); // Start visible

    // Layouting with QVBoxLayouts for label+combobox pairs
    QVBoxLayout* sourceVLayout = new QVBoxLayout();
    sourceVLayout->addWidget(sourceLabel_);
    sourceVLayout->addWidget(sourceComboBox_);
    sourceVLayout->setSpacing(2);

    QVBoxLayout* destVLayout = new QVBoxLayout();
    destVLayout->addWidget(destinationLabel_);
    destVLayout->addWidget(destinationComboBox_);
    destVLayout->setSpacing(2);

    QVBoxLayout* criteriaVLayout = new QVBoxLayout();
    criteriaVLayout->addWidget(criteriaLabel_);
    criteriaVLayout->addWidget(criteriaComboBox_);
    criteriaVLayout->setSpacing(2);

    mainLayout->addLayout(sourceVLayout, 0, 0);
    mainLayout->addLayout(destVLayout, 1, 0);
    mainLayout->addLayout(criteriaVLayout, 2, 0);
    mainLayout->addWidget(findPathButton_, 3, 0, Qt::AlignCenter);
    mainLayout->addWidget(outputLabel_, 4, 0, Qt::AlignLeft);
    mainLayout->addWidget(outputDisplay_, 5, 0); // Output display takes the rest of this column

    mainLayout->setRowStretch(5, 1); // outputDisplay_ expands vertically
    mainLayout->setColumnStretch(0, 1); // The first column expands horizontally

    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setVerticalSpacing(10);

    centralWidget->setLayout(mainLayout);
    setMinimumSize(450, 550); // Adjusted for a single column layout
}

void MainWindow::loadData() {
    std::string errorMsg;
    // Ensure this matches your 10-column CSV for segment data + Lat/Lon
    std::string dataFileName = "metroFinalData.csv";
    std::string dataFilePath = QCoreApplication::applicationDirPath().toStdString() + "/" + dataFileName;
    qDebug() << "Attempting to load data from (app dir):" << QString::fromStdString(dataFilePath);

    if (!metroSystem_.loadMetroData(dataFilePath, errorMsg)) {
        std::string relativeDataFilePath = dataFileName;
        qDebug() << "Failed to load from app dir. Attempting to load data from (relative path):" << QString::fromStdString(relativeDataFilePath);
        if (!metroSystem_.loadMetroData(relativeDataFilePath, errorMsg)) {
            QMessageBox::critical(this, "Error Loading Data", QString::fromStdString(errorMsg + "\nPlease ensure '" + dataFileName + "' is in the application directory or the current working directory. Check Application Output for parsing details."));
            sourceComboBox_->setEnabled(false);
            destinationComboBox_->setEnabled(false);
            criteriaComboBox_->setEnabled(false);
            findPathButton_->setEnabled(false);
            outputDisplay_->setHtml("<b>Failed to load metro data.</b> Application may not function correctly.");
            return;
        }
    }
    populateComboBoxes();
}

void MainWindow::populateComboBoxes() {
    std::vector<std::string> stations = metroSystem_.getStationNames();
    QStringList stationList;
    for (const auto& s : stations) {
        stationList.append(QString::fromStdString(s));
    }
    sourceComboBox_->clear();
    destinationComboBox_->clear();
    if (stationList.isEmpty()) {
        outputDisplay_->setHtml("<b>No stations loaded. Check data file and format. See Application Output for parsing details.</b>");
        sourceComboBox_->setEnabled(false);
        destinationComboBox_->setEnabled(false);
        criteriaComboBox_->setEnabled(false);
        findPathButton_->setEnabled(false);
    } else {
        sourceComboBox_->addItems(stationList);
        destinationComboBox_->addItems(stationList);
        outputDisplay_->setHtml("<p>Select source, destination, and optimization criteria, then click 'Find Route'.</p>");
        sourceComboBox_->setEnabled(true);
        destinationComboBox_->setEnabled(true);
        criteriaComboBox_->setEnabled(true);
        findPathButton_->setEnabled(true);
    }
}

void MainWindow::findPath() {
    outputOpacityEffect_->setOpacity(0.0); // Make text output transparent for animation

    if (sourceComboBox_->currentIndex() < 0 || destinationComboBox_->currentIndex() < 0) {
        QMessageBox::warning(this, "Input Missing", "Please select both source and destination stations.");
        outputOpacityEffect_->setOpacity(1.0);
        return;
    }
    std::string sourceStdStr = sourceComboBox_->currentText().toStdString();
    std::string destStdStr = destinationComboBox_->currentText().toStdString();
    int criteriaChoice = criteriaComboBox_->currentData().toInt();
    QString criteriaText = criteriaComboBox_->currentText();
    QString qSource = QString::fromStdString(sourceStdStr);
    QString qDest = QString::fromStdString(destStdStr);

    QString htmlOutputContent;
    std::vector<PathSegment> pathSegments;

    if (sourceStdStr == destStdStr) {
        htmlOutputContent = QString("<h3>Route from %1 to %2</h3><hr><p>Source and Destination are the same station: <b>%3</b></p>")
        .arg(qSource.toHtmlEscaped(), qDest.toHtmlEscaped(), qSource.toHtmlEscaped());
    } else {
        switch (criteriaChoice) {
        case 1: pathSegments = metroSystem_.findPathLeastStops(sourceStdStr, destStdStr); break;
        case 2: pathSegments = metroSystem_.findPathByCost(sourceStdStr, destStdStr); break;
        case 3: pathSegments = metroSystem_.findPathByTime(sourceStdStr, destStdStr); break;
        default:
            QMessageBox::critical(this, "Error", "Invalid criteria selected.");
            outputOpacityEffect_->setOpacity(1.0);
            return;
        }

        if (pathSegments.empty()) {
            htmlOutputContent = QString("<h3>Route from %1 to %2</h3><hr><p><b>No path found.</b></p><p><i>Criteria: %3</i></p>")
            .arg(qSource.toHtmlEscaped(), qDest.toHtmlEscaped(), criteriaText.toHtmlEscaped());
        } else {
            // --- Build HTML for textual output ---
            htmlOutputContent = QString("<h3>Route from %1 to %2</h3>").arg(qSource.toHtmlEscaped(), qDest.toHtmlEscaped());
            htmlOutputContent += QString("<p><i>Optimized for: %1</i></p><hr>").arg(criteriaText.toHtmlEscaped());
            long long totalTime = 0; long long totalCost = 0; int lineChanges = 0; int totalSegments = 0;
            QString prevLineForSummary = "";
            if (pathSegments.size() > 1) {
                for (size_t i = 1; i < pathSegments.size(); ++i) {
                    const auto& seg = pathSegments[i];
                    totalTime += seg.timeForSegment; totalCost += seg.costForSegment; totalSegments++;
                    QString currentSegLine = QString::fromStdString(seg.lineTakenToReach);
                    if (!currentSegLine.isEmpty() && currentSegLine != prevLineForSummary && !prevLineForSummary.isEmpty()) lineChanges++;
                    prevLineForSummary = currentSegLine;
                }
            }
            htmlOutputContent += "<h4>Summary:</h4><ul>";
            htmlOutputContent += QString("<li><b>Total Stations in Path:</b> %1 (%2 hops)</li>").arg(pathSegments.size()).arg(totalSegments);
            htmlOutputContent += QString("<li><b>Estimated Time:</b> %1 minutes</li>").arg(totalTime);
            htmlOutputContent += QString("<li><b>Estimated Cost:</b> INR %1</li>").arg(totalCost);
            htmlOutputContent += QString("<li><b>Line Changes:</b> %1</li>").arg(lineChanges);
            htmlOutputContent += "</ul><hr><h4>Directions:</h4><ol>";
            QString currentActiveLine = "";
            for (size_t i = 0; i < pathSegments.size(); ++i) {
                const auto& segment = pathSegments[i];
                QString qStationName = QString::fromStdString(segment.stationName);
                QString stationNameHtml = QString("<b>%1</b>").arg(qStationName.toHtmlEscaped());
                if (segment.isFirstSegment) {
                    htmlOutputContent += QString("<li>Start at %1.</li>").arg(stationNameHtml);
                    if (pathSegments.size() > 1) {
                        currentActiveLine = QString::fromStdString(pathSegments[i+1].lineTakenToReach);
                        if (!currentActiveLine.isEmpty()){
                            QString firstLineHtml = QString("<font color='%1'>%2</font>").arg(getLineColor(currentActiveLine)).arg(currentActiveLine.toHtmlEscaped());
                            htmlOutputContent += QString("<li>Board %1.</li>").arg(firstLineHtml);
                        }
                    }
                } else {
                    QString qLineTaken = QString::fromStdString(segment.lineTakenToReach);
                    QString lineHtml = "";
                    if(!qLineTaken.isEmpty()) lineHtml = QString("<font color='%1'>%2</font>").arg(getLineColor(qLineTaken)).arg(qLineTaken.toHtmlEscaped());
                    if (!qLineTaken.isEmpty() && qLineTaken != currentActiveLine && !currentActiveLine.isEmpty()) htmlOutputContent += QString("<li>Change to %1.</li>").arg(lineHtml);
                    currentActiveLine = qLineTaken;
                    htmlOutputContent += QString("<li>Arrive at %1 ").arg(stationNameHtml);
                    if (!lineHtml.isEmpty()) htmlOutputContent += QString("via %1 ").arg(lineHtml);
                    htmlOutputContent += QString("(Segment: %1 min, INR %2).</li>").arg(segment.timeForSegment).arg(segment.costForSegment);
                }
            }
            htmlOutputContent += "</ol>";
            // --- End of HTML generation ---

            // --- Launch Python Script for Map Visualization ---
            if (!pathSegments.empty()) {
                QStringList pythonArgs;
                QString pythonScriptPath = QCoreApplication::applicationDirPath() + "/map_generator.py";

                pythonArgs << pythonScriptPath;

                QJsonArray pathForPythonJsonArray;
                const auto& stationCoordsMap = metroSystem_.getStationCoordinates();

                for (const auto& seg : pathSegments) {
                    auto it = stationCoordsMap.find(seg.stationName);
                    if (it != stationCoordsMap.end()) {
                        QJsonObject stationObj;
                        stationObj["name"] = QString::fromStdString(seg.stationName);
                        stationObj["lat"] = it->second.y(); // Latitude from QPointF's y()
                        stationObj["lng"] = it->second.x(); // Longitude from QPointF's x()
                        pathForPythonJsonArray.append(stationObj);
                    } else {
                        qWarning() << "findPath (for Python): Coordinate not found for station:" << QString::fromStdString(seg.stationName);
                    }
                }

                if (!pathForPythonJsonArray.isEmpty()) {
                    QJsonDocument doc(pathForPythonJsonArray);
                    QString jsonDataString = doc.toJson(QJsonDocument::Compact);
                    pythonArgs << jsonDataString;

                    QProcess *pythonMapProcess = new QProcess(this);

                    // Connect signals for debugging python script execution (optional)
                    connect(pythonMapProcess, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error){
                        qWarning() << "Python script error occurred:" << error << pythonMapProcess->readAllStandardError();
                        pythonMapProcess->deleteLater();
                    });
                    connect(pythonMapProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                            [=](int exitCode, QProcess::ExitStatus exitStatus){
                                qDebug() << "Python script finished. Exit code:" << exitCode << "Exit status:" << exitStatus;
                                if (exitStatus == QProcess::CrashExit || exitCode != 0) {
                                    qWarning() << "Python script seems to have failed. STDERR:" << pythonMapProcess->readAllStandardError();
                                } else {
                                    qDebug() << "Python script STDOUT:" << pythonMapProcess->readAllStandardOutput();
                                }
                                pythonMapProcess->deleteLater();
                            });


                    QString pythonExecutable = "python3"; // Or "python"
#ifdef Q_OS_WIN
                    pythonExecutable = "python"; // Or full path to python.exe if not in PATH
#endif

                    qDebug() << "Launching Python Map:" << pythonExecutable << pythonArgs;
                    // Using startDetached because we don't need to wait for it,
                    // and we want its window to be independent.
                    bool started = pythonMapProcess->startDetached(pythonExecutable, pythonArgs);
                    if(!started) {
                        qWarning() << "Failed to start Python map process (startDetached failed):" << pythonMapProcess->errorString();
                        delete pythonMapProcess; // Clean up if startDetached itself fails
                    }
                    // If you don't use startDetached, you must manage the QProcess object more carefully
                    // or connect its finished signal to deleteLater().
                }
            }
        }
    }

    outputDisplay_->setHtml(htmlOutputContent);
    // Textual output fade-in animation
    QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(outputOpacityEffect_, "opacity", this);
    fadeInAnimation->setDuration(400);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    fadeInAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}
