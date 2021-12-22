#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QRandomGenerator>
#include <QAbstractItemModel>
#include <QGuiApplication>
#include "qqml.h"


struct Tile
{
    enum Color {
        NO_COLOR = 0,
        GREEN,
        BLUE,
        YELLO,
        BROWN
    };
    int pos;
    Color color = NO_COLOR;
    friend QDebug operator << (QDebug debug, const Tile *tile) {
        debug << "color= " + QString::number(tile->color);
        return debug;
    }
};

class ItemModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT
    Tile m_tiles[81];
    int getRandomEmptyPos();
    QList<QVariant> findHLines(int index);
    QList<QVariant> findVLines(int index);
    int m_score = 0;
    QVariantList m_toDelete;
    bool m_finished = false;
    QSqlError initDb();
    bool loadData();
    QString m_state ="start";

public:
    enum Roles {
        ColorRole = 0,
    };
    explicit ItemModel(QObject *parent = nullptr);
    Q_PROPERTY(int score READ score WRITE setScore NOTIFY scoreChanged)
    Q_PROPERTY( QList<QVariant>  toDelete READ toDelete WRITE setToDelete NOTIFY toDeleteChanged)
    Q_PROPERTY(bool finished READ finished WRITE setFinished NOTIFY finishedChanged)
    Q_PROPERTY(QString state READ state WRITE setState NOTIFY stateChanged)

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void swap(int first, int second);
    Q_INVOKABLE void setupColors();
    Q_INVOKABLE bool findLines(int index);
    Q_INVOKABLE void clearColor(int pos);
    Q_INVOKABLE void restart();

    int score() const;
    void setScore(int newScore);
    const  QList<QVariant>  &toDelete() const;
    void setToDelete(const  QList<QVariant>   &newToDelete);

    bool finished() const;
    void setFinished(bool newFinished);

    const QString &state() const;
    void setState(const QString &newState);

signals:
    void scoreChanged();
    void toDeleteChanged();
    void finishedChanged();
    void stateChanged();
};

#endif // ITEMMODEL_H
