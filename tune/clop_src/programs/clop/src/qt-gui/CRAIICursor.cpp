#include "CRAIICursor.h"
#include <QApplication>

CRAIICursor::CRAIICursor(const QCursor &cursor)
{
 QApplication::setOverrideCursor(cursor);
}

CRAIICursor::~CRAIICursor()
{
 QApplication::restoreOverrideCursor();
}
