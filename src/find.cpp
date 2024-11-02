#include "find.h"
#include "ui_find.h"

#include <QRegularExpression>

DialogFind::DialogFind(QPlainTextEdit *editor, QWidget *parent): QDialog(parent), m_ui(new Ui::DialogFind), m_editor(editor) {
    m_ui->setupUi(this);
    connect(m_ui->pushButtonFind, &QPushButton::clicked, this, &DialogFind::find);
    connect(m_ui->pushButtonCancel, &QPushButton::clicked, this, &DialogFind::close);
    connect(m_ui->horizontalSliderOpacity, &QSlider::valueChanged, this, [=](int value) {
        setWindowOpacity(double(value) / m_ui->horizontalSliderOpacity->maximum());
    });
}

DialogFind::~DialogFind() {
    delete m_ui;
}

void DialogFind::setOpacity(double value) {
    m_ui->horizontalSliderOpacity->setValue(qRound(m_ui->horizontalSliderOpacity->maximum() * value));
}

void DialogFind::find() {
    QTextDocument::FindFlags flag;
    if (m_ui->radioButtonDirectionUp->isChecked()) flag |= QTextDocument::FindBackward;
    if (m_ui->checkBoxCaseSensitive->isChecked()) flag |= QTextDocument::FindCaseSensitively;

    if (m_ui->checkBoxRegEx->isChecked()) {
        QRegularExpression re(m_ui->lineEditWhat->text());
        m_editor->find(re, flag);
    } else {
        m_editor->find(m_ui->lineEditWhat->text(), flag);
    }
}
