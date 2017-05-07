#ifndef NOEDITABLEDELEGATE_H
#define NOEDITABLEDELEGATE_H

#include <QItemDelegate>

class NoEditableDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    NoEditableDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // NOEDITABLEDELEGATE_H
