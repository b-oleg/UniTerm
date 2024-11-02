#include "actionbutton.h"

ActionButton::ActionButton(QWidget *parent) :QPushButton(parent) {}

void ActionButton::setAction(QAction *action) {
    if (actionOwner && actionOwner != action) {
        disconnect(actionOwner, &QAction::changed, this, &ActionButton::updateButtonStatusFromAction);
        disconnect(this, &ActionButton::clicked, actionOwner, &QAction::trigger);
    }
    actionOwner = action;
    updateButtonStatusFromAction();
    connect(action, &QAction::changed, this, &ActionButton::updateButtonStatusFromAction);
    connect(this, &ActionButton::clicked, actionOwner, &QAction::trigger);
}

void ActionButton::updateButtonStatusFromAction() {
    if (!actionOwner) return;
    //setText(actionOwner->text());
    setStatusTip(actionOwner->statusTip());
    setToolTip(actionOwner->toolTip());
    //setIcon(actionOwner->icon());
    setEnabled(actionOwner->isEnabled());
    setCheckable(actionOwner->isCheckable());
    setChecked(actionOwner->isChecked());
}
