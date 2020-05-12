#ifndef KAMOULOX_H
#define KAMOULOX_H

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>

namespace Ui {
class Kamoulox;
}

class Kamoulox : public QMainWindow
{
    Q_OBJECT

public:
    explicit Kamoulox(QWidget *parent = 0);
    ~Kamoulox();

protected slots:
    void go();

private:
    Ui::Kamoulox *ui;

    std::vector<std::pair<std::string, std::string>> m_models;
    std::vector<std::vector<std::string>> m_categories;
    QComboBox* m_model;
    std::list<QComboBox*> m_combos;
    QLineEdit* m_name;
    std::string m_baseFolder;
};

#endif // KAMOULOX_H
