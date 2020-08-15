#include "searchwidget.h"
#include "pinyin.h"

#include <QDebug>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QXmlStreamReader>
#include <QCompleter>
#include <QRegularExpression>

class ukCompleter : public QCompleter
{
public:
    ukCompleter(QAbstractItemModel *model, QObject *parent = nullptr)
        : QCompleter(model, parent)
    {
    }

public:
    bool eventFilter(QObject *o, QEvent *e) override;
};

SearchWidget::SearchWidget(QWidget *parent)
    : QLineEdit(parent)
    , m_xmlExplain("")
    , m_bIsChinese(false)
    , m_searchValue("")
    , m_bIstextEdited(false)
    , m_speechState(false)
{
    m_model = new QStandardItemModel(this);
    m_completer = new ukCompleter(m_model, this);
    m_completer->setFilterMode(Qt::MatchContains);//设置QCompleter支持匹配字符搜索
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);//这个属性可设置进行匹配时的大小写敏感性
    m_completer->setCompletionRole(Qt::UserRole); //设置ItemDataRole
    this->setCompleter(m_completer);
    m_completer->setWrapAround(false);
    m_completer->installEventFilter(this);

    connect(this, &QLineEdit::textEdited, this, [ = ] {
        //m_bIstextEdited，　true : 用户输入　，　false : 直接调用setText
        //text(). ""　：　表示使用清除按钮删除数据，发送的信号；　非空　：　表示用户输入数据发送的信号
        if (text() != "") {
            m_bIstextEdited = true;
        } else {
            m_bIstextEdited = false;
        }
    });

    connect(this, &QLineEdit::textChanged, this, [ = ] {
        QString retValue = text();
        if (false == m_speechState) {
            //用户输入的时候，还是按直接设置setText流程运行(旧的流程)
            //外部调用setText的时候，需要先对setText的内容进行解析，解析获取对应的存在数据
            if (m_bIstextEdited) {
                m_bIstextEdited = false;
                //解决无法在已经输入数据前面输入数据,但是目前不清楚外部调用会出现什么问题,暂时注释代码
    //            this->setText(transPinyinToChinese(retValue));
                return ;
            }

            //避免输入单个字符，直接匹配到第一个完整字符(导致不能匹配正确的字符)
            if ("" == retValue || m_searchValue.contains(retValue, Qt::CaseInsensitive)) {
                m_searchValue = retValue;
                return ;
            }

            retValue = transPinyinToChinese(text());

            //拼音转化没找到，再搜索字符包含关联字符
            if (retValue == text()) {
                retValue = containTxtData(retValue);
            }

            m_searchValue = retValue;

            //发送该信号，用于解决外部直接setText的时候，搜索的图标不消失的问题
//            Q_EMIT focusChanged(true);
            this->setText(retValue);
        }
    });

    connect(this, &QLineEdit::returnPressed, this, [ = ] {

        if (!text().isEmpty()) {
            //enter defalt set first
            if (!jumpContentPathWidget(text())) {
                const QString &currentCompletion = this->completer()->currentCompletion();
                qDebug() << Q_FUNC_INFO << " [SearchWidget] currentCompletion : " << currentCompletion;

                //中文遍历一遍,若没有匹配再遍历将拼音转化为中文再遍历
                //解决输入拼音时,有配置数据后,直接回车无法进入第一个匹配数据页面的问题
                if (!jumpContentPathWidget(currentCompletion)) {
                    jumpContentPathWidget(transPinyinToChinese(currentCompletion));
                }
            }
        }
    });

    //鼠标点击后直接页面跳转(存在同名信号)
    connect(m_completer, SIGNAL(activated(QString)), this, SLOT(onCompleterActivated(QString)));
}

SearchWidget::~SearchWidget()
{

}

