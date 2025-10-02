#ifndef FILESTABLEMODEL_H
#define FILESTABLEMODEL_H

#include <QWidget>
#include <QAbstractTableModel>
#include "file.h"
#include <libtorrent/download_priority.hpp>
#include "priority.h"
#include "utils.h"
#include <QDir>

class BaseItem {
public:
    BaseItem() = default;
    virtual ~BaseItem() = default;

    virtual bool isDir() const {
        return true;
    }
    virtual void setPath(const QString& newPath) {
    }
    virtual QString getPath() const {
        return {};
    }
    virtual QVariant getValue(int column) const {
        qDebug() << "Default get value";
        return {};
    }
    virtual bool setValue(int column, const QVariant& value) {
        return false;
    }


    void addChild(BaseItem *file);
    QList<BaseItem*> children() const {
        return m_children;
    }
    void removeChild(BaseItem* child) {
        m_children.remove(m_children.indexOf(child));
    }
    int childCount() const {
        return m_children.size();
    }

    void setParent(BaseItem* newParent);
    BaseItem* parent() const {
        return m_parent;
    }


private:
    BaseItem* m_parent{nullptr};
    QList<BaseItem*> m_children;
};

class FileItem : public BaseItem {
    File m_fileData;
public:
    FileItem() = default;
    ~FileItem() override = default;

    bool isDir() const override {
        return false;
    }
    void setPath(const QString& newPath) override {
        m_fileData.filename = newPath;
    }
    QString getPath() const override {
        return m_fileData.filename;
    }

    void setData(const File& data) {
        m_fileData = data;
    }
    File getData() const {
        return m_fileData;
    }

    QVariant getValue(int column) const override;

    bool setValue(int column, const QVariant& value) override;
};

class DirItem : public BaseItem {
private:
    QString m_path;
public:
    DirItem() = default;
    void setPath(const QString& newPath) override {
        m_path = newPath;
    }
    QString getPath() const override {
        return m_path;
    }

    QVariant getValue(int column) const override;
};



class FileTreeModel : public QAbstractItemModel {
    Q_OBJECT

    BaseItem* objByIndex(const QModelIndex& index) const;

    void resetRoot();

    bool isRootVirgin() const {
        return m_rootItem == nullptr || (m_rootItem && m_rootItem->childCount() == 0);
        // return m_rootItem->childCount() == 0;
    }
public:
    explicit FileTreeModel(QObject* parent = nullptr);

    ~FileTreeModel() { delete m_rootItem; }

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override {
        return objByIndex(parent)->childCount();
    }

    int columnCount(const QModelIndex &parent) const override {
        Q_UNUSED(parent);
        return FILE_FIELD_COUNT;
    }

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;



    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clearFiles() {
        beginResetModel();
        resetRoot();
        endResetModel();
    }

    void traverseRecursively(QString curDir, BaseItem* root, const QList<File>& hFiles, const QModelIndex& parentIndex);

    Qt::ItemFlags flags(const QModelIndex &index) const override;


    void setFiles(const QList<File>& files);
signals:
    void statusChanged(int index, bool value);
    void priorityChanged(int index, int priority);
private:
    BaseItem* m_rootItem{nullptr};
};



#endif // FILESTABLEMODEL_H
