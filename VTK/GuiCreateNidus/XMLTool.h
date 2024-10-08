#ifndef XMLTOOL_H
#define XMLTOOL_H

#include <QVariant>
#include <QDomDocument>
namespace xml
{
    int addNode(QString xmPath, QString strTags, QString tagValue);

    int appendNode(QString xmPath, QString strTags, QString tagValue = QString(), QMap<QString, QString> attrMap = QMap<QString, QString>());//添加新的节点和属性

	QDomDocument open(QString xmlPath, bool isRemove = false);

	int close(QString xmlPath, QDomDocument doc);

    int Append(const QString& xmlPath,QString strTags,const QVariant v, QMap<QString, QVariant> where,bool isRemove=false);
    int Append(const QString& xmlPath,QString strTags,const QVariant v );

    int AddElement(QDomDocument& doc,QDomElement& root,const QVariant v );

    int updateXml(QString xmPath, QString tags, QMap<QString, QString> attr, QMap<QString, QString> where);

    int Delete(QString path, QString strTags, QMap<QString, QVariant> where);

    QVariant Read(QString path, QString strTags= QString(),QMap<QString, QVariant> where =QMap<QString, QVariant>());

    QVariant ReadItem(const QDomNodeList& nodeList);
}

#endif // XMLTOOL_H
