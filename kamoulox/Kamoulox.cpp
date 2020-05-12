#include "Kamoulox.h"
#include <QStandardPaths>
#include <iostream>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileInfo>
#include <QDate>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include "ui_Kamoulox.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include <fstream>

Kamoulox::Kamoulox(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Kamoulox)
{
    ui->setupUi(this);

    QString configFilePath = QStandardPaths::locate(QStandardPaths::ConfigLocation, "kamoulox.json");

    if(configFilePath.isEmpty())
    {
        std::cerr << "Failed finding kamoulox.json in " << QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).join(", ").toStdString() << std::endl;
        return;
    }

    rapidjson::Document pouet;
    std::cout << "Opening " << configFilePath.toStdString() << std::endl;

    // Read models
    std::ifstream file(configFilePath.toStdString());
    if(file.is_open())
    {
        rapidjson::IStreamWrapper isw(file);

        rapidjson::Document document;
        rapidjson::ParseResult parseFileOk = document.ParseStream(isw);
        if (!parseFileOk)
        {
            std::cerr << "Failed parsing file " << configFilePath.toStdString() << std::endl;
            return;
        }

        if(document.HasMember("models") && document["models"].IsArray())
        {
            const rapidjson::Value& models = document["models"];
            for (rapidjson::SizeType index = 0; index < models.Size(); ++index)
            {
                const rapidjson::Value& model = models[index];
                // assume well-formed
                std::pair<std::string, std::string> currentModel;
                currentModel.first = model["name"].GetString();
                currentModel.second = model["path"].GetString();
                std::cout << "Adding model " << currentModel.first << ": " << currentModel.second << std::endl;
                m_models.push_back(currentModel);
            }
        }

        if(document.HasMember("categories") && document["categories"].IsArray())
        {
            const rapidjson::Value& categories = document["categories"];
            for (rapidjson::SizeType index = 0; index < categories.Size(); ++index)
            {
                std::vector<std::string> currentCategory;
                const rapidjson::Value& entry = categories[index]["entries"];
                if(entry.IsArray())
                {
                    for (rapidjson::SizeType index2 = 0; index2 < entry.Size(); ++index2)
                    {
                        currentCategory.push_back(entry[index2].GetString());
                    }
                }

                m_categories.push_back(currentCategory);
            }

        }

        m_baseFolder = document["baseFolder"].GetString();
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    ui->centralWidget->setLayout(layout);

    QGroupBox* modelGroupBox = new QGroupBox("Modèles", this);
    layout->addWidget(modelGroupBox);
    QVBoxLayout* modelLayout = new QVBoxLayout(this);
    modelGroupBox->setLayout(modelLayout);
    m_model = new QComboBox(this);
    modelLayout->addWidget(m_model);
    for(const auto& model: m_models)
    {
        m_model->addItem(QString::fromStdString(model.first));
    }

    QGroupBox* catGroupBox = new QGroupBox("Catégories", this);
    layout->addWidget(catGroupBox);
    QVBoxLayout* catLayout = new QVBoxLayout(this);
    catGroupBox->setLayout(catLayout);
    for(const auto& category: m_categories)
    {
        QComboBox* combo = new QComboBox(this);
        m_combos.push_back(combo);

        catLayout->addWidget(combo);
        for(const auto& val: category)
        {
            combo->addItem(QString::fromStdString(val));
        }
    }

    QGroupBox* nameGroupBox = new QGroupBox("Nom de fichier", this);
    QVBoxLayout* nameLayout = new QVBoxLayout(this);
    nameGroupBox->setLayout(nameLayout);
    m_name = new QLineEdit(this);
    nameLayout->addWidget(m_name);

    layout->addWidget(nameGroupBox);

    QPushButton* button = new QPushButton("Hop !", this);
    layout->addWidget(button);

    connect(button, SIGNAL(clicked()), this, SLOT(go()));

}

Kamoulox::~Kamoulox()
{
    delete ui;
}

void Kamoulox::go()
{
    const auto& model = m_models[m_model->currentIndex()];
    QString modelFile = QString::fromStdString(model.second);
    QFileInfo fileInfo(modelFile);
    QString extension = fileInfo.suffix();

    std::cout << "Copying " << modelFile.toStdString() << " with extension " << extension.toStdString() << std::endl;

    QString folder = QString::fromStdString(m_baseFolder);
    for(const auto combo: m_combos)
    {
        folder.append("/");
        folder.append(combo->currentText());
    }
    folder.append("/");

    QDir dir(folder);
    if (!dir.exists())
        dir.mkpath(".");

    QString targetFileName = QDate::currentDate().toString("yyyy-MM-dd") + QString("_") + m_name->text() + QString(".") + extension;

    targetFileName = folder + targetFileName;

    if(QFile::copy(modelFile, targetFileName))
    {

        QDesktopServices::openUrl(QUrl::fromLocalFile(targetFileName));
    }
    else
    {
        std::cerr << "Could not copy " << modelFile.toStdString() << " to " << targetFileName.toStdString() << std::endl;
    }


}