bool SearchWidget::jumpContentPathWidget(QString path)
{
    qDebug() << Q_FUNC_INFO << path;
    bool bResult = false;

    if (m_EnterNewPagelist.count() > 0) {
        SearchBoxStruct data = getModuleBtnString(path);
        if (data.translateContent != "" && data.fullPagePath != "") {
            for (int i = 0; i < m_EnterNewPagelist.count(); i++) {
                if (m_EnterNewPagelist[i].translateContent == data.fullPagePath) {//getModuleBtnString解析SearchBoxStruct.fullPagePath，满足此处判断
#if DEBUG_XML_SWITCH
                    qDebug() << " [SearchWidget] m_EnterNewPagelist[i].translateContent : " << m_EnterNewPagelist[i].translateContent << " , fullPagePath : " << m_EnterNewPagelist[i].fullPagePath << " , actualModuleName: " << m_EnterNewPagelist[i].actualModuleName;
                    qDebug() << " [SearchWidget] data.translateContent : " << data.translateContent << " , data.fullPagePath : " << data.fullPagePath << " , data.actualModuleName: " << data.actualModuleName;
#endif
                    //the data.actualModuleName had translate to All lowercase
                    qDebug() <<" actulaModuleName is:" << data.translateContent << " " << m_EnterNewPagelist[i].fullPagePath.section('/', 2, -1) << endl;
                    Q_EMIT notifyModuleSearch(data.translateContent);//fullPagePath need delete moduleName
                    bResult = true;
                    break;
                }
            }
        } else {
            qWarning() << "[SearchWidget] translateContent : " << data.translateContent << " , fullPagePath : " << data.fullPagePath;
        }
    } else {
        qWarning() << " [SearchWidget] QList is nullptr.";
    }

    return bResult;
}

void SearchWidget::loadxml()
{
#if DEBUG_XML_SWITCH
    qDebug() << " [SearchWidget] " << Q_FUNC_INFO;
#endif
    if (!m_EnterNewPagelist.isEmpty()) {
        m_EnterNewPagelist.clear();
    }

    if (!m_inputList.isEmpty()) {
        m_inputList.clear();
    }

    if (m_model->rowCount() > 0) {
        QStandardItem *item = nullptr;
        for (int i = 0; i < m_model->rowCount(); i++) {
            item = m_model->takeItem(i);
            delete item;
            item = nullptr;
        }
        m_model->clear();
    }

    //添加一项空数据，为了防止使用setText输入错误数据时直接跳转到list中正确的第一个页面
    m_searchBoxStruct.fullPagePath = "";
    m_searchBoxStruct.actualModuleName = "";
    m_searchBoxStruct.translateContent = "";
    m_searchBoxStruct.childPageName = "";
    m_EnterNewPagelist.append(m_searchBoxStruct);
    m_inputList.append(SearchDataStruct());
    m_model->appendRow(new QStandardItem(""));

    auto isChineseFunc = [](const QString &str)->bool {
        QRegularExpression rex_expression(R"(^[^a-zA-Z]+$)");
        return rex_expression.match(str).hasMatch();
    };


    for (const QString i : m_xmlFilePath) {

        QString xmlPath = i.arg(m_lang);
        QFile file(xmlPath);

        if (!file.exists()) {
            qWarning() << " [SearchWidget] File not exist";
            continue;
        }

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << " [SearchWidget] File open failed";
            continue;
        }

        QXmlStreamReader xmlRead(&file);
        QStringRef dataName;
        QXmlStreamReader::TokenType type = QXmlStreamReader::Invalid;
        /*
            <message>
                <source>Update Setting</source>
                <translation>更新设置</translation>
                <extra-contents_path>/update/Update Setting</extra-contents_path>
        </message>

        +::StartElement:  "message"

            +::StartElement:  "source"
                xmlRead.text :  "Update Setting"
            -::EndElement:  "source"

            +::StartElement:  "translation"
                xmlRead.text :  "更新设置"
            -::EndElement:  "translation"

            +::StartElement:  "extra-contents_path"
                xmlRead.text :  "/update/Update Setting"
            -::EndElement:  "extra-contents_path"

        -::EndElement:  "message"

        //以上 <>  </> 一一对应,
        //在xml的 <> 时进入 StartElement ,
        //在xml 显示中间内容时进入 Characters,
        //在xml的 </>时进入EndElement

        */
        //遍历XML文件,读取每一行的xml数据都会
        //先进入StartElement读取出<>中的内容;
        //再进入Characters读取出中间数据部分;
        //最后进入时进入EndElement读取出</>中的内容
        while (!xmlRead.atEnd()) {
            type = xmlRead.readNext();

            switch (type) {
            case QXmlStreamReader::StartElement:
#if DEBUG_XML_SWITCH
                qDebug() << " [SearchWidget] +::StartElement: " << xmlRead.name() << xmlRead.text();
#endif
                m_xmlExplain = xmlRead.name().toString();
                break;
            case QXmlStreamReader::Characters:
                if (!xmlRead.isWhitespace()) {
#if DEBUG_XML_SWITCH
                    qDebug() << " [SearchWidget]  xmlRead.text : " << xmlRead.text().toString();
#endif
                    if (m_xmlExplain == XML_Source) { // get xml source date

                        m_searchBoxStruct.translateContent = xmlRead.text().toString();

                    } else if (m_xmlExplain == XML_Title) {
                        if (xmlRead.text().toString() != "") // translation not nullptr can set it
                            m_searchBoxStruct.translateContent = xmlRead.text().toString();
#if DEBUG_XML_SWITCH
                        qDebug() << " [SearchWidget] m_searchBoxStruct.translateContent : "
                                 << m_searchBoxStruct.translateContent;
#endif
                    } else if (m_xmlExplain == XML_Numerusform) {
                        if (xmlRead.text().toString() != "") // translation not nullptr can set it
                            m_searchBoxStruct.translateContent = xmlRead.text().toString();
                    } else if (m_xmlExplain == XML_Explain_Path) {
                        m_searchBoxStruct.fullPagePath = xmlRead.text().toString();
                        // follow path module name to get actual module name  ->  Left module dispaly can support
                        // mulLanguages
                        m_searchBoxStruct.actualModuleName =
                                getModulesName(m_searchBoxStruct.fullPagePath.section('/', 1, 1));

                        if (!isChineseFunc(m_searchBoxStruct.translateContent)) {
                            if (!m_TxtList.contains(m_searchBoxStruct.translateContent)) {
                                m_TxtList.append(m_searchBoxStruct.translateContent);
                            }
                        }

                        m_EnterNewPagelist.append(m_searchBoxStruct);

                        // Add search result content
                        if (!m_bIsChinese) {
//                            auto icon = m_iconMap.find(m_searchBoxStruct.fullPagePath.section('/', 1, 1));
//                            if (icon == m_iconMap.end()) {
//                                continue;
//                            }

                            if ("" == m_searchBoxStruct.childPageName) {
                                m_model->appendRow(new QStandardItem(
                                        QString("%1 --> %2")
                                                .arg(m_searchBoxStruct.actualModuleName)
                                                .arg(m_searchBoxStruct.translateContent)));
                            } else {
                                m_model->appendRow(new QStandardItem(
                                        QString("%1 --> %2 / %3")
                                                .arg(m_searchBoxStruct.actualModuleName)
                                                .arg(m_searchBoxStruct.childPageName)
                                                .arg(m_searchBoxStruct.translateContent)));
                            }
                        } else {
                            appendChineseData(m_searchBoxStruct);
                        }

                        clearSearchData();
                    } else {
                        // donthing
                    }
                } else {
                    // qDebug() << "  QXmlStreamReader::Characters with whitespaces.";
                }
                break;
            case QXmlStreamReader::EndElement:
#if DEBUG_XML_SWITCH
                qDebug() << " [SearchWidget] -::EndElement: " << xmlRead.name();
#endif
                // if (m_xmlExplain != "") {
                //     m_xmlExplain = "";
                // }
                break;
            default:
                break;
            }
        }

        m_xmlExplain = "";
        clearSearchData();
        qDebug() << " [SearchWidget] m_EnterNewPagelist.count : " << m_EnterNewPagelist.count();

        file.close();
    }
}

