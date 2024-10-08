#include "XMLTool.h"
#include <QDomDocument>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>

int xml::addNode(QString xmlPath,QString strTtags,QString tagValue)
{
    //QFile file(xmlPath); //相对路径、绝对路径、资源路径都行
    //if (!file.open(QIODevice::ReadOnly))
    //{
    //	return 1;
    //}
    //QDomDocument doc;
    //if (!doc.setContent(&file))
    //{
    //	file.close();
    //	return 1;
    //}
    //file.close();

    QDomDocument doc = open(xmlPath);
    QDomElement root = doc.documentElement(); //返回根节点
    QStringList tags = strTtags.split("/");
    for (int i = 0; i<tags.size(); ++i)
    {
        QDomElement node = doc.createElement(tags.at(i));//添加元素

        if (i == tags.size() - 1)
        {
            QDomText qDomText = doc.createTextNode(tagValue);//添加value
            node.appendChild(qDomText);
        }
        root.appendChild(node);
        root = node;
    }
    close(xmlPath, doc);
    //if (!file.open(QFile::WriteOnly | QFile::Truncate))
    //{
    //	return 1;
    //}
    //QTextStream out(&file);
    //doc.save(out, 4); //缩进4格
    //file.close();
    return 0;
}

int xml::appendNode(QString xmlPath, QString strTtags, QString tagValue, QMap<QString, QString> attrMap)
{
    //QFile file(xmlPath);
    //if (!file.open(QIODevice::ReadOnly))
    //{
    //	return 1;
    //}
    //QDomDocument doc;
    //if (!doc.setContent(&file))
    //{
    //	file.close();
    //	return 1;
    //}
    //file.close();
    QDomDocument doc = open(xmlPath);
    QDomElement root = doc.documentElement(); //返回根节点
    QStringList tags = strTtags.split("/");
    int tagsSize = tags.size();
    QDomText qDomText = doc.createTextNode(tagValue);//添加value
    for (int i = 0; i<tagsSize; ++i)
    {
        QString tageName = tags.at(i);
        QDomNodeList nodes = root.elementsByTagName(tageName);//查找节点
        int nodeSize = nodes.size();
        if (i == tagsSize - 1)//是最后一个节点
        {
            //直接添加
            QDomElement node = doc.createElement(tags.at(i));//添加元素
            if (false == tagValue.trimmed().isEmpty())
            {
                node.appendChild(qDomText);//添加
            }
            if (attrMap.size() > 0)
            {
                QMap<QString, QString>::const_iterator iterator = attrMap.constBegin();
                while (iterator != attrMap.constEnd())
                {
                    node.setAttribute(iterator.key(), iterator.value());
                    ++iterator;
                }
            }
            root.appendChild(node);
        }
        else//不是最后一个节点
        {
            if (nodes.size() >0) //找到了中间节点 赋值给root
            {
                root = nodes.at(0).toElement();//查找节点并赋值当前节点给root

            }
            else {//没找到中间节点 创建后赋值给root 继续查找

                QDomElement node = doc.createElement(tags.at(i));
                root.appendChild(node);
                root = node;
            }
        }
    }

    //if (!file.open(QFile::WriteOnly | QFile::Truncate))
    //{
    //	return 1;
    //}
    //QTextStream out(&file);
    //doc.save(out, 4); //缩进4格
    //file.close();

    close(xmlPath, doc);
    return 0;
}

QDomDocument xml::open(QString xmlPath,bool isRemove)
{
    QFile file(xmlPath);  //打开或创建文件
    QDomDocument doc;
    if (true==isRemove &&true == file.exists())
    {
        file.remove();//如果存在则删除
    }
    if (!file.exists() || !file.open(QFile::ReadOnly) || !doc.setContent(&file))
    {
        QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);
        QDomElement root = doc.createElement("Root");
        doc.appendChild(root);
    }
    file.close();
    return doc;
}

