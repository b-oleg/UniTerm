#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QAction>
#include <QPushButton>

class ActionButton : public QPushButton
{
    Q_OBJECT
private:
    QAction *actionOwner = nullptr;

public:
    explicit ActionButton(QWidget *parent = nullptr);
    void setAction(QAction *action);

public slots:
    void updateButtonStatusFromAction();

};

#endif // ACTIONBUTTON_H
