#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include <cstring>
#include <ctype.h>

//#include <QFileInfo>

//#include "checkfile.cpp"
//#include "checkoneline.cpp"
//#include "checktwoline.cpp"
//#include "func.cpp"

/*using namespace std;

extern bool ifDefBrac; //включен, если до этого уже просканили хотя бы одни кавычки
extern bool brac; //включен, если кавычки остаются в той же строке

extern bool ifDefIndent;  //включен, если до этого уже просканили хотя бы один отступ
extern int ind; //количество пробелов в отступе

string uncoment(string line)
{
    size_t pos = 0;
    string unLine = "";
    if ((pos = line.find("//")) != string::npos)
    {
        unLine = line.erase(pos);
        return unLine;
    }
    else
    {
        return line;
    }
}

string unspaceForward(string line)
{
    int i = 0;
    while (line[i] == ' ')
    {
        line.erase(i);
        i++;
    }
}

string unspaceBackward(string line)
{
    int i = line.size() - 1;
    while (line[i] == ' ')
    {
        line.erase(i);
        i--;
    }
    return line;
    return line;
}

void printError(string error)
{
    ofstream fout("log.txt"); // создаём объект класса ofstream для записи и связываем его с файлом cppstudio.txt
    fout << error; // запись строки в файл
    fout.close(); // закрываем файл
}

//void checkUtf8File(string file_name);
//void checkFileName(string file_name);
void checkComment(string file_name);
void checkMultipleInclude(string file_name);

void checkFile(string file_name)
{
    //checkUtf8File(file_name);
    //checkFileName(file_name);
    checkComment(file_name);
    checkMultipleInclude(file_name);
}

/*void checkUtf8File(string file_name)
{
    ifstream file(file_name.c_str());
    if (!file)
        return; // even better, throw here

    istreambuf_iterator<char> it(file.rdbuf());
    istreambuf_iterator<char> eos;

    if (utf8::is_valid(it, eos))
    {
        std::stringstream ss("");
        ss << "incorrect encoding!";
        cout << ss.str() << endl;
        printError(ss.str());
    }
    file.close();
}*/

/*void checkFileName(string file_name)
{
    QFileInfo fi(QString::fromStdString(file_name));
    string basename =  fi.baseName().toStdString();
    std::cout << "path: " << basename << std::endl;
    for(unsigned int i =0; i < basename.size(); i++)
    {
        if (isalpha(basename[i]) && basename[i] == toupper(basename[i]))
        {
            std::stringstream ss("");
            ss << "filename must be in lower register! " << basename[i];
            cout << ss.str() << endl;
            printError(ss.str());

            break;
        }
    }
}*/

