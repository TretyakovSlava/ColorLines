#include "itemmodel.h"

ItemModel::ItemModel(QObject *parent) : QAbstractItemModel(parent)
{
    for (int k = 0; k < 81; ++k) {
        m_tiles[k] = Tile{k};
    }
    QSqlError err = initDb();
    if(err.type() == QSqlError::NoError){
        if(loadData()){
            setState("game");
        }
    }
}

bool ItemModel::loadData()
{
    bool result = false;
    QSqlQuery query;
    query.exec("SELECT id, color FROM circles");

    while (query.next()) {
        result = true;
        int pos = query.value(0).toInt();
        int color = query.value(1).toInt();
        m_tiles[pos].color = (Tile::Color)color;
    }
    dataChanged(index(0,0,QModelIndex()),index(80,0,QModelIndex()),{0});

    query.exec("SELECT score FROM score");
    while (query.next()) {
        result = true;
        int score = query.value(0).toInt();
        setScore(score);
    }

    return result;
}

QSqlError ItemModel::initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QGuiApplication::applicationDirPath() + "/colorLines.db");

    if (!db.open())
        return db.lastError();

    db.exec("PRAGMA synchronous = OFF");
    db.exec("PRAGMA journal_mode = MEMORY");



    QStringList tables = db.tables();
    if (tables.contains("circles", Qt::CaseInsensitive)
            && tables.contains("score", Qt::CaseInsensitive))
        return QSqlError();

    QSqlQuery q;
    if (!q.exec(QLatin1String("create table circles(id integer primary key, color integer)")))
        return q.lastError();
    if (!q.exec(QLatin1String("create table score(id integer primary key, score integer)")))
        return q.lastError();

    return QSqlError();
}

QModelIndex ItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column, (Tile*)&m_tiles[row]);
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 81;
}

int ItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 0;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Tile *tile = static_cast<Tile*>(index.internalPointer());
    switch (role) {
    case ColorRole:
        return tile->color;
    }
    return QVariant();
}

QModelIndex ItemModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

QHash<int, QByteArray> ItemModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[ColorRole] = "color";
    return roles;
}

void ItemModel::setupColors()
{
    for(int i = 0; i < 3;i++){
        int color = QRandomGenerator::global()->bounded(4) + 1;
        int pos = getRandomEmptyPos();
        if(pos == -1) {
            setFinished(true);
            return;
        }
        setData(index(pos, 0, QModelIndex()), color, ColorRole);
        findLines(pos);
    }
}

int ItemModel::getRandomEmptyPos()
{
    std::vector<Tile*> noColor;
    for (int i = 0; i < 81; ++i) {
        if(m_tiles[i].color == Tile::NO_COLOR){
            noColor.push_back(&m_tiles[i]);
        }
    }
    int len = noColor.size();
    if(len == 0)
        return -1;

    int index = QRandomGenerator::global()->bounded(len);
    return noColor[index]->pos;
}

bool ItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Tile *tile = static_cast<Tile*>(index.internalPointer());
    switch (role) {
    case ColorRole:
        tile->color = Tile::Color(value.toUInt());
        break;
    }

    QSqlQuery q;
    q.prepare("INSERT or REPLACE into circles (id, color) VALUES (:id, :color)");
    q.bindValue(":id", index.row());
    q.bindValue(":color", value.toInt());
    q.exec();

    emit dataChanged(index, index, QVector<int>() << role);
    return true;
}

void ItemModel::swap(int first, int second)
{
    setData(index(second, 0, QModelIndex()), m_tiles[first].color, ColorRole);
    setData(index(first, 0, QModelIndex()), Tile::NO_COLOR, ColorRole);
}

bool ItemModel::findLines(int i)
{
    QList<QVariant> v1 = findHLines(i);
    if(v1.size() < 5)v1.clear();
    else setScore(score() + 10);

    QList<QVariant> v2 = findVLines(i);
    if(v2.size() < 5) v2.clear();
    else setScore(score() + 10);

    for(int i = 0; i < v2.size(); i++) {
        if(!v1.contains(v2[i])){
            v1.append(v2[i]);
        }
    }

    setToDelete(v1);
    return v1.size() > 0;
}

void ItemModel::clearColor(int pos)
{
    setData(index(pos, 0, QModelIndex()), Tile::NO_COLOR, ColorRole);
    m_toDelete.removeAt(m_toDelete.indexOf(pos));
    if(m_toDelete.size() == 0){
        setupColors();
    }
    emit toDeleteChanged();
}

void ItemModel::restart()
{
    QSqlQuery q;
    q.prepare("insert or replace into circles values (?, ?)");

    QVariantList colors;
    QVariantList ids;

    for(int i = 0; i < 81; i++) {
        ids << i;
        colors << 0;
        m_tiles[i].color = Tile::NO_COLOR;
    }

    q.addBindValue(ids);
    q.addBindValue(colors);
    if (!q.execBatch())
        qDebug() << q.lastError();

    dataChanged(index(0,0,QModelIndex()),index(80,0,QModelIndex()),{0});

    setScore(0);
    setToDelete(QVariantList());
    setFinished(false);
    setupColors();
}

QList<QVariant> ItemModel::findHLines(int index)
{
    int j = index / 9;
    Tile::Color color = m_tiles[index].color;
    QList<QVariant> hLine;
    for(int i = index; i >= j * 9; i--) {
        if(m_tiles[i].color == color){
            hLine.push_back(i);
        } else {
            break;
        }
    }
    for(int i = index; i < (j + 1) * 9; i++) {
        if(m_tiles[i].color == color ){
            if(!hLine.contains(i)){
                hLine.push_back(i);
            }
        } else {
            break;
        }
    }
    return hLine;
}

QList<QVariant> ItemModel::findVLines(int index)
{
    Tile::Color color = m_tiles[index].color;
    QList<QVariant> vLine;
    for(int i = index; i >= 0; i-=9) {
        if(m_tiles[i].color == color){
            vLine.push_back(i);
        } else {
            break;
        }
    }
    for(int i = index; i < 81; i+=9) {
        if(m_tiles[i].color == color){
            if(!vLine.contains(i)){
                vLine.push_back(i);
            }
        } else {
            break;
        }
    }
    return vLine;
}

int ItemModel::score() const
{
    return m_score;
}

void ItemModel::setScore(int newScore)
{
    m_score = newScore;
    QSqlQuery query;
    query.prepare("INSERT or REPLACE into score (id, score) VALUES (?, ?)");
    query.addBindValue(0);
    query.addBindValue(newScore);
    query.exec();
    emit scoreChanged();
}

const QList<QVariant> &ItemModel::toDelete() const
{
    return m_toDelete;
}

void ItemModel::setToDelete(const QList<QVariant> &newToDelete)
{
    if (m_toDelete == newToDelete)
        return;
    m_toDelete = newToDelete;
    emit toDeleteChanged();
}

bool ItemModel::finished() const
{
    return m_finished;
}

void ItemModel::setFinished(bool newFinished)
{
    if (m_finished == newFinished)
        return;
    m_finished = newFinished;
    emit finishedChanged();
}

const QString &ItemModel::state() const
{
    return m_state;
}

void ItemModel::setState(const QString &newState)
{
    if (m_state == newState)
        return;
    m_state = newState;
    emit stateChanged();
}
