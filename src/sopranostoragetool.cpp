//Usage: sopranostoragetool myclass.defs myclass.h myclass.cpp MyClass
/*
    myclass.defs example:
    +MyBaseClass
    #include <QUrl>
    #include "mybaseclass.h"
    QUrl testUrl Vocabulary::T::testUrl
    QString testString Vocabulary::T::testString
*/

#include <iostream>

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

int main(int argc, const char **argv)
{
    QString inFileName = argv[1];
    QString hFileName = argv[2];
    QString cppFileName = argv[3];
    QString className = argv[4];

    QString cppRelativeFileName = cppFileName;
    if (cppRelativeFileName.contains('/')) {
        cppRelativeFileName = cppRelativeFileName.split('/').last();
    }

    QString hRelativeFileName = hFileName;
    if (hRelativeFileName.contains('/')) {
        hRelativeFileName = hRelativeFileName.split('/').last();
    }

    if (argc != 5){
        std::cout << "soprano storage tool: Usage: sopranostoragetool myclass.defs myclass.h myclass.cpp MyClass";
        return EXIT_FAILURE;
    }

    bool sObj;
    QString baseClassName;

    QStringList macros;
    QStringList types;
    QStringList names;
    QStringList titles;
    QStringList getters;
    QStringList setters;
    QStringList properties;
    QStringList predicates;

    QHash<QString, QString> typeDefaults;
    typeDefaults.insert("bool", "false");
    typeDefaults.insert("int", "0");
    typeDefaults.insert("qreal", "0");
    typeDefaults.insert("uint", "0");

    QHash<QString, QString> typeConversion;
    typeConversion.insert("bool", ".literal().toBool()");
    typeConversion.insert("int", ".literal().toInt()");
    typeConversion.insert("QDate", ".literal().toDate()");
    typeConversion.insert("QString", ".literal().toString()");
    typeConversion.insert("QUrl", ".uri()");
    typeConversion.insert("qreal", ".literal().toDouble()");
    typeConversion.insert("uint", ".literal().toUnsignedInt()");

    QHash<QString, QString> nullChecker;
    nullChecker.insert("bool", "");
    nullChecker.insert("int", "");
    nullChecker.insert("QUrl", ".isEmpty()");
    nullChecker.insert("qreal", "");
    nullChecker.insert("uint", "");

    QFile defsFile(inFileName);
    defsFile.open(QIODevice::ReadOnly);
    if (!defsFile.isOpen()){
        std::cout << "soprano storage tool: Error: Cannot open definitions.";
        return EXIT_FAILURE;
    }
    QTextStream defsS(&defsFile);

    baseClassName = defsS.readLine();
    sObj = baseClassName.startsWith('+');
    baseClassName.replace('+', "");
    if (baseClassName.isEmpty()){
        std::cout << "soprano storage tool: Error: Empty file!\n";
        return EXIT_FAILURE;
    }

    Q_FOREVER {
        QString cLine = defsS.readLine();
        if (cLine.isEmpty()) break;

        if (cLine.startsWith('#')) {
            macros << cLine;
            continue;
        }

        QStringList tmp = cLine.split(' ');
        if (tmp.size() < 3){
            std::cout << "soprano storage tool: Error: Syntax error";
            return EXIT_FAILURE;
        }
        QChar first = tmp.at(1).at(0).toUpper();
        QString titleCase = tmp.at(1);
        titleCase[0] = first;

        types << tmp.at(0);
        names << tmp.at(1);
        predicates << tmp.at(2);
        titles << titleCase;
        getters << QString("%1 %2() const").arg(tmp.at(0), tmp.at(1));
        setters << QString("void set%2(%1 n%2) const").arg(tmp.at(0), titleCase);

        properties << QString("    Q_PROPERTY(%1 %2 READ %2 WRITE set%3)").arg(tmp.at(0), tmp.at(1), titleCase);
    }

    QFile hFile(hFileName);
    hFile.open(QIODevice::WriteOnly);
    if (!hFile.isOpen()){
        std::cout << "soprano storage tool: Error: Cannot write .h file";
        return EXIT_FAILURE;
    }
    QTextStream hStream(&hFile);

    QString headerDefine = hRelativeFileName;
    headerDefine.replace('.','_');
    headerDefine = headerDefine.toUpper();
    hStream << QString("#ifndef %1_\n").arg(headerDefine);
    hStream << QString("#define %1_\n").arg(headerDefine);
    hStream << "\n";
    hStream << macros.join("\n");
    hStream << "\n\n";
    hStream << QString("class %1 : public %2 {\n").arg(className, baseClassName);
    hStream << "\n";
    hStream << "    Q_OBJECT\n";
    hStream << "\n";
    hStream << QString("    Q_PROPERTY(QUrl objectUri READ objectUri)\n");
    hStream << properties.join("\n");
    hStream << "\n";
    hStream << "\n";
    hStream << "    public:\n";
    hStream << QString("        %1(QUrl nObjectUri, Soprano::Model *model, QObject *parent = NULL);\n").arg(className);
    hStream << QString("        virtual ~%1();\n").arg(className);
    hStream << "\n";
    hStream << "        virtual void load() const;\n";
    hStream << "        virtual void save() const;\n";
    hStream << "        virtual void removeFromStorage() const;\n";
    hStream << "\n";
    hStream << "        Soprano::Model *model() const;\n";
    hStream << "        QUrl objectUri() const;\n";
    hStream << "        ";
    hStream << getters.join(";\n        ");
    hStream << ";\n\n        ";
    hStream << setters.join(";\n        ");
    hStream << ";\n\n";
    hStream << "    protected:\n";
    hStream << "        virtual void prepareObjectRemoval() const;\n";
    hStream << "\n";
    hStream << "    private:\n";
    hStream << "        class Private;\n";
    hStream << "        Private * const d;\n";
    hStream << "};\n";
    hStream << "\n";
    hStream << "#endif\n";

    hStream.flush();
    hFile.close();


    QFile cppFile(cppFileName);
    cppFile.open(QIODevice::WriteOnly);
    if (!cppFile.isOpen()){
        std::cout << "soprano storage tool: Error: Cannot write .cpp file";
        return EXIT_FAILURE;
    }
    QTextStream cppStream(&cppFile);

    cppStream << QString("#include \"%1\"\n").arg(hRelativeFileName);
    cppStream << "\n";
    cppStream << "#include <Soprano/Model>\n";
    cppStream << "#include <Soprano/Statement>\n";
    cppStream << "#include <Soprano/StatementIterator>\n";
    cppStream << "\n";

    cppStream << QString("class %1::Private {\n").arg(className);
    cppStream << "    public:\n";
    cppStream << "        Soprano::Model *model;\n";
    cppStream << "        QUrl objectUri;\n";
    for (int i = 0; i < names.size(); i++) {
         cppStream << QString("        %1 %2;\n").arg(types.at(i), names.at(i));
    }
    cppStream << "\n";
    for (int i = 0; i < names.size(); i++) {
         cppStream << QString("        bool %1DirtyFlag : 1;\n").arg(names.at(i));
    }
    cppStream << "};\n";
    cppStream << "\n";


    cppStream << QString("%1::%1(QUrl nObjectUri, Soprano::Model *model, QObject *parent)\n").arg(className);
    if (sObj){
        cppStream << QString("    : %1(nObjectUri, model, parent), d(new Private)\n").arg(baseClassName);
    }else{
        cppStream << QString("    : %1(parent), d(new Private)\n").arg(baseClassName);
    }
    cppStream << "{\n";
    cppStream << "    d->objectUri = nObjectUri;\n";
    cppStream << "    d->model = model;\n";
    cppStream << "\n";
    for (int i = 0; i < names.size(); i++) {
         if (!typeDefaults.value(types.at(i)).isEmpty()){
             cppStream << QString("    d->%1 = %2;\n").arg(names.at(i), typeDefaults.value(types.at(i)));
         }
    }
    for (int i = 0; i < names.size(); i++) {
         cppStream << QString("    d->%1DirtyFlag = false;\n").arg(names.at(i));
    }
    cppStream << "}\n\n";

    cppStream << QString("%1::~%1()\n").arg(className);
    cppStream << "{\n";
    cppStream << "    delete d;\n";
    cppStream << "}\n\n";

    cppStream << QString("void %1::load() const\n").arg(className);
    cppStream << "{\n";
    if (sObj) cppStream << QString("    %1::load();\n").arg(baseClassName);
    cppStream << "    Soprano::StatementIterator it = d->model->listStatements(d->objectUri, Soprano::Node(), Soprano::Node());\n";
    cppStream << "    QList<Soprano::Statement> allStatements = it.allElements();\n";
    cppStream << "    Q_FOREACH (const Soprano::Statement &s, allStatements){\n";
    for (int i = 0; i < names.size(); i++) {
        cppStream << QString("        %1 (s.predicate() == %2()){\n").arg(((i == 0) ? "if" : "}else if"), predicates.at(i));
        if (!types.at(i).contains(QRegExp("Q*List"))){
            cppStream << QString("            d->%1 = s.object()%2;\n").arg(names.at(i), typeConversion.value(types.at(i)));
        }else{
            cppStream << QString("            d->%1.append(s.object()%2);\n").arg(names.at(i), typeConversion.value(types.at(i).split(QRegExp("[<>]")).at(1)));
        }
    }
    cppStream << "        }\n";
    cppStream << "    }\n";
    cppStream << "}\n\n";

    cppStream << QString("void %1::save() const\n").arg(className);
    cppStream << "{\n";
    if (sObj) cppStream << QString("    %1::save();\n").arg(baseClassName);
    for (int i = 0; i < names.size(); i++){
        QString extraIdent = "";
        QString itemString = "";
        QString nullCheckerString = nullChecker.value(types.at(i), "isNull()");
        bool hasNullChecker = nullCheckerString != "";
        bool listType = false;
        cppStream << QString("    if (d->%1DirtyFlag){\n").arg(names.at(i));
        if (types.at(i).contains(QRegExp("Q*List"))){
            listType = true;
            extraIdent = "    ";
            itemString = ".at(i)";
            cppStream << QString("        d->model->removeAllStatements(d->objectUri, %1(), Soprano::Node());\n").arg(predicates.at(i));
            cppStream << QString("        for (int i = 0; i < d->%1.size(); i++){\n").arg(names.at(i));
        }
        if (types.at(i).contains("QUrl")){
            cppStream << QString("%1        if (!d->%2%3.isEmpty()){\n").arg(extraIdent, names.at(i), itemString);
            cppStream << QString("%1            d->model->addStatement(d->objectUri, %2(), d->%3%4);\n").arg(extraIdent, predicates.at(i), names.at(i), itemString);
        }else{
            if (hasNullChecker) cppStream << QString("%1        if (!d->%2%3.%4){\n").arg(extraIdent, names.at(i), itemString, nullCheckerString);
            cppStream << QString("%1            d->model->addStatement(d->objectUri, %2(), Soprano::LiteralValue(d->%3%4));\n").arg(extraIdent, predicates.at(i), names.at(i), itemString);
        }
        if (listType){
            if (hasNullChecker) cppStream << "            }\n";
            cppStream << "        }\n";
        }else if (hasNullChecker){
            cppStream << "        }else{\n";
            cppStream << QString("            d->model->removeAllStatements(d->objectUri, %1(), Soprano::Node());\n").arg(predicates.at(i));
            cppStream << "        }\n";
        }
        cppStream << "    }\n";
    }
    cppStream << "\n";
    for (int i = 0; i < names.size(); i++) {
         cppStream << QString("    d->%1DirtyFlag = false;\n").arg(names.at(i));
    }
    cppStream << "}\n\n";

    cppStream << QString("void %1::removeFromStorage() const\n").arg(className);
    cppStream << "{\n";
    cppStream << "    prepareObjectRemoval();\n";
    cppStream << "    d->model->removeAllStatements(d->objectUri, Soprano::Node(), Soprano::Node());\n";
    cppStream << "}\n\n";

    cppStream << QString("void %1::prepareObjectRemoval() const\n").arg(className);
    cppStream << "{\n";
    if (sObj) cppStream << QString("    %1::prepareObjectRemoval();\n").arg(baseClassName);
    for (int i = 0; i < names.size(); i++) {
         cppStream << QString("    d->%1DirtyFlag = true;\n").arg(names.at(i));
    }
    cppStream << "}\n\n";

    cppStream << QString("Soprano::Model *%1::model() const\n").arg(className);
    cppStream << "{\n";
    cppStream << "    return d->model;\n";
    cppStream << "}\n\n";

    cppStream << QString("QUrl %1::objectUri() const\n").arg(className);
    cppStream << "{\n";
    cppStream << "    return d->objectUri;\n";
    cppStream << "}\n\n";


    for (int i = 0; i < getters.size(); i++) {
         cppStream << QString("%1 %2::%3() const\n").arg(types.at(i), className, names.at(i));
         cppStream << "{\n";
         cppStream << QString("    return d->%1;\n").arg(names.at(i));
         cppStream << "}\n\n";
    }

    for (int i = 0; i < setters.size(); i++) {
         cppStream << QString("void %1::set%2(%3 n%2) const\n").arg(className, titles.at(i), types.at(i));
         cppStream << "{\n";
         cppStream << QString("    d->%1DirtyFlag = true;\n").arg(names.at(i));
         cppStream << QString("    d->%1 = n%2;\n").arg(names.at(i), titles.at(i));
         cppStream << "}\n\n";
    }

    cppStream.flush();
    cppFile.close();

    return EXIT_SUCCESS;
}

