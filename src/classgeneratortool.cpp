//Usage: classgeneratortool myclass.defs myclass.h myclass.cpp MyClass
/*
    myclass.defs example:
    MyBaseClass
    #include <QUrl>
    QUrl testUrl
    QString testString
*/ 

#include <iostream>

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

int main(int argc, const char **argv)
{
    QString inFileName = argv[1];
    QString hFileName = argv[2];
    QString cppFileName = argv[3];
    QString className = argv[4];

    if (argc != 5){
        std::cout << "class generator tool: Usage: classgeneratortool myclass.defs myclass.h myclass.cpp MyClass";
        return EXIT_FAILURE;
    }

    QString baseClassName;

    QStringList macros;
    QStringList types;
    QStringList names;
    QStringList titles;
    QStringList getters;
    QStringList setters;
    QStringList properties;

    QFile defsFile(inFileName);
    defsFile.open(QIODevice::ReadOnly);
    if (!defsFile.isOpen()){
        std::cout << "class generator tool: Error: Cannot open definitions.";
        return EXIT_FAILURE;
    }
    QTextStream defsS(&defsFile);

    baseClassName = defsS.readLine();
    if (baseClassName.isEmpty()){
        std::cout << "class generator tool: Error: Empty file!\n";
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
        if (tmp.size() < 2){
            std::cout << "class generator tool: Error: Syntax error";
            return EXIT_FAILURE;
        }
        QChar first = tmp.at(1).at(0).toUpper();
        QString titleCase = tmp.at(1);
        titleCase[0] = first;

        types << tmp.at(0);
        names << tmp.at(1);
        titles << titleCase;
        getters << QString("%1() const").arg(cLine);
        setters << QString("void set%2(%1 n%2) const").arg(tmp.at(0), titleCase);
    
        properties << QString("    Q_PROPERTY(%1 %2 READ %2 WRITE set%3)").arg(tmp.at(0), tmp.at(1), titleCase);
    }

    QFile hFile(hFileName);
    hFile.open(QIODevice::WriteOnly);
    if (!hFile.isOpen()){
        std::cout << "class generator tool: Error: Cannot write .h file";
        return EXIT_FAILURE;
    }
    QTextStream hStream(&hFile);

    QString headerDefine = hFileName;
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
    hStream << properties.join("\n");
    hStream << "\n";
    hStream << "\n";
    hStream << "    public:\n";
    hStream << QString("        %1(QObject *parent = NULL);\n").arg(className);
    hStream << QString("        virtual ~%1();\n").arg(className);
    hStream << "\n        ";
    hStream << getters.join(";\n        ");
    hStream << ";\n\n        ";
    hStream << setters.join(";\n        ");
    hStream << ";\n\n";
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
        std::cout << "class generator tool: Error: Cannot write .cpp file";
        return EXIT_FAILURE;
    }
    QTextStream cppStream(&cppFile);

    cppStream << QString("#include \"%1\"\n").arg(hFileName);
    cppStream << "\n";

    cppStream << QString("class %1::Private {\n").arg(className);
    cppStream << "    public:\n";
    for (int i = 0; i < names.size(); i++) {
         cppStream << QString("        %1 %2;\n").arg(types.at(i), names.at(i));
    }
    cppStream << "};\n";
    cppStream << "\n";


    cppStream << QString("%1::%1(QObject *parent)\n").arg(className);
    cppStream << QString("    : %1(parent), d(new Private)\n").arg(baseClassName);
    cppStream << "{\n";
    cppStream << "}\n\n";

    cppStream << QString("%1::~%1()\n").arg(className);
    cppStream << "{\n";
    cppStream << "    delete d;\n";
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
         cppStream << QString("    d->%1 = n%2;\n").arg(names.at(i), titles.at(i));
         cppStream << "}\n\n";
    }

    cppStream << QString("#include \"moc_%1\"\n").arg(cppFileName);

    cppStream.flush();
    cppFile.close(); 

    return EXIT_SUCCESS;
}