//Follow display content to Analysis SearchBoxStruct data
SearchWidget::SearchBoxStruct SearchWidget::getModuleBtnString(QString value)
{
    SearchBoxStruct data;

    data.translateContent = value.section('-', 0, 1).remove('-').trimmed();
    //follow actual module name to get path module name
    data.actualModuleName = getModulesName(data.translateContent, false);
    data.fullPagePath = value.section('>', 1, -1).remove('>').trimmed();

    if (data.fullPagePath.contains('/', Qt::CaseInsensitive)) {
        data.fullPagePath = data.fullPagePath.section('/', 0, 0).remove('/').trimmed();
    }

#if DEBUG_XML_SWITCH
    qDebug() << Q_FUNC_INFO << " [SearchWidget] data.translateContent : " << data.translateContent << "   ,  data.fullPagePath : " << data.fullPagePath;
#endif

    return data;
}

//tranlate the path name to tr("name")
QString SearchWidget::getModulesName(QString name, bool state)
{
    QString strResult = "";

    for (auto it : m_moduleNameList) {
        if (state) { //true : follow first search second (use pathName translate to actual moduleName)
            if (it.first == name) {
                strResult = it.second;
                break;
            }
        } else { //false : follow second search first (use actual moduleName translate to pathName)
            if (it.second == name) {
                strResult = it.first;
                break;
            }
        }
    }

    return strResult;
}