int xml::close(QString xmlPath, QDomDocument doc)
{
    QFile file(xmlPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        return 1;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    doc.save(out, 4);
    file.close();
    return 0;
}

int xml::Append(const QString& xmlPath,QString strTtags,const QVariant node, QMap<QString, QVariant> where,bool isRemove)
{
    QDomDocument doc = open(xmlPath, isRemove);

    QDomElement root = doc.documentElement(); //返回根节点
    QStringList tags = strTtags.split("/");
    for (int i = 0; i<tags.size(); ++i)
    {
        QString name = tags.at(i).trimmed();
        QDomNodeList nodes = root.elementsByTagName(name);//查找上级节点 A/B/C
        bool isFindNode = false;
        if(nodes.size() >0)
        {
            if(i == tags.size() - 1)
            {
                //最后一级节点
                for(auto index = 0; index < nodes.size(); ++index)
                {
                    //遍历每一个节点
                    QDomElement e = nodes.at(index).toElement();
                    auto attrs = e.attributes();
                    bool isFindKey = true;

                    for (auto it = where.constBegin(); it != where.constEnd(); ++it)
                    {
                        auto key = it.key().trimmed();
                        auto val = it.value().toString().trimmed();
                        if(false == attrs.contains(key) || val != attrs.namedItem(key).toAttr().value())
                        {
                            isFindKey &= false;
                        }//end if

                    }//end for
                    if(true == isFindKey)
                    {
                        //找到节点
                        root = e;
                        isFindNode = true;
                        break;
                    }//end if
                }//end for

            }else {
                root = nodes.at(0).toElement();
                isFindNode = true;
            }//end else
        }

        if(false == isFindNode)
        {
            //没找到
            QDomElement e = doc.createElement(name);//添加节点
            if(i == tags.size() - 1)
            {
                for (auto it = where.constBegin();i == tags.size() - 1 && it != where.constEnd(); ++it)
                {
                    e.setAttribute(it.key().trimmed(), it.value().toString().trimmed());
                }
            }
            root.appendChild(e);
            root = e;

        }
    }

    AddElement(doc,root,node);
    close(xmlPath, doc);
    return 0;
}

int xml::Append(const QString& xmlPath,QString strTtags,const QVariant v)
{
    QDomDocument doc = open(xmlPath);

    QDomElement root = doc.documentElement(); //返回根节点
    QStringList tags = strTtags.split("/");
    for (int i = 0; i<tags.size(); ++i)
    {
        QDomElement element;
        QString tagName = tags.at(i);
        QDomNodeList nodes = root.elementsByTagName(tagName);//查找节点
        if(nodes.size() == 0)
        {
            element = doc.createElement(tagName);//添加节点
            root.appendChild(element);
            root = element;
            //            if(i == tags.size() - 1){ }
        }
        else
        {
            root = nodes.at(0).toElement();
        }
    }

    AddElement(doc,root,v);
    close(xmlPath, doc);
    return 0;
}

int xml::AddElement(QDomDocument& doc,QDomElement& root,const QVariant v )
{
    if (v.canConvert<QVariantMap>())
    {
        auto map = v.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it)
        {
            auto key = it.key();
            auto val = it.value();
            if(key.startsWith("@"))
            {
                root.setAttribute(key.replace("@",""), val.toString());
            }
            else
            {
                if (val.canConvert<QVariantMap>())
                {
                    auto e = doc.createElement(key);//添加节点
                    AddElement(doc,e,val.toMap());
                    root.appendChild(e);
                }
                else if (val.canConvert<QVariantList>())
                {
                    auto list = val.toList();
                    for(auto i=0;i<list.size();++i)
                    {

                        auto e = doc.createElement(key);//添加节点
                        AddElement(doc,e,list.at(i).toMap());
                        root.appendChild(e);
                    }

                }

            }

        }
    }
    //    else if (v.canConvert<QVariantList>())
    //    {
    //        auto list = v.toList();
    //        for(auto i=0;i<list.size();++i)
    //        {

    //            auto e = doc.createElement(key);//添加节点
    //            AddElement(doc,e,list.at(i).toMap());
    //            root.appendChild(e);
    //        }
    //    }

    return EXIT_SUCCESS;
}


/**
 * 修改节内容
 * @brief xml::updateXml
 * @param path xml路径
 * @param tagsName 节点
 * @param attr 对应的key 和 Vale
 * @param where 条件
 * @return
 */