/*void checkComment(string file_name)
{
    if  (file_name.find(".h") != string::npos || file_name.find(".hpp") != string::npos)
    {
        ifstream file(file_name.c_str());
        string strline;
        if (file.is_open())
        {
            getline(file,strline);
            std::cout << "EEEEEEEE: " << strline << " " << strline.size() << std::endl;

            if (strline.find("/*") == string::npos && strline.find("//") == string::npos)
            {
                std::stringstream ss("");
                ss << "must add comment to introduce  file";
                cout << ss.str() << endl;
                printError(ss.str());
            }
            else if (strline.size() < 10)
            {
                std::stringstream ss("");
                ss << "must add a bigger comment to introduce  file";
                cout << ss.str() << endl;
                printError(ss.str());

            }
            file.close();
        }
    }
}

void checkMultipleInclude(string file_name)
{
    bool isIfndef = false;
    if (file_name.find(".h") != string::npos || file_name.find(".hpp") != string::npos)
    {
        ifstream file(file_name.c_str());
        string strline;
        if (file.is_open())
        {
            getline(file,strline);
            while (getline(file,strline))
            {
                string str = uncoment(strline);
                if (str.find("#ifndef") != string::npos)
                {
                    isIfndef = true;
                }
            }
            file.close();
        }
        if (!isIfndef)
        {
            std::stringstream ss("");
            ss << "must add #ifndef to exclude muiltiple include";
            cout << ss.str() << endl;
            printError(ss.str());
        }
    }
}

void checkIndent(string strline, string prevline, int line);
void checkFuncIndent(string strline, string prevline, int line);
void checkTwoLine(string strline, string prevline, int line)
{
    //checkIndent(strline, prevline, line);
    checkFuncIndent(strline, prevline, line);
}

int indent(string line)
{
    int i = 0;
    while (line[i] == ' ')
    {
        i++;
    }

    return i;
}

void checkIndent(string strline, string prevline, int line)
{
    //std::cout << "EEEEEEEEEEEE: " << strline << " " << ifDefIndent << " " << ind << std::endl;
    if(ifDefIndent)
    {
        string pvln = prevline;
        int fpv = pvln.size() -1;
        int fst = strline.size() - 1;
        if(pvln[fpv] == '{')
        {
            if (indent(strline) != (indent(pvln)+ind))
            {
                std::stringstream ss("");
                ss << "Неровное количество отступов в { строке " << line;
                cout << ss.str() << endl;
                printError(ss.str());
            }
        }
        else if(strline[fst] == '}')
        {
            std::cout << "prevline: " << prevline << " " << indent(prevline) << std::endl;
            std::cout << "strline: " << strline << " " << indent(strline) << std::endl << std::endl;
            if (indent(strline) != (indent(pvln)-ind))
            {
                std::stringstream ss("");
                ss << "Неровное количество отступов в } строке " << line;
                cout << ss.str() << endl;
                printError(ss.str());
            }
        }
        else
        {
            if (indent(strline) != indent(pvln))
            {
                std::stringstream ss("");
                ss << "Неровное количество отступов в строке " << line;
                cout << ss.str() << endl;
                printError(ss.str());
            }
        }
    }
}

void checkFuncIndent(string strline, string prevline, int line)
{
    int openBracCount = 0, closeBracCount = 0, index = -1;
    for (unsigned int i = 0; i < prevline.size(); i++)
    {
        if (prevline[i] == '(')
        {
            if (openBracCount == 0)
            {
                index = i;
            }
            openBracCount++;
        }
        if (prevline[i] == ')')
        {
            closeBracCount++;
        }
    }
    if (openBracCount > closeBracCount)
    {
        unsigned int i = 0;
        while (strline[i] == ' ')
        {
            i++;
        }
        if (i != (index+1))
        {
            std::stringstream ss("");
            ss << "Надо выравнивать по первому аргументу " << line;
            cout << ss.str() << endl;
            printError(ss.str());
        }
    }
}

void process(std::string filename)
{
    std::string strline = "", prevline = "", nextline = "";

    std::ifstream file(filename.c_str());
    unsigned int line = 0;

    checkFile(filename);
    std::cout << "Finish checkFile." << std::endl;

    if (file.is_open())
    {
        while (getline(file,strline))
        {
            line++;
            if (strline != "")
            {
                if (prevline == "{" && !ifDefIndent)
                {
                    ind = indent(strline);
                    ifDefIndent = true;
                }
                if (strline.find("{") != std::string::npos && !ifDefBrac)
                {
                    std::string ucStrline = uncoment(strline);
                    int len = ucStrline.size();
                    std::string ucUsbkStrline = unspaceBackward(ucStrline);
                    if (ucUsbkStrline[len - 1] == '{')
                    {
                        if(ucUsbkStrline.find("(") != std::string::npos)
                        {
                            brac = true;
                        }
                    }
                    ifDefBrac = true;
                    //std::cout << "EEEEEE brac def " << ifDefBrac << " " << brac << " " << line << std::endl;
                }

                if (strline != "")
                {
                    //checkOneLine(strline, line);
                    checkTwoLine(strline, prevline, line);
                }
                prevline = strline;
            }
        }
        file.close();
        std::cout << "Finish check" << std::endl;
    }
    else
    {
        std::cout << "Can't open file. " << std::endl;
    }
}
*/