QString SearchWidget::removeDigital(QString input)
{
    if ("" == input)
        return "";

    QString value = "";
    QByteArray ba = input.toLocal8Bit();
    char *data = nullptr;
    data = ba.data();
    while (*data) {
        if (!(*data >= '0' && *data <= '9')) {
            value += *data;
        }
        data++;
    }

    return value;
}

QString SearchWidget::transPinyinToChinese(QString pinyin)
{
    QString value = pinyin;

    //遍历"汉字-拼音"列表,将存在的"拼音"转换为"汉字"
    for (auto data : m_inputList) {
        if (value == data.pinyin) {
            value = data.chiese;
            break;
        }
    }

    return value;
}

QString SearchWidget::containTxtData(QString txt)
{
    QString value = txt;

    //遍历"汉字-拼音"列表,将存在的"拼音"转换为"汉字"
    for (auto data : m_inputList) {
        if (data.chiese.contains(txt, Qt::CaseInsensitive) ||
               data.pinyin.contains(txt, Qt::CaseInsensitive)) {
            value = data.chiese;
            break;
        }
    }

    return value;
}

void SearchWidget::appendChineseData(SearchWidget::SearchBoxStruct data)
{
    if ("" == data.childPageName) {
        //先添加使用appenRow添加Qt::EditRole数据(用于下拉框显示),然后添加Qt::UserRole数据(用于输入框搜索)
        //Qt::EditRole数据用于显示搜索到的结果(汉字)
        //Qt::UserRole数据用于输入框输入的数据(拼音/汉字 均可)
        //即在输入框搜索Qt::UserRole的数据,就会在下拉框显示Qt::EditRole的数据
        m_model->appendRow(new QStandardItem(//icon.value(),
                                             QString("%1 --> %2").arg(data.actualModuleName).arg(data.translateContent)));

        //设置汉字的Qt::UserRole数据
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0),
                         QString("%1 --> %2")
                         .arg(data.actualModuleName)
                         .arg(data.translateContent),
                         Qt::UserRole);

        QString hanziTxt = QString("%1 --> %2").arg(data.actualModuleName).arg(data.translateContent);

        for (auto datas : m_TxtList) {
            for (int i = 0; i < datas.count(); i++) {
                if( data.translateContent == datas){
                    return;
                }
            }
        }

        QString pinyinTxt = QString("%1 --> %2")
                            .arg(removeDigital(Chinese2Pinyin(data.actualModuleName)))
                            .arg(removeDigital(Chinese2Pinyin(data.translateContent)));

        //添加显示的汉字(用于拼音搜索显示)
        m_model->appendRow(new QStandardItem(/*icon.value(),*/ hanziTxt));
        //设置Qt::UserRole搜索的拼音(即搜索拼音会显示上面的汉字)
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0), pinyinTxt, Qt::UserRole);

        SearchDataStruct transdata;
        transdata.chiese = hanziTxt;
        transdata.pinyin = pinyinTxt;
        //存储 汉字和拼音 : 在选择对应的下拉框数据后,会将Qt::UserRole数据设置到输入框(即pinyin)
        //而在输入框发送 DSearchEdit::textChanged 信号时,会遍历m_inputList,根据pinyin获取到对应汉字,再将汉字设置到输入框
        m_inputList.append(transdata);
    } else {
        //先添加使用appenRow添加Qt::EditRole数据(用于下拉框显示),然后添加Qt::UserRole数据(用于输入框搜索)
        //Qt::EditRole数据用于显示搜索到的结果(汉字)
        //Qt::UserRole数据用于输入框输入的数据(拼音/汉字 均可)
        //即在输入框搜索Qt::UserRole的数据,就会在下拉框显示Qt::EditRole的数据
        m_model->appendRow(new QStandardItem(//icon.value(),
                                             QString("%1 --> %2 / %3").arg(data.actualModuleName).arg(data.childPageName).arg(data.translateContent)));

        //设置汉字的Qt::UserRole数据
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0),
                         QString("%1 --> %2 / %3")
                         .arg(data.actualModuleName)
                         .arg(data.childPageName)
                         .arg(data.translateContent),
                         Qt::UserRole);

        QString hanziTxt = QString("%1 --> %2 / %3").arg(data.actualModuleName).arg(data.childPageName).arg(data.translateContent);
        QString pinyinTxt = QString("%1 --> %2 / %3")
                            .arg(removeDigital(Chinese2Pinyin(data.actualModuleName)))
                            .arg(removeDigital(Chinese2Pinyin(data.childPageName)))
                            .arg(removeDigital(Chinese2Pinyin(data.translateContent)));

        //添加显示的汉字(用于拼音搜索显示)