int xml::updateXml(QString path, QString tagsName, QMap<QString, QString> attr,QMap<QString, QString> where)
{
    if (tagsName.isEmpty())
    {
        return 1;
    }
    bool changed = true;
    QDomDocument doc = open(path);
    QDomElement root = doc.documentElement(); //返回根节点
    QStringList tags = tagsName.split("/");
    for (int i = 0; i<tags.size(); ++i)
    {
        QString tagName = tags.at(i).trimmed();
        QDomNodeList nodes = root.elementsByTagName(tagName);//查找节点
        if (nodes.size() > 0)
        {
            if (i == tags.size() - 1)
            {
                bool isFind = true;
                QDomElement e;
                for (int j = 0; j < nodes.size(); ++j)
                {
                    e = nodes.at(j).toElement();

                    isFind = true;
                    for(auto it = where.begin(); it != where.end(); ++it)
                    {
                        auto key = it.key().trimmed();
                        auto val = it.value().trimmed();
                        if(val != e.attribute(key))
                        {
                            isFind = false;
                            break;
                        }//end if
                    }//end for

                    if(true == isFind)
                    {
                        break;
                    }
                }//end for
                if(true == isFind)
                {
                    //找到
                    for(auto it = attr.begin(); it != attr.end(); ++it)
                    {
                        auto tagKey = it.key().trimmed();
                        auto tagVal = it.value().trimmed();
                        if (tagVal != e.attribute(tagKey,QString()))
                        {
                            e.setAttribute(tagKey, tagVal);
                        }
                    }//end for
                }//end if
                else
                {
                    //没找到
                    QDomElement e = doc.createElement(tagName);//添加节点
                    //最后一个
                    for(auto it = attr.constBegin(); it != attr.constEnd();++it)
                    {
                        auto tagKey = it.key().trimmed();
                        auto tagVal = it.value().trimmed();
                        e.setAttribute(tagKey, tagVal);
                    }
                    root.appendChild(e);
                    root = e;
                }//end else

            }
            else
            {
                root = nodes.at(0).toElement();
            }
        }
        else
        {
            //没找到
            QDomElement e = doc.createElement(tagName);//添加节点
            if(i == tags.size() - 1)
            {
                //最后一个
                for(auto it = attr.constBegin(); it != attr.constEnd();++it)
                {
                    auto tagKey = it.key().trimmed();
                    auto tagVal = it.value().trimmed();
                    e.setAttribute(tagKey, tagVal);
                }
                changed = true;
            }
            root.appendChild(e);
            root = e;
        }
    }
    if (changed)
    {
        close(path, doc);
    }

    return 0;
}


int xml::Delete(QString xmlPath, QString strTtags, QMap<QString, QVariant> where)
{
    QList<QMap<QString, QString>> values;
    QDomDocument doc = open(xmlPath);
    QDomElement root = doc.documentElement(); //返回根节点

    QStringList tags = strTtags.split("/");
    int tageSize = tags.size();
    bool isChange = false;
    for (int i = 0; i < tageSize; ++i)
    {
        QString tageName = tags.at(i);
        QDomNodeList nodes = root.elementsByTagName(tageName);//查找节点
        if(nodes.size() == 0)
        {
            return 0;
        }
        if (i == tags.size() - 1)//最后一个节点
        {
            for (int j = 0; j < nodes.size(); ++j)
            {
                QMap<QString, QString> map;
                QDomElement e = nodes.at(j).toElement();
                auto attributes = e.attributes();
                bool isDelete = true;
                for(auto it = where.begin(); it != where.end(); ++it)
                {
                    auto key = it.key().trimmed();
                    auto val = it.value().toString().trimmed();
                    if(val != e.attribute(key))
                    {
                        isDelete &= false;
                    }
                }
                if(true == isDelete)
                {
                    --j;//索引减一
                    auto parent = e.parentNode();
                    parent.removeChild(e);
//                    if (parent.childNodes().isEmpty())
//                    {
//                        parent.parentNode().removeChild(parent);
//                    }
                    isChange = true;
                }
            }//end for
        }
        else
        {
            root = nodes.at(0).toElement();
        }//end if

    }//end for
    if(true == isChange)
    {
        close(xmlPath, doc);
    }

    return 0;
}

QVariant xml::Read(QString path, QString strTtags,QMap<QString, QVariant> where)
{
    QVariantList variant;
    QDomDocument doc = open(path);
    QDomElement root = doc.documentElement(); //返回根节点

    QStringList tags = strTtags.split("/");
    int tageSize = tags.size();
    for (int i = 0; i < tageSize; ++i)
    {
        QString tageName = tags.at(i);
        QDomNodeList nodes = root.elementsByTagName(tageName);//查找节点
        if(nodes.size() == 0)
        {
            return 0;
        }
        if (i != tags.size() - 1)
        {
            root = nodes.at(0).toElement();
            continue;
        }

        return ReadItem(nodes);

    }//end for

    return QVariant();
}

QVariant xml::ReadItem(const QDomNodeList& nodeList)
{
    QVariantList list;
    for(auto i=0;i<nodeList.size(); ++i)
    {
        auto node = nodeList.at(i);
        if(true == node.isElement())
        {
            auto e = node.toElement();
            auto key = e.tagName();
            auto attributes = e.attributes();
            QMap<QString, QVariant> tagMap;
            for(auto i=0;i<attributes.size();++i)
            {
                auto attr = attributes.item(i).toAttr();
                auto arrrKey = QString("@").append(attr.name());
                tagMap[arrrKey] =attr.value();
            }

            tagMap[key] = ReadItem(e.childNodes());
            list.push_back(tagMap);
        }

        if(true == node.isText())
        {
            auto text = node.toText();

        }
    }
    return list;
}
