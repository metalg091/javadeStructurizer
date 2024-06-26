#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <utility>
#include <set>

using namespace std; // for some reason it started to require std:: but don't remove, it could break the code

// declarations:
vector<string> split(string s, string delimiter);
string protlvl(string line);
string removeQuotes(string line);
string concat(vector<string> v, char delimiter);
ofstream *createFile(string package, string name);
class Wrapper;
class Field;
class Method;
class Constructor;
class File;
class Enum;
class Class;
class Interface;
class Exception;
// these functions must be placed after the classes
Field *createField(fstream *infile, string line, File *parent);
Method *createMethod(fstream *infile, string line, File *parent);
Constructor *createConstructor(fstream *infile, string line, File *parent);
File *getFile(string inpth, Wrapper *wp); // nullptr == fail
string typeMaker(string type, File *parent);
pair<string, string> typeNameSeparator(string line, File *parent);
void importMaker(string type, File *parent);
string getSpecialParam(void *caller, int type, int parenttype, vector<string> paramNames, bool isConstructor);

vector<string> split(string s, string delimiter)
{
    vector<string> result;
    std::size_t pos = 0;
    while ((pos = s.find(delimiter)) != string::npos)
    {
        result.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    result.push_back(s);
    return result;
}

string protlvl(string line)
{
    // search for VISIBLE_TO_ALL VISIBLE_TO_SUBCLASSES VISIBLE_TO_NONE
    if (line.find("VISIBLE_TO_ALL") != string::npos)
    {
        return "public ";
    }
    else if (line.find("VISIBLE_TO_SUBCLASSES") != string::npos)
    {
        return "protected ";
    }
    else if (line.find("VISIBLE_TO_NONE") != string::npos)
    {
        return "private ";
    }
    else
    {
        return "";
    }
}

string removeQuotes(string line)
{
    std::size_t pos = line.find('"');
    if (pos == string::npos)
    {
        return line;
    }
    return removeQuotes(line.substr(0, pos) + line.substr(pos + 1));
}

string concat(vector<string> v, char delimiter)
{
    string result;
    for (std::size_t i = 0; i < v.size(); i++)
    {
        result += v[i];
        if (i != v.size() - 1)
        {
            result += delimiter;
        }
    }
    return result;
}

ofstream *createFile(string package, string name)
{
    string pth;
    if (package != "")
    {
        // replace all . with /
        for (std::size_t i = 0; i < package.size(); i++)
        {
            if (package[i] == '.')
            {
                pth += '/';
            }
            else
            {
                pth += package[i];
            }
        }
        string command = "mkdir -p " + pth;
        system(command.c_str());
        pth += '/';
    }
    pth += name + ".java";
    ofstream *file = new ofstream(pth);
    if (!file)
    {
        std::cout << "File error if ya here, I can't help ya!" << endl;
        return nullptr;
    }
    return file;
}

class Field
{
public:
    string name;
    string type;
    string visibility;
    bool isStatic = false;
    bool isFinal = false;
    bool hasGetter = false;
    bool hasSetter = false;
    Field(){};
    ~Field(){};
    string toString()
    {
        string result = "\t" + visibility;
        if (isStatic)
        {
            result += "static ";
        }
        if (isFinal)
        {
            result += "final " + type + " " + name;
            // final fields need to be initialized
            if (type == "int" || type == "double" || type == "float" || type == "long" || type == "char" || type == "byte" || type == "short")
            {
                result += " = 0; //TODO\n";
            }
            else if (type == "boolean")
            {
                result += " = false; //TODO\n";
            }
            else
            {
                result += " = null; //TODO\n";
            }
        }
        else
        {
            result += type + " " + name + ";\n";
        }
        if (hasGetter)
        {
            result += "\tpublic " + type + " get" + std::string(1, static_cast<char>(toupper(name[0]))) + name.substr(1) + "() {\n";
            result += "\t\treturn " + name + ";\n";
            result += "\t}\n";
        }
        if (!isFinal && hasSetter) // final fields can't have setters
        {
            result += "\tpublic void set";
            result += std::string(1, static_cast<char>(toupper(name[0]))) + name.substr(1) + "(" + type + " " + name + ") {\n";
            result += "\t\t// TODO\n";
            result += "\t\tthis." + name + " = " + name + ";\n";
            result += "\t}\n";
        }
        return result;
    }
};

class Method
{
public:
    string name;
    string visibility;
    string returnType;
    vector<string> paramTypes;
    vector<string> paramNames;
    bool isStatic = false;
    bool simField = false;
    Method(){};
    ~Method(){};
    string toString(bool hasBody, void *caller, int type) // type 0 -> class, type 1 -> exception, type 2 -> interface
    {
        string result = "\t" + visibility;
        if (isStatic)
        {
            result += "static ";
        }
        result += returnType + " " + name + "(";
        if (simField)
        {
            result += getSpecialParam(caller, 0, 0, paramNames, false);
        }
        else
        {
            for (std::size_t i = 0; i < paramTypes.size(); i++)
            {
                result += paramTypes[i] + " " + paramNames[i];
                if (i != paramTypes.size() - 1)
                {
                    result += ", ";
                }
            }
            result += ")";
        }
        if (hasBody)
        {
            result += " {\n";
            result += "\t\t// TODO\n";
            // dummy return here
            if (returnType != "void")
            {
                if (returnType == "int")
                {
                    result += "\t\treturn 0;\n";
                }
                else if (returnType == "double")
                {
                    result += "\t\treturn 0.0;\n";
                }
                else if (returnType == "float")
                {
                    result += "\t\treturn 0.0f;\n";
                }
                else if (returnType == "long")
                {
                    result += "\t\treturn 0L;\n";
                }
                else if (returnType == "char")
                {
                    result += "\t\treturn '0';\n";
                }
                else if (returnType == "byte")
                {
                    result += "\t\treturn (byte)0;\n";
                }
                else if (returnType == "short")
                {
                    result += "\t\treturn (short)0;\n";
                }
                else if (returnType == "boolean")
                {
                    result += "\t\treturn false;\n";
                }
                else
                {
                    result += "\t\treturn null;\n";
                }
            }
            result += "\t}\n";
        }
        else
        {
            result += ";\n";
        }
        return result;
    }
};

class Constructor
{
public:
    string name;
    string visibility;
    vector<string> paramTypes;
    vector<string> paramNames;
    bool simField = false;
    bool simParent = false;
    Constructor(){};
    ~Constructor(){};
    string toString(void *caller, int type) // type 0 -> class, type 1 -> exception
    {
        string result = "\t" + visibility + name + "(";
        if (simField)
        {
            result += getSpecialParam(caller, 0, type, paramNames, true);
        }
        else if (simParent)
        {
            result += getSpecialParam(caller, 1, type, paramNames, true);
        }
        else
        {
            for (std::size_t i = 0; i < paramTypes.size(); i++)
            {
                result += paramTypes[i] + " " + paramNames[i];
                if (i != paramTypes.size() - 1)
                {
                    result += ", ";
                }
            }
            result += ") {\n";
            result += "\t\t// TODO\n";
        }
        result += "\t}\n";
        return result;
    }
};

class File // its abstract
{
public:
    string testPath;
    string name;
    string package;
    string visibility;
    set<string> imports;
    Wrapper *wrapper; // DO NOT DELETE THIS!
    File(string pth, Wrapper *wp)
    {
        this->testPath = pth;
        this->wrapper = wp;
    };
    virtual ~File(){}; // default destructor
    virtual void process()
    {
        cout << "Error - Processing an abstract File\n";
    }
    virtual void toFile()
    {
        cout << "Error - Writing to abstract File\n";
    }
};

class Enum : public File
{
public:
    Enum(string inpth, Wrapper *wp, string iname) : File(inpth, wp)
    {
        package = "";
        size_t pos = iname.find_last_of('.');
        if (pos != string::npos)
        {
            package = iname.substr(0, pos);
            iname = iname.substr(pos + 1);
        }
        this->name = iname;
    }
    ~Enum(){};
    vector<string> elements;
    string toString()
    {
        string result = "package " + package + ";\n\n";
        bool hasImport = false;
        for (string s : imports)
        {
            size_t pos = s.find_last_of('.');
            string temppackage = s.substr(0, pos);
            if (temppackage != package)
            {
                hasImport = true;
                result += "import " + s + ";\n";
            }
        }
        if (hasImport)
        {
            result += "\n";
        }
        result += visibility + "enum " + name + " {\n";
        for (std::size_t i = 0; i < elements.size(); i++)
        {
            result += "\t" + elements[i];
            if (i != elements.size() - 1)
            {
                result += ",\n";
            }
        }
        result += "\n}";
        return result;
    }
    void toFile()
    {
        ofstream *p = createFile(package, name);
        if (p == nullptr)
        {
            cout << "Error creating file\n";
            return;
        }
        *p << this->toString();
        p->close();
        delete p;
    }
    void process() override
    {
        fstream file(testPath);
        string line;
        bool go = true;
        while (go && getline(file, line))
        {
            if (line.find("theEnum") != string::npos)
            {
                go = false;
            }
        }
        getline(file, line);
        visibility = protlvl(line);
        while (getline(file, line))
        {
            if (line.find("hasEnumElements") != string::npos)
            {
                string temp = line.substr(line.find("hasEnumElements(") + 16);
                temp = temp.substr(0, temp.find(")"));
                vector<string> tempV = split(temp, ", ");
                for (std::size_t i = 0; i < tempV.size(); i++)
                {
                    elements.push_back(removeQuotes(tempV[i]));
                }
            }
        }
        file.close();
    }
};

class Class : public File
{
public:
    Class(string inpth, Wrapper *wp, string iname, string iparent, string iinterface) : File(inpth, wp)
    {
        package = "";
        size_t pos = iname.find_last_of('.');
        if (pos != string::npos)
        {
            package = iname.substr(0, pos);
            iname = iname.substr(pos + 1);
        }
        this->name = iname;
        string patentpckg = "";
        pos = iparent.find_last_of('.');
        if (pos != string::npos)
        {
            patentpckg = iparent.substr(0, pos);
            iparent = iparent.substr(pos + 1);
        }
        this->parent = iparent;
        if (patentpckg != "")
        {
            imports.insert(patentpckg + "." + iparent);
        }
        string interfacepckg = "";
        pos = iinterface.find_last_of('.');
        if (pos != string::npos)
        {
            interfacepckg = iinterface.substr(0, pos);
            iinterface = iinterface.substr(pos + 1);
        }
        this->interface = iinterface;
        if (interfacepckg != "")
        {
            imports.insert(interfacepckg + "." + iinterface);
        }
    }
    ~Class()
    {
        for (std::size_t i = 0; i < fields.size(); i++)
        {
            delete (fields[i]);
        }
        for (std::size_t i = 0; i < methods.size(); i++)
        {
            delete (methods[i]);
        }
        for (std::size_t i = 0; i < constructors.size(); i++)
        {
            delete (constructors[i]);
        }
    };
    string parent;    // its a class
    string interface; // its an interface
    vector<Field *> fields;
    vector<Method *> methods;
    vector<Constructor *> constructors;
    bool hasTextualRepresentation = false;
    bool hasEqualityCheck = false;
    bool hasHashCode = false;
    string toString()
    {
        string result = "package " + package + ";\n\n";
        bool hasImport = false;
        for (string s : imports)
        {
            size_t pos = s.find_last_of('.');
            string temppackage = s.substr(0, pos);
            if (temppackage != package)
            {
                hasImport = true;
                result += "import " + s + ";\n";
            }
        }
        if (hasImport)
        {
            result += "\n";
        }
        result += visibility + "class " + name;
        if (parent != "")
        {
            result += " extends " + parent;
        }
        if (interface != "")
        {
            result += " implements " + interface;
        }
        result += " {\n";
        for (std::size_t i = 0; i < fields.size(); i++)
        {
            result += fields[i]->toString();
        }
        for (std::size_t i = 0; i < constructors.size(); i++)
        {
            result += constructors[i]->toString(this, 0);
        }
        for (std::size_t i = 0; i < methods.size(); i++)
        {
            result += methods[i]->toString(true, this, 0);
        }
        if (hasTextualRepresentation)
        {
            result += "\n\t@Override\n";
            result += "\tpublic String toString() {\n";
            result += "\t\treturn \"TODO\";\n";
            result += "\t}\n";
        }
        if (hasEqualityCheck)
        {
            result += "\n\t@Override\n";
            result += "\tpublic boolean equals(Object obj) {\n";
            result += "\t\t// todo\n";
            result += "\t\treturn true;\n";
            result += "\t}\n";
        } // need to overwrite hashCode too
        if (hasHashCode || hasEqualityCheck)
        {
            result += "\n\t@Override\n";
            result += "\tpublic int hashCode() {\n";
            result += "\t\t// todo\n";
            result += "\t\treturn 1;\n";
            result += "\t}\n";
        }
        result += "}";
        return result;
    }
    void toFile()
    {
        ofstream *p = createFile(package, name);
        if (p == nullptr)
        {
            cout << "Error creating file\n";
            return;
        }
        *p << this->toString();
        p->close();
        delete p;
    }
    void process() override
    {
        fstream file(testPath);
        string line;
        bool go = true;
        while (go && getline(file, line))
        {
            if (line.find("theClass") != string::npos)
            {
                go = false;
            }
        }
        getline(file, line);
        visibility = protlvl(line);
        while (getline(file, line))
        {
            if (line.find("it.hasMethod") != string::npos)
            {
                methods.push_back(createMethod(&file, line, this));
            }
            else if (line.find("it.hasConstructor") != string::npos)
            {
                constructors.push_back(createConstructor(&file, line, this));
            }
            else if (line.find("hasNoArgConstructor") != string::npos)
            {
                getline(file, line);
                string vis = protlvl(line);
                Constructor *c = new Constructor();
                c->name = this->name;
                c->visibility = vis;
                constructors.push_back(c);
            }
            else if (line.find("it.hasField") != string::npos)
            {
                fields.push_back(createField(&file, line, this));
            }
            else if (line.find("it.has(TEXTUAL_REPRESENTATION") != string::npos)
            {
                hasTextualRepresentation = true;
            }
            else if (line.find("has(EQUALITY_CHECK") != string::npos)
            {
                hasEqualityCheck = true;
                hasHashCode = true;
            }
            else if (line.find("has(HASH_CODE") != string::npos)
            {
                hasHashCode = true;
            }
        }
        file.close();
    }
};

class Interface : public File
{
public:
    Interface(string inpth, Wrapper *wp, string iname, string iinterface) : File(inpth, wp)
    {
        package = "";
        size_t pos = iname.find_last_of('.');
        if (pos != string::npos)
        {
            package = iname.substr(0, pos);
            iname = iname.substr(pos + 1);
        }
        this->name = iname;
        string interfacepckg = "";
        pos = iinterface.find_last_of('.');
        if (pos != string::npos)
        {
            interfacepckg = iinterface.substr(0, pos);
            iinterface = iinterface.substr(pos + 1);
        }
        this->parentInterface = iinterface;
        if (interfacepckg != "")
        {
            imports.insert(interfacepckg + "." + iinterface);
        }
    }
    ~Interface()
    {
        for (std::size_t i = 0; i < fields.size(); i++)
        {
            delete (fields[i]);
        }
        for (std::size_t i = 0; i < methods.size(); i++)
        {
            delete (methods[i]);
        }
    };
    string parentInterface; // its an interface
    vector<Field *> fields;
    vector<Method *> methods;
    string toString()
    {
        string result = "package " + package + ";\n\n";
        bool hasImport = false;
        for (string s : imports)
        {
            size_t pos = s.find_last_of('.');
            string temppackage = s.substr(0, pos);
            if (temppackage != package)
            {
                hasImport = true;
                result += "import " + s + ";\n";
            }
        }
        if (hasImport)
        {
            result += "\n";
        }
        result += visibility + "interface " + name;
        if (parentInterface != "")
        {
            result += " extends " + parentInterface;
        }
        result += " {\n";
        for (std::size_t i = 0; i < fields.size(); i++)
        {
            result += fields[i]->toString();
        }
        for (std::size_t i = 0; i < methods.size(); i++)
        {
            result += methods[i]->toString(false, this, 2);
        }
        result += "\n}";
        return result;
    }
    void toFile()
    {
        ofstream *p = createFile(package, name);
        if (p == nullptr)
        {
            cout << "Error creating file\n";
            return;
        }
        *p << this->toString();
        p->close();
        delete p;
    }
    void process() override
    {

        fstream file(testPath);
        string line;
        bool go = true;
        while (go && getline(file, line))
        {
            if (line.find("theInterface") != string::npos)
            {
                go = false;
            }
        }
        getline(file, line);
        visibility = protlvl(line);
        while (getline(file, line))
        {
            if (line.find("it.hasMethod") != string::npos)
            {
                methods.push_back(createMethod(&file, line, this));
            }
            else if (line.find("it.hasConstructor") != string::npos)
            {
                // interfaces don't have constructors
                cout << "Error - Interface: " + name + " has constructor\n";
            }
            else if (line.find("it.hasField") != string::npos)
            {
                fields.push_back(createField(&file, line, this));
            }
        }
        file.close();
    }
};

class Exception : public File
{
public:
    Exception(string inpth, Wrapper *wp, string iname, string iparent) : File(inpth, wp)
    {
        package = "";
        size_t pos = iname.find_last_of('.');
        if (pos != string::npos)
        {
            package = iname.substr(0, pos);
            iname = iname.substr(pos + 1);
        }
        this->name = iname;
        string patentpckg = "";
        pos = iparent.find_last_of('.');
        if (pos != string::npos)
        {
            patentpckg = iparent.substr(0, pos);
            iparent = iparent.substr(pos + 1);
        }
        this->parentException = iparent;
        if (patentpckg != "")
        {
            imports.insert(patentpckg + "." + iparent);
        }
    }
    ~Exception()
    {
        for (std::size_t i = 0; i < fields.size(); i++)
        {
            delete (fields[i]);
        }
        for (std::size_t i = 0; i < methods.size(); i++)
        {
            delete (methods[i]);
        }
        for (std::size_t i = 0; i < constructors.size(); i++)
        {
            delete (constructors[i]);
        }
    };
    string parentException;
    vector<Field *> fields;
    vector<Method *> methods;
    vector<Constructor *> constructors;
    string toString()
    {
        string result = "package " + package + ";\n\n";
        bool hasImport = false;
        for (string s : imports)
        {
            size_t pos = s.find_last_of('.');
            string temppackage = s.substr(0, pos);
            if (temppackage != package)
            {
                hasImport = true;
                result += "import " + s + ";\n";
            }
        }
        if (hasImport)
        {
            result += "\n";
        }
        result += visibility + "class " + name;
        if (parentException != "")
        {
            result += " extends " + parentException;
        }
        result += " {\n";
        for (std::size_t i = 0; i < fields.size(); i++)
        {
            result += fields[i]->toString();
        }
        for (std::size_t i = 0; i < constructors.size(); i++)
        {
            result += constructors[i]->toString(this, 1);
        }
        for (std::size_t i = 0; i < methods.size(); i++)
        {
            result += methods[i]->toString(true, this, 1);
        }
        result += "\n}";
        return result;
    }
    void toFile()
    {
        ofstream *p = createFile(package, name);
        if (p == nullptr)
        {
            cout << "Error creating file\n";
            return;
        }
        *p << this->toString();
        p->close();
        delete p;
    }
    void process() override
    {
        fstream file(testPath);
        string line;
        bool go = true;
        while (go && getline(file, line))
        {
            if (line.find("theCheckedException") != string::npos)
            {
                go = false;
            }
        }
        getline(file, line);
        visibility = protlvl(line);
        while (getline(file, line))
        {
            if (line.find("it.hasMethod") != string::npos)
            {
                methods.push_back(createMethod(&file, line, this));
            }
            else if (line.find("it.hasConstructor") != string::npos)
            {
                constructors.push_back(createConstructor(&file, line, this));
            }
            else if (line.find("hasNoArgConstructor") != string::npos)
            {
                getline(file, line);
                string vis = protlvl(line);
                Constructor *c = new Constructor();
                c->name = this->name;
                c->visibility = vis;
                constructors.push_back(c);
            }
            else if (line.find("it.hasField") != string::npos)
            {
                fields.push_back(createField(&file, line, this));
            }
        }
        file.close();
    }
};

class Wrapper
{
public:
    Wrapper(){}; // default constructor
    ~Wrapper()
    {
        for (std::size_t i = 0; i < files.size(); i++)
        {
            delete (files[i]);
        }
    }
    vector<File *> files;
    vector<string> testSuites;
    void addFile(File *file)
    {
        if (file != nullptr)
        {
            files.push_back(file);
        }
    }
    void process()
    {
        for (std::size_t i = 0; i < files.size(); i++)
        {
            // cast file to class, interface or enum (happens only if called by pointers)
            files[i]->process();
        }
        for (std::size_t i = 0; i < files.size(); i++)
        {
            files[i]->toFile();
        }
        vector<string> testFiles;
        vector<string> imports;
        for (std::size_t i = 0; i < testSuites.size(); i++)
        {
            fstream file(testSuites[i]);
            string line;
            while (getline(file, line))
            {
                if (line.find("import") != string::npos)
                {
                    imports.push_back(line.substr(line.find("import") + 7, line.find(';') - 7)); // 6 for import 1 for extra space, -7 because second param is lenght
                }
                if (line.find("@SelectClasses") != string::npos)
                {
                    bool b = true;
                    while (b && getline(file, line))
                    {
                        if (line.find("@") != string::npos)
                        {
                            b = false;
                        }
                        if (line.find("Test.class") != string::npos && line.find("StructureTest.class") == string::npos)
                        {
                            while (line.find(' ') != string::npos)
                            {
                                line = line.substr(0, line.find(' ')) + line.substr(line.find(' ') + 1);
                            }
                            while (line.find(',') != string::npos)
                            {
                                line = line.substr(0, line.find(',')) + line.substr(line.find(',') + 1);
                            }
                            while (line.find('\t') != string::npos)
                            {
                                line = line.substr(0, line.find('\t')) + line.substr(line.find('\t') + 1);
                            }
                            line = line.substr(0, line.find(".class"));
                            testFiles.push_back(line);
                        }
                    }
                }
            }
            file.close();
        }
        for (std::size_t i = 0; i < testFiles.size(); i++)
        {
            string search = testFiles[i];
            string package = "";
            /* Search in files? -> don't do this every test class is imported
            search = search.substr(0, search.find("Test"));
            for (std::size_t j = 0; j < files.size(); j++)
            {
                if (files[j]->name == search)
                {
                    package += files[j]->package;
                    break;
                }
            }
            */
            for (std::size_t j = 0; j < imports.size(); j++)
            {
                if (search == imports[j].substr(imports[j].find_last_of('.') + 1))
                {
                    package += imports[j].substr(0, imports[j].find_last_of('.'));
                }
            }
            ofstream *outfile = createFile(package, testFiles[i]);
            if (package == "")
            {
                cout << "**ERROR** - Unkonw test file path!\n";
            }
            *outfile << "package " << package << ";\n\n";
            *outfile
                << "import static check.CheckThat.*;\nimport static org.junit.jupiter.api.Assertions.*;\nimport org.junit.jupiter.api.*;\nimport org.junit.jupiter.api.condition.*;\nimport org.junit.jupiter.api.extension.*;\nimport org.junit.jupiter.params.*;\nimport org.junit.jupiter.params.provider.*;\nimport check.*;\n\n";
            *outfile << "public class " << testFiles[i] << "{\n\t//TODO\n}";
            outfile->close();
            delete outfile;
        }
    }
    File *getParent(string name)
    {
        File *result = nullptr;
        for (std::size_t i = 0; i < files.size(); i++)
        {
            if (files[i]->name == name)
            {
                if (result == nullptr)
                {
                    result = files[i];
                }
                else
                {
                    cout << "**Error** - Multiple potential parents!\n";
                    return nullptr;
                }
            }
        }
        return result;
    }
};

// makes all necessary imports from typemaker
void importMaker(string type, File *parent)
{
    // type one -> some.package.name"
    // type two -> some.package.name[]
    // type three -> some.package.name<some.package.name>
    // type four -> some.package.name<some.package.name, some.package.name>
    // combinations like some.package.name<some.package.name[], some.package.name>
    if (type.find("[]") != string::npos)
    {
        string type1 = type.substr(0, type.find("[]"));
        string type2 = type.substr(type.find("[]") + 2);
        importMaker(type1, parent);
        importMaker(type2, parent);
        return;
    }
    if (type.find("<") != string::npos && type.find(">") != string::npos)
    {
        string type1 = type.substr(0, type.find("<"));
        string type2 = type.substr(type.find("<") + 1);
        type2 = type2.substr(0, type2.find_last_of(">"));
        importMaker(type1, parent);
        importMaker(type2, parent);
        return;
    }
    if (type.find(",") != string::npos)
    {
        string type1 = type.substr(0, type.find(","));
        string type2 = type.substr(type.find(",") + 2);
        importMaker(type1, parent);
        importMaker(type2, parent);
        return;
    }
    if (type.find(".") != string::npos)
    {
        parent->imports.insert(removeQuotes(type));
    }
    return;
}

// check for list and arrays (never seen a set or map in a test) => there is a map in one of the tests
string typeMaker(string type, File *parent)
{
    if (type.find(')') != string::npos)
    {
        type = type.substr(0, type.find(')'));
    }
    // don't need to write List or HashSet, they are simple types
    if (type.find("vararg of ") != string::npos)
    {
        type = type.substr(type.find("of ") + 3) + "...";
    }
    else if (type.find("array of ") != string::npos)
    {
        type = type.substr(type.find("of ") + 3) + "[]";
    }
    else if (type.find("HashMap of ") != string::npos)
    {
        if (parent != nullptr)
        {
            parent->imports.insert("java.util.*");
        }
        string temp = type.substr(type.find("of ") + 3);
        string key = temp.substr(0, temp.find(" to "));
        string value = temp.substr(temp.find(" to ") + 4);
        type = "HashMap<" + typeMaker(key, parent) + ", " + typeMaker(value, parent) + ">";
    }
    else if (type.find("of ") != string::npos) // try to autocomplete unimplemented types
    {
        if (parent != nullptr)
        {
            parent->imports.insert("java.util.*");
        }
        type = type.substr(0, type.find("of ") - 1) + "<" + type.substr(type.find("of ") + 3) + ">";
    }
    if (type.find("of ") != string::npos)
    {
        if (type.find("...") != string::npos)
        {
            return typeMaker(type.substr(0, type.find("...")), parent) + "...";
        }
        if (type.find("<") != string::npos && type.find(">") != string::npos)
        {
            if (type.find(",") != string::npos)
            {
                string key = type.substr(0, type.find(","));
                key = typeMaker(key.substr(key.find("<") + 1), parent);
                string value = type.substr(type.find(",") + 2);
                value = typeMaker(value.substr(0, value.find(">")), parent);
                return removeQuotes(type.substr(0, type.find("<") + 1) + key + ", " + value + type.substr(type.find(">")));
            }
            string temp = type.substr(type.find("<"));
            temp = temp.substr(1, temp.find(">") - 1);
            return removeQuotes(type.substr(0, type.find("<") + 1) + typeMaker(temp, parent) + type.substr(type.find(">")));
        }
        else if (type.find("array of ") != string::npos)
        {
            return typeMaker(type.substr(type.find("of ") + 3) + "[]", parent);
        }
    }
    else if (type.find(".") != string::npos && type.find("...") == string::npos)
    {
        if (parent != nullptr)
        {
            importMaker(type, parent);
        }
        type = type.substr(type.find_last_of('.') + 1);
    }
    return removeQuotes(type);
}

pair<string, string> typeNameSeparator(string line, File *parent)
{
    string type;
    string name;
    if (line.find("ofType") != string::npos)
    {
        // structure => it.hasField("defaultAuthor", ofType("String"))
        name = line.substr(line.find('"') + 1);
        name = name.substr(0, name.find('"'));
        type = line.substr(line.find("ofType(\"") + 8);
        type = type.substr(0, type.find("\")"));
        return make_pair(typeMaker(type, parent), removeQuotes(name));
    }
    else
    {
        if (line.find(':') != string::npos)
        {
            type = line.substr(line.find(':') + 2);
            name = line.substr(0, line.find(':'));
            return make_pair(typeMaker(type, parent), removeQuotes(name));
        }
        else
        {
            return make_pair(typeMaker(line, parent), "todoName");
        }
    }
}

Field *createField(fstream *infile, string line, File *parent)
{
    Field *f = new Field();
    // structure -> ("name: type")
    // auto [type, name] = typeNameSeparator(line); // only works after c++11
    string tempname;
    string temptype;
    if (line.find("hasFieldOfType") != string::npos)
    {
        tempname = line.substr(line.find('"') + 1);
        tempname = tempname.substr(0, tempname.find('"'));
        temptype = line.substr(0, line.find_last_of('"'));
        temptype = temptype.substr(temptype.find(',') + 3);
    }
    else
    {
        pair<string, string> temp = typeNameSeparator(line, parent);
        temptype = temp.first;
        tempname = temp.second;
    }
    f->name = tempname;
    f->type = typeMaker(temptype, parent);
    getline(*infile, line);
    // search for visibility
    f->visibility = protlvl(line);
    if (line.find("USABLE_WITHOUT_INSTANCE") != string::npos)
    {
        f->isStatic = true;
    }
    else
    {
        f->isStatic = false;
    }
    if (line.find("NOT_MODIFIABLE") != string::npos)
    {
        f->isFinal = true;
    }
    else
    {
        f->isFinal = false;
    }
    getline(*infile, line);
    if (line.find("thatHas(") != string::npos && line.find("thatHasNo(") == string::npos)
    {
        if (line.find("GETTER") != string::npos)
        {
            f->hasGetter = true;
        }
        if (line.find("SETTER") != string::npos)
        {
            f->hasSetter = true;
        }
    }
    return f;
}

Method *createMethod(fstream *infile, string line, File *parent)
{
    Method *m = new Method();
    string tempReturnType = "void";
    vector<string> tempParamTypes;
    vector<string> tempParamNames;
    string tempname;
    tempname = line.substr(line.find('"') + 1);
    tempname = tempname.substr(0, tempname.find('"'));
    m->name = tempname;
    if (line.find("withArgsSimilarToFields") != string::npos) // using the File class we can search for these
    {
        m->simField = true;
        string temp = line.substr(line.find("withArgsSimilarToFields") + 24);
        temp = temp.substr(temp.find('"'));
        temp = temp.substr(0, temp.find(")"));
        temp = temp.substr(0, temp.find_last_of('"'));
        temp = removeQuotes(temp);
        tempParamNames = split(temp, ", ");
    }
    else if (line.find("withParams") != string::npos)
    {
        string temp = line.substr(line.find("withParams(") + 11);
        temp = temp.substr(0, temp.find(")"));
        vector<string> tempV = split(temp, ", ");
        for (std::size_t i = 0; i < tempV.size(); i++)
        {
            // auto [temptype, tempname] = typeNameSeparator(tempV[i]); // only works after c++11
            pair<string, string> tempPair = typeNameSeparator(tempV[i], parent);
            string tempParamType = tempPair.first;
            string tempParamName = tempPair.second;
            if (tempname == "todoName")
            {
                tempname = "todoName" + to_string(i);
            }
            tempParamTypes.push_back(tempParamType);
            tempParamNames.push_back(tempParamName);
        }
    }
    else if (line.find("hasMethodWithParams") != string::npos)
    {
        string temp = line.substr(line.find(',') + 2);
        vector<string> tempV = split(temp, ", ");
        for (std::size_t i = 0; i < tempV.size(); i++)
        {
            tempParamTypes.push_back(typeMaker(tempV[i], parent));
            tempParamNames.push_back("todoName" + to_string(i));
        }
    } // else no params
    m->paramNames = tempParamNames;
    m->paramTypes = tempParamTypes;
    for (int i = 0; i < 2; i++)
    {
        getline(*infile, line);
        if (line.find("thatIs(") != string::npos)
        {
            m->visibility = protlvl(line);
            if (line.find("USABLE_WITHOUT_INSTANCE") != string::npos)
            {
                m->isStatic = true;
            }
            else
            {
                m->isStatic = false;
            }
        }
        else if (line.find("thatReturns") != string::npos)
        {
            if (line.find("thatReturnsNothing") != string::npos)
            {
                tempReturnType = "void";
            }
            else
            {
                tempReturnType = line.substr(line.find('"') + 1);
                tempReturnType = tempReturnType.substr(0, tempReturnType.find('"'));
                tempReturnType = typeMaker(tempReturnType, parent);
            }
        }
        m->returnType = tempReturnType;
    }
    return m;
}

Constructor *createConstructor(fstream *infile, string line, File *parent)
{
    Constructor *c = new Constructor();
    c->name = parent->name;
    vector<string> tempParamTypes;
    vector<string> tempParamNames;
    if (line.find("withArgs") != string::npos)
    {
        if (line.find("withArgsAsInParent") != string::npos) // using the File class we can search for these
        {
            c->simParent = true;
        }
        else if (line.find("withArgsSimilarToFields") != string::npos) // using the File class we can search for these
        {
            c->simField = true;
            string temp = line.substr(line.find("withArgsSimilarToFields") + 24);
            temp = temp.substr(temp.find('"'));
            temp = temp.substr(0, temp.find(")"));
            temp = temp.substr(0, temp.find_last_of('"'));
            temp = removeQuotes(temp);
            tempParamNames = split(temp, ", ");
        }
        else if (line.find("withArgs(") != string::npos)
        {
            string temp = line.substr(line.find("withArgs(") + 9);
            temp = temp.substr(0, temp.find(")"));
            vector<string> tempV = split(temp, ", ");
            for (std::size_t i = 0; i < tempV.size(); i++)
            {
                // auto [temptype, tempname] = typeNameSeparator(tempV[i]); // only works after c++11
                pair<string, string> tempPair = typeNameSeparator(tempV[i], parent);
                string temptype = tempPair.first;
                string tempname = tempPair.second;
                if (tempname == "todoName")
                {
                    tempname = "todoName" + to_string(i);
                }
                tempParamTypes.push_back(temptype);
                tempParamNames.push_back(tempname);
            }
        }
        else
        {
            cout << "**WARNING** Constructor with unimplemented args type\n";
            tempParamTypes.push_back("unimplemented");
            tempParamNames.push_back("unimplemented");
        }
    }
    else if (line.find("hasConstructorWithParams") != string::npos)
    {
        string temp = line.substr(line.find('(') + 1);
        temp = temp.substr(0, temp.find(')'));
        vector<string> tempV = split(temp, ", ");
        for (std::size_t i = 0; i < tempV.size(); i++)
        {
            tempParamTypes.push_back(typeMaker(tempV[i], parent));
            tempParamNames.push_back("todoName" + to_string(i));
        }
    }
    else if (line.find("withParams") != string::npos)
    {
        string temp = line.substr(line.find("withParams") + 12);
        temp = temp.substr(0, temp.find(')'));
        vector<string> tempV = split(temp, ", ");
        for (std::size_t i = 0; i < tempV.size(); i++)
        {
            // auto [temptype, tempname] = typeNameSeparator(tempV[i]); // only works after c++11
            pair<string, string> tempPair = typeNameSeparator(tempV[i], parent);
            string temptype = tempPair.first;
            string tempname = tempPair.second;
            if (tempname == "todoName")
            {
                tempname = "todoName" + to_string(i);
            }
            tempParamTypes.push_back(temptype);
            tempParamNames.push_back(tempname);
        }
    } // else no params
    c->paramNames = tempParamNames;
    c->paramTypes = tempParamTypes;
    getline(*infile, line);
    c->visibility = protlvl(line);
    return c;
}

// type == 0 similarToFields, type == 1 similarToParent || parenttype == 0 class parenttype == 1 exception
string getSpecialParam(void *caller, int type, int parenttype, vector<string> paramNames, bool isConstructor)
{
    string result = "";
    if (parenttype == 0)
    {
        Class *c = static_cast<Class *>(caller);
        if (type == 0)
        {
            vector<string> paramTypes;
            for (std::size_t i = 0; i < c->fields.size(); i++)
            {
                for (std::size_t j = 0; j < paramNames.size(); j++)
                {
                    if (paramNames[j] == c->fields[i]->name)
                    {
                        paramTypes.push_back(c->fields[i]->type);
                    }
                }
            }
            for (std::size_t i = 0; i < paramTypes.size(); i++)
            {
                result += paramTypes[i] + " " + paramNames[i];
                if (i != paramTypes.size() - 1)
                {
                    result += ", ";
                }
            }
            result += ")";
            if (isConstructor)
            {
                result += " {\n";
                for (std::size_t i = 0; i < paramNames.size(); i++)
                {
                    result += "\t\tthis." + paramNames[i] + " = " + paramNames[i] + ";\n";
                }
            }
        }
        else if (type == 1)
        {
            File *parent = c->wrapper->getParent(c->parent);
            if (parent == nullptr)
            {
                result += "TODO simParent){\n ";
            }
            else
            {
                Class *classParent = dynamic_cast<Class *>(parent);
                if (classParent != nullptr)
                {
                    Constructor *constructor = nullptr;
                    std::size_t prtc = 0;
                    for (std::size_t i = 0; i < classParent->constructors.size(); i++)
                    {
                        if (classParent->constructors[i]->paramTypes.size() > prtc)
                        {
                            constructor = classParent->constructors[i];
                        }
                    }
                    if (constructor == nullptr)
                    {
                        result += "TODO simParent){\n ";
                    }
                    else
                    {
                        for (std::size_t i = 0; i < constructor->paramTypes.size(); i++)
                        {
                            result += constructor->paramTypes[i] + " " + constructor->paramNames[i];
                            if (i != constructor->paramTypes.size() - 1)
                            {
                                result += ", ";
                            }
                        }
                        result += ") {\n";
                        result += "\t\tsuper(";
                        for (std::size_t i = 0; i < constructor->paramNames.size(); i++)
                        {
                            result += constructor->paramNames[i];
                            if (i != constructor->paramNames.size() - 1)
                            {
                                result += ", ";
                            }
                        }
                        result += ");\n\t\t//TODO\n";
                    }
                }
                else
                {
                    result += "TODO simParent){\n ";
                }
            }
        }
    }
    else if (parenttype == 1) // I have no idea how to do this better :/
    {
        Exception *e = static_cast<Exception *>(caller);
        if (type == 0)
        {
            vector<string> paramTypes;
            for (std::size_t i = 0; i < e->fields.size(); i++)
            {
                for (std::size_t j = 0; j < paramNames.size(); j++)
                {
                    if (paramNames[j] == e->fields[i]->name)
                    {
                        paramTypes.push_back(e->fields[i]->type);
                    }
                }
            }
            for (std::size_t i = 0; i < paramTypes.size(); i++)
            {
                result += paramTypes[i] + " " + paramNames[i];
                if (i != paramTypes.size() - 1)
                {
                    result += ", ";
                }
            }
            result += ") {\n\t\t//TODO}\n";
        }
        else if (type == 1)
        {
            File *parent = e->wrapper->getParent(e->parentException);
            if (parent == nullptr)
            {
                result += "TODO simParent){\n ";
            }
            else
            {
                Exception *exceptionParent = dynamic_cast<Exception *>(parent);
                if (exceptionParent != nullptr)
                {
                    Constructor *constructor = nullptr;
                    std::size_t prtc = 0;
                    for (std::size_t i = 0; i < exceptionParent->constructors.size(); i++)
                    {
                        if (exceptionParent->constructors[i]->paramTypes.size() > prtc)
                        {
                            constructor = exceptionParent->constructors[i];
                        }
                    }
                    if (constructor == nullptr)
                    {
                        result += "TODO simParent){\n ";
                    }
                    else
                    {
                        for (std::size_t i = 0; i < constructor->paramTypes.size(); i++)
                        {
                            result += constructor->paramTypes[i] + " " + constructor->paramNames[i];
                            if (i != constructor->paramTypes.size() - 1)
                            {
                                result += ", ";
                            }
                        }
                        result += ") {\n";
                        result += "\t\tsuper(";
                        for (std::size_t i = 0; i < constructor->paramNames.size(); i++)
                        {
                            result += constructor->paramNames[i];
                            if (i != constructor->paramNames.size() - 1)
                            {
                                result += ", ";
                            }
                        }
                        result += ");\n\t\t//TODO\n";
                    }
                }
                else
                {
                    result += "TODO simParent){\n ";
                }
            }
        }
    }
    return result;
}

File *getFile(string inpth, Wrapper *wp) // nullptr == fail
{
    fstream file(inpth);
    string line;
    string stopString = "CheckThat.the";
    if (file.is_open())
    {
        bool go = true;
        while (go && getline(file, line)) // first bool else it will skip the first line
        {
            if (line.find(stopString) != string::npos)
            {
                go = false;
            }
        }
        if (line.find("theClass") != string::npos)
        {
            string name;
            name = line.substr(line.find("theClass(") + 9);
            size_t pos = name.find(',');
            string parent = "";
            string interface = "";
            if (pos != string::npos)
            {
                name = name.substr(0, pos);
                if (line.find("withParent(") != string::npos)
                {
                    parent = line.substr(line.find("withParent(") + 11);
                    pos = parent.find(')');
                    parent = parent.substr(0, pos);
                    pos = parent.find('"');                 // this must be here
                    size_t pos2 = parent.find_last_of('"'); // this must not be the same as above
                    parent = parent.substr(pos + 1, pos2 - pos - 1);
                    /*if (parent.find("of ") != string::npos) // this removes quotes // TODO this is wrong
                    {
                        parent = typeMaker(parent, nullptr);
                    }*/
                }
                if (line.find("withInterface(") != string::npos)
                {
                    interface = line.substr(line.find("withInterface(") + 14);
                    pos = interface.find(')');
                    interface = interface.substr(0, pos);
                    pos = interface.find('"');                 // this must be here
                    size_t pos2 = interface.find_last_of('"'); // this must not be the same as above
                    interface = interface.substr(pos + 1, pos2 - pos - 1);
                }
            }
            else
            {
                name = name.substr(0, name.find(')'));
            }
            pos = name.find('"');
            size_t pos2 = name.find_last_of('"');
            name = name.substr(pos + 1, pos2 - pos - 1);
            return (new Class(removeQuotes(inpth), wp, removeQuotes(name), removeQuotes(parent), removeQuotes(interface)));
        }
        else if (line.find("theInterface") != string::npos)
        {
            string name;
            name = line.substr(line.find("theInterface(") + 13);
            size_t pos = name.find(',');
            string interface = "";
            if (pos != string::npos)
            {
                name = name.substr(0, pos);
                if (line.find("withInterface(") != string::npos)
                {
                    interface = line.substr(line.find("withInterface(") + 14);
                    pos = interface.find(')');
                    interface = interface.substr(0, pos);
                    pos = interface.find('"');                 // this must be here
                    size_t pos2 = interface.find_last_of('"'); // this must not be the same as above
                    interface = interface.substr(pos + 1, pos2 - pos - 1);
                }
            }
            else
            {
                name = name.substr(0, name.find(')'));
            }
            return (new Interface(removeQuotes(inpth), wp, removeQuotes(name), removeQuotes(interface)));
        }
        else if (line.find("theEnum") != string::npos)
        {
            string name;
            name = line.substr(line.find("theEnum(") + 8);
            name = name.substr(0, name.find(')'));
            size_t pos = name.find('"');
            size_t pos2 = name.find_last_of('"');
            name = name.substr(pos + 1, pos2 - pos - 1);
            return (new Enum(removeQuotes(inpth), wp, removeQuotes(name)));
        }
        else if (line.find("theCheckedException") != string::npos)
        {
            string name;
            name = line.substr(line.find("theCheckedException(") + 9);
            size_t pos = name.find(',');
            string parent = "";
            if (pos != string::npos)
            {
                name = name.substr(0, pos);
                if (line.find("withParent(") != string::npos)
                {
                    parent = line.substr(line.find("withParent(") + 11);
                    pos = parent.find(')');
                    parent = parent.substr(0, pos);
                    pos = parent.find('"');                 // this must be here
                    size_t pos2 = parent.find_last_of('"'); // this must not be the same as above
                    parent = parent.substr(pos + 1, pos2 - pos - 1);
                    /*if (parent.find("of ") != string::npos) // this removes quotes // TODO this is wrong
                    {
                        parent = typeMaker(parent, nullptr);
                    }*/
                }
            }
            else
            {
                name = name.substr(0, name.find(')'));
            }
            pos = name.find('"');
            size_t pos2 = name.find_last_of('"');
            name = name.substr(pos + 1, pos2 - pos - 1);
            return (new Exception(removeQuotes(inpth), wp, removeQuotes(name), removeQuotes(parent)));
        }
        else
        {
            cout << "Unknown type of class\n"
                 << line << endl;
            file.close();
            return nullptr;
        }
    }
    else
    {
        std::cout << "Unable to open file: " << inpth << std::endl;
        file.close();
        return nullptr;
    }
}

int main(int argc, char **args)
{
    if (argc < 2)
    {
        cout << "Usage: " << args[0] << " <input file>" << endl;
        exit(1);
    }
    Wrapper *wp = new Wrapper(); // this is a global variable
    for (int i = 1; i < argc; i++)
    {
        if (static_cast<string>(args[i]).find("Suite.java") != string::npos)
        {
            wp->testSuites.push_back(args[i]);
        }
        else
        {
            wp->addFile(getFile(args[i], wp));
        }
    }
    wp->process();
    delete wp;
    return 0;
}