//        auto icon = m_iconMap.find(data.fullPagePath.section('/', 1, 1));
//        if (icon == m_iconMap.end()) {
//            return;
//        }
        m_model->appendRow(new QStandardItem(/*icon.value(),*/ hanziTxt));
        //设置Qt::UserRole搜索的拼音(即搜索拼音会显示上面的汉字)
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0), pinyinTxt, Qt::UserRole);

        SearchDataStruct transdata;
        transdata.chiese = hanziTxt;
        transdata.pinyin = pinyinTxt;
        //存储 汉字和拼音 : 在选择对应的下拉框数据后,会将Qt::UserRole数据设置到输入框(即pinyin)
        //而在输入框发送 DSearchEdit::textChanged 信号时,会遍历m_inputList,根据pinyin获取到对应汉字,再将汉字设置到输入框
        m_inputList.append(transdata);
    }
}

void SearchWidget::clearSearchData()
{
    m_searchBoxStruct.translateContent = "";
    m_searchBoxStruct.actualModuleName = "";
    m_searchBoxStruct.childPageName = "";
    m_searchBoxStruct.fullPagePath = "";
}

void SearchWidget::setLanguage(QString type)
{
    m_lang = type;

    if (type == "zh_CN" || type == "zh_HK" || type == "zh_TW") {
        m_bIsChinese = true;
        m_completer->setCompletionRole(Qt::UserRole); //设置ItemDataRole
    } else {
        m_completer->setCompletionRole(Qt::DisplayRole);
    }

    loadxml();
}

//save all modules moduleInteface name and actual moduleName
//moduleName : moduleInteface name  (used to path module to translate searchName)
//searchName : actual module
void SearchWidget::addModulesName(QString moduleName, QString searchName, QString translation)
{
    QPair<QString, QString> data;
    data.first = moduleName;
    data.second = searchName;
    m_moduleNameList.append(data);

    if (!translation.isEmpty()) {
        m_xmlFilePath.insert(translation);
    }


#if DEBUG_XML_SWITCH
    qDebug() << " [SearchWidget] moduleName : " << moduleName << " , searchName : " << searchName;
#endif
}

void SearchWidget::addUnExsitData(QString module, QString datail)
{
    for (auto value : m_unexsitList) {
        if (value.module == module)
            return;
    }

    UnexsitStruct data;
    data.module = module;
    data.datail = datail;
    m_unexsitList.append(data);

    loadxml();
}

void SearchWidget::onCompleterActivated(QString value)
{
    qDebug() << Q_FUNC_INFO << value;
    Q_EMIT returnPressed();
}

bool ukCompleter::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusOut) {
        return QCompleter::eventFilter(o, e);
    }

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        QModelIndex keyIndex;
        switch (ke->key()) {
        case Qt::Key_Up: {
            if (popup()->currentIndex().row() == 0) {
                keyIndex = popup()->model()->index(popup()->model()->rowCount() - 1, 0);
                popup()->setCurrentIndex(keyIndex);
            } else {
                keyIndex = popup()->model()->index(popup()->currentIndex().row() - 1, 0);
                popup()->setCurrentIndex(keyIndex);
            }
            return true;
        }
        case Qt::Key_Down: {
            if (popup()->currentIndex().row() == popup()->model()->rowCount() - 1) {
                keyIndex = popup()->model()->index(0, 0);
                popup()->setCurrentIndex(keyIndex);
            } else {
                keyIndex = popup()->model()->index(popup()->currentIndex().row() + 1, 0);
                popup()->setCurrentIndex(keyIndex);
            }
            return true;
        }
        case Qt::Key_Enter: {
            if (popup()->isVisible() && !popup()->currentIndex().isValid()) {
                keyIndex = popup()->model()->index(0, 0);
                popup()->setCurrentIndex(keyIndex);
            }
            popup()->hide();
        }
        }
    }
    return QCompleter::eventFilter(o, e);
}
