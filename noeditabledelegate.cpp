#include "noeditabledelegate.h"

NoEditableDelegate::NoEditableDelegate(QObject *parent)
{
}

QWidget* NoEditableDelegate::createEditor ( QWidget*, const QStyleOptionViewItem&, const QModelIndex& ) const
{
    return 0;
}

void NoEditableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    return;
}
void NoEditableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    return;
}

void NoEditableDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return;
}
