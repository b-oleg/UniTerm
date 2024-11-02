#ifndef FIND_H
#define FIND_H

#include <QDialog>
#include <QPlainTextEdit>

namespace Ui {
class DialogFind;
}

class DialogFind : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFind(QPlainTextEdit *editor, QWidget *parent = nullptr);
    ~DialogFind();

    void setOpacity(double value);

private slots:
    void find();

private:
    Ui::DialogFind *m_ui;
    QPlainTextEdit *m_editor;

};

#endif // FIND_H
