#include "console.h"
#include <QScrollBar>
#include <QFont>

Console::Console(QWidget *parent): QPlainTextEdit(parent) {
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    document()->setDefaultFont(fixedFont);

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    setReadOnly(true);
}

void Console::putData(const QByteArray &data) {    
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    insertPlainText(QString::fromLocal8Bit(data));
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Alt:
    case Qt::Key_Shift:
    case Qt::Key_Control:
        QPlainTextEdit::keyPressEvent(e);
        break;
    default:
        emit getData(e->text().toLocal8Bit());
    }
}
