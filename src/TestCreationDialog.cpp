#include "TestCreationDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>

TestCreationDialog::TestCreationDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Создание теста");
    auto *layout = new QVBoxLayout(this);

    nameEdit = new QLineEdit(this);
    descriptionEdit = new QTextEdit(this);
    forbiddenEdit = new QLineEdit(this);
    inputEdit = new QTextEdit(this);
    expectedOutputEdit = new QTextEdit(this);

    layout->addWidget(new QLabel("Название теста:"));
    layout->addWidget(nameEdit);

    layout->addWidget(new QLabel("Описание:"));
    layout->addWidget(descriptionEdit);

    layout->addWidget(new QLabel("Запрещённые конструкции (через запятую):"));
    layout->addWidget(forbiddenEdit);

    layout->addWidget(new QLabel("Входные данные:"));
    layout->addWidget(inputEdit);

    layout->addWidget(new QLabel("Ожидаемый вывод:"));
    layout->addWidget(expectedOutputEdit);

    auto *buttonLayout = new QHBoxLayout();

    auto *saveButton = new QPushButton("Сохранить", this);
    auto *cancelButton = new QPushButton("Отмена", this);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);

    connect(saveButton, &QPushButton::clicked, this, &TestCreationDialog::saveTest);
    connect(cancelButton, &QPushButton::clicked, this, &TestCreationDialog::reject);
}


TestCreationDialog::TestCreationDialog(const QString &filePath, QWidget *parent)
    : TestCreationDialog(parent) {
    loadTest(filePath);
}

void TestCreationDialog::loadTest(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл теста.");
        reject();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        QMessageBox::critical(this, "Ошибка", "Неверный формат файла теста.");
        reject();
        return;
    }

    QJsonObject obj = doc.object();

    QString testName = obj.value("name").toString();
    QString description = obj.value("description").toString();
    QJsonArray forbiddenArray = obj.value("forbidden").toArray();
    QString input = obj.value("input").toString();
    QString expected = obj.value("expected").toString();

    nameEdit->setText(testName);
    descriptionEdit->setText(description);

    QStringList forbiddenList;
    for (const QJsonValue &val : forbiddenArray) {
        forbiddenList << val.toString();
    }
    forbiddenEdit->setText(forbiddenList.join(", "));

    inputEdit->setText(input);
    expectedOutputEdit->setText(expected);
}


void TestCreationDialog::saveTest() {
    QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации", "Название теста не может быть пустым.");
        return;
    }

    QString description = descriptionEdit->toPlainText();
    QString forbiddenText = forbiddenEdit->text();
    QStringList forbiddenList = forbiddenText.split(",", Qt::SkipEmptyParts);

    for (QString &item : forbiddenList)
        item = item.trimmed();

    QString input = inputEdit->toPlainText();
    QString expected = expectedOutputEdit->toPlainText();

    QJsonObject obj;
    obj["name"] = name;
    obj["description"] = description;

    QJsonArray forbiddenArray;
    for (const QString &s : forbiddenList)
        forbiddenArray.append(s);

    obj["forbidden"] = forbiddenArray;
    obj["input"] = input;
    obj["expected"] = expected;

    QJsonDocument doc(obj);

    QDir dir(QCoreApplication::applicationDirPath() + "/tests");
    if (!dir.exists())
        dir.mkpath(".");

    QString filePath = dir.filePath(name + ".json");
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось записать файл теста.");
        return;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QMessageBox::information(this, "Сохранено", "Тест успешно сохранён.");
    accept();
}
