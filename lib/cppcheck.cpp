/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cppcheck.h"

#include "preprocessor.h" // Preprocessor
#include "tokenize.h" // Tokenizer

#include "check.h"
#include "path.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "timer.h"
#include "version.h"

#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <ctype.h>

#ifdef HAVE_RULES
#define PCRE_STATIC
#include <pcre.h>
#endif

static const char Version[] = CPPCHECK_VERSION_STRING;
static const char ExtraVersion[] = "";

static TimerResults S_timerResults;


bool ifDefBrac = false; //включен, если до этого уже просканили хотя бы одни кавычки
bool brac = false; //включен, если кавычки остаются в той же строке

bool ifDefIndent;  //включен, если до этого уже просканили хотя бы один отступ
int ind; //количество пробелов в отступе

void process(std::string filename);

CppCheck::CppCheck(ErrorLogger &errorLogger, bool useGlobalSuppressions)
    : _errorLogger(errorLogger), exitcode(0), _useGlobalSuppressions(useGlobalSuppressions), tooManyConfigs(false), _simplify(true)
{
}

CppCheck::~CppCheck()
{
    while (!fileInfo.empty()) {
        delete fileInfo.back();
        fileInfo.pop_back();
    }
    S_timerResults.ShowResults(_settings.showtime);
}

const char * CppCheck::version()
{
    return Version;
}

const char * CppCheck::extraVersion()
{
    return ExtraVersion;
}

unsigned int CppCheck::check(const std::string &path)
{
    std::ifstream fin(path.c_str());
    return processFile(path, fin);
}

unsigned int CppCheck::check(const std::string &path, const std::string &content)
{
    std::istringstream iss(content);
    return processFile(path, iss);
}

unsigned int CppCheck::processFile(const std::string& filename, std::istream& fileStream)
{
    exitcode = 0;

    // only show debug warnings for accepted C/C++ source files
    if (!Path::acceptFile(filename))
        _settings.debugwarnings = false;

    if (_settings.terminated())
        return exitcode;

    if (_settings.quiet == false) {
        std::string fixedpath = Path::simplifyPath(filename);
        fixedpath = Path::toNativeSeparators(fixedpath);
        _errorLogger.reportOut(std::string("Checking ") + fixedpath + std::string("..."));
    }

    bool internalErrorFound(false);
    try {


        Preprocessor preprocessor(_settings, this);
        std::list<std::string> configurations;
        std::string filedata;

        {
            Timer t("Preprocessor::preprocess", _settings.showtime, &S_timerResults);
            preprocessor.preprocess(fileStream, filedata, configurations, filename, _settings.includePaths);
        }

        if (_settings.checkConfiguration) {
            return 0;
        }

        // Run define rules on raw code
        for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
            if (it->tokenlist == "define") {
                Tokenizer tokenizer2(&_settings, this);
                std::istringstream istr2(filedata);
                tokenizer2.list.createTokens(istr2, filename);

                for (const Token *tok = tokenizer2.list.front(); tok; tok = tok->next()) {
                    if (tok->str() == "#define") {
                        std::string code = std::string(tok->linenr()-1U, '\n');
                        for (const Token *tok2 = tok; tok2 && tok2->linenr() == tok->linenr(); tok2 = tok2->next())
                            code += " " + tok2->str();
                        Tokenizer tokenizer3(&_settings, this);
                        std::istringstream istr3(code);
                        tokenizer3.list.createTokens(istr3, tokenizer2.list.file(tok));
                        executeRules("define", tokenizer3);
                    }
                }
                break;
            }
        }

        if (!_settings.userDefines.empty() && _settings.maxConfigs==1U) {
            configurations.clear();
            configurations.push_back(_settings.userDefines);
        }

        if (!_settings.force && configurations.size() > _settings.maxConfigs) {
            if (_settings.isEnabled("information")) {
                tooManyConfigsError(Path::toNativeSeparators(filename),configurations.size());
            } else {
                tooManyConfigs = true;
            }
        }

        // write dump file xml prolog
        std::ofstream fdump;
        if (_settings.dump) {
            const std::string dumpfile(filename + ".dump");
            fdump.open(dumpfile.c_str());
            if (fdump.is_open()) {
                fdump << "<?xml version=\"1.0\"?>" << std::endl;
                fdump << "<dumps>" << std::endl;
            }
        }

        std::set<unsigned long long> checksums;
        unsigned int checkCount = 0;
        for (std::list<std::string>::const_iterator it = configurations.begin(); it != configurations.end(); ++it) {
            // bail out if terminated
            if (_settings.terminated())
                break;

            // Check only a few configurations (default 12), after that bail out, unless --force
            // was used.
            if (!_settings.force && ++checkCount > _settings.maxConfigs)
                break;

            cfg = *it;

            // If only errors are printed, print filename after the check
            if (_settings.quiet == false && it != configurations.begin()) {
                std::string fixedpath = Path::simplifyPath(filename);
                fixedpath = Path::toNativeSeparators(fixedpath);
                _errorLogger.reportOut("Checking " + fixedpath + ": " + cfg + "...");
            }

            if (!_settings.userDefines.empty()) {
                if (!cfg.empty())
                    cfg = ";" + cfg;
                cfg = _settings.userDefines + cfg;
            }

            Timer t("Preprocessor::getcode", _settings.showtime, &S_timerResults);
            std::string codeWithoutCfg = preprocessor.getcode(filedata, cfg, filename);
            t.Stop();

            codeWithoutCfg += _settings.append();

            if (_settings.preprocessOnly) {
                if (codeWithoutCfg.compare(0,5,"#file") == 0)
                    codeWithoutCfg.insert(0U, "//");
                std::string::size_type pos = 0;
                while ((pos = codeWithoutCfg.find("\n#file",pos)) != std::string::npos)
                    codeWithoutCfg.insert(pos+1U, "//");
                pos = 0;
                while ((pos = codeWithoutCfg.find("\n#endfile",pos)) != std::string::npos)
                    codeWithoutCfg.insert(pos+1U, "//");
                pos = 0;
                while ((pos = codeWithoutCfg.find(Preprocessor::macroChar,pos)) != std::string::npos)
                    codeWithoutCfg[pos] = ' ';
                reportOut(codeWithoutCfg);
                continue;
            }

            std::cout << "MMMMMMMMMMMMMMMMMMMMMMM process" << std::endl;
            process(filename);

            Tokenizer _tokenizer(&_settings, this);
            if (_settings.showtime != SHOWTIME_NONE)
                _tokenizer.setTimerResults(&S_timerResults);

            try {
                // Create tokens, skip rest of iteration if failed
                std::istringstream istr(codeWithoutCfg);
                Timer timer("Tokenizer::createTokens", _settings.showtime, &S_timerResults);
                bool result = _tokenizer.createTokens(istr, filename.c_str());
                timer.Stop();
                if (!result)
                    continue;

                // skip rest of iteration if just checking configuration
                if (_settings.checkConfiguration)
                    continue;

                // Check raw tokens
                checkRawTokens(_tokenizer);

                // Simplify tokens into normal form, skip rest of iteration if failed
                Timer timer2("Tokenizer::simplifyTokens1", _settings.showtime, &S_timerResults);
                result = _tokenizer.simplifyTokens1(cfg);
                timer2.Stop();
                if (!result)
                    continue;

                // dump xml if --dump
                if (_settings.dump && fdump.is_open()) {
                    fdump << "<dump cfg=\"" << cfg << "\">" << std::endl;
                    preprocessor.dump(fdump);
                    _tokenizer.dump(fdump);
                    fdump << "</dump>" << std::endl;
                }

                // Skip if we already met the same simplified token list
                if (_settings.force || _settings.maxConfigs > 1) {
                    const unsigned long long checksum = _tokenizer.list.calculateChecksum();
                    if (checksums.find(checksum) != checksums.end())
                        continue;
                    checksums.insert(checksum);
                }

                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc("KIRILL.cpp", 1);
                locationList.push_back(loc);
                loc.setfile(_tokenizer.list.getSourceFilePath());
                //std::string fixedpath = Path::simplifyPath(filename);
                //loc.setfile(fixedpath);
                const ErrorLogger::ErrorMessage errmsgKirill(locationList,
                                                             Severity::error,
                                                             "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",
                                                             "0",
                                                             false);
                reportErr(errmsgKirill);
                // Check normal tokens
                std::cout << "MMMMMMMMMMMMMMMMMMMMM checkNormalTokens" << std::endl;
                checkNormalTokens(_tokenizer);

                // simplify more if required, skip rest of iteration if failed
                if (_simplify) {
                    // if further simplification fails then skip rest of iteration
                    Timer timer3("Tokenizer::simplifyTokenList2", _settings.showtime, &S_timerResults);
                    result = _tokenizer.simplifyTokenList2();
                    timer3.Stop();
                    if (!result)
                        continue;

                    // Check simplified tokens
                    std::cout << "MMMMMMMMMMMMMMMMMMMMM checkSimplifiedTokens" << std::endl;
                    checkSimplifiedTokens(_tokenizer);
                }

            } catch (const InternalError &e) {
                if (_settings.isEnabled("information") && (_settings.debug || _settings.verbose))
                    purgedConfigurationMessage(filename, cfg);
                internalErrorFound=true;
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                if (e.token) {
                    loc.line = e.token->linenr();
                    const std::string fixedpath = Path::toNativeSeparators(_tokenizer.list.file(e.token));
                    loc.setfile(fixedpath);
                } else {
                    ErrorLogger::ErrorMessage::FileLocation loc2;
                    loc2.setfile(Path::toNativeSeparators(filename.c_str()));
                    locationList.push_back(loc2);
                    loc.setfile(_tokenizer.list.getSourceFilePath());
                }
                locationList.push_back(loc);
                const ErrorLogger::ErrorMessage errmsg(locationList,
                                                       Severity::error,
                                                       e.errorMessage,
                                                       e.id,
                                                       false);

                reportErr(errmsg);
            }
        }

        // dumped all configs, close root </dumps> element now
        if (_settings.dump && fdump.is_open())
            fdump << "</dumps>" << std::endl;

    } catch (const std::runtime_error &e) {
        internalError(filename, e.what());
    } catch (const InternalError &e) {
        internalError(filename, e.errorMessage);
        exitcode=1; // e.g. reflect a syntax error
    }

    // In jointSuppressionReport mode, unmatched suppressions are
    // collected after all files are processed
    if (!_settings.jointSuppressionReport && (_settings.isEnabled("information") || _settings.checkConfiguration)) {
        reportUnmatchedSuppressions(_settings.nomsg.getUnmatchedLocalSuppressions(filename, unusedFunctionCheckIsEnabled()));
    }

    _errorList.clear();
    if (internalErrorFound && (exitcode==0)) {
        exitcode=1;
    }
    return exitcode;
}

void CppCheck::internalError(const std::string &filename, const std::string &msg)
{
    const std::string fixedpath = Path::toNativeSeparators(filename);
    const std::string fullmsg("Bailing out from checking " + fixedpath + " since there was an internal error: " + msg);

    if (_settings.isEnabled("information")) {
        const ErrorLogger::ErrorMessage::FileLocation loc1(filename, 0);
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        callstack.push_back(loc1);

        ErrorLogger::ErrorMessage errmsg(callstack,
                                         Severity::information,
                                         fullmsg,
                                         "internalError",
                                         false);

        _errorLogger.reportErr(errmsg);

    } else {
        // Report on stdout
        _errorLogger.reportOut(fullmsg);
    }
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a raw token list
//---------------------------------------------------------------------------
void CppCheck::checkRawTokens(const Tokenizer &tokenizer)
{
    // Execute rules for "raw" code
    executeRules("raw", tokenizer);
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a normal token list
//---------------------------------------------------------------------------

void CppCheck::checkNormalTokens(const Tokenizer &tokenizer)
{
    // call all "runChecks" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        if (_settings.terminated())
            return;

        Timer timerRunChecks((*it)->name() + "::runChecks", _settings.showtime, &S_timerResults);
        (*it)->runChecks(&tokenizer, &_settings, this);
    }

    // Analyse the tokens..
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        Check::FileInfo *fi = (*it)->getFileInfo(&tokenizer, &_settings);
        if (fi != nullptr)
            fileInfo.push_back(fi);
    }

    executeRules("normal", tokenizer);
}

//---------------------------------------------------------------------------
// CppCheck - A function that checks a simplified token list
//---------------------------------------------------------------------------

void CppCheck::checkSimplifiedTokens(const Tokenizer &tokenizer)
{
    // call all "runSimplifiedChecks" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it) {
        if (_settings.terminated())
            return;

        Timer timerSimpleChecks((*it)->name() + "::runSimplifiedChecks", _settings.showtime, &S_timerResults);
        (*it)->runSimplifiedChecks(&tokenizer, &_settings, this);
        timerSimpleChecks.Stop();
    }

    if (!_settings.terminated())
        executeRules("simple", tokenizer);
}

void CppCheck::executeRules(const std::string &tokenlist, const Tokenizer &tokenizer)
{
    (void)tokenlist;
    (void)tokenizer;

#ifdef HAVE_RULES
    // Are there rules to execute?
    bool isrule = false;
    for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
        if (it->tokenlist == tokenlist)
            isrule = true;
    }

    // There is no rule to execute
    if (isrule == false)
        return;

    // Write all tokens in a string that can be parsed by pcre
    std::ostringstream ostr;
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
        ostr << " " << tok->str();
    const std::string str(ostr.str());

    for (std::list<Settings::Rule>::const_iterator it = _settings.rules.begin(); it != _settings.rules.end(); ++it) {
        const Settings::Rule &rule = *it;
        if (rule.pattern.empty() || rule.id.empty() || rule.severity == Severity::none || rule.tokenlist != tokenlist)
            continue;

        const char *error = nullptr;
        int erroffset = 0;
        pcre *re = pcre_compile(rule.pattern.c_str(),0,&error,&erroffset,nullptr);
        if (!re) {
            if (error) {
                ErrorLogger::ErrorMessage errmsg(std::list<ErrorLogger::ErrorMessage::FileLocation>(),
                                                 Severity::error,
                                                 error,
                                                 "pcre_compile",
                                                 false);

                reportErr(errmsg);
            }
            continue;
        }

        int pos = 0;
        int ovector[30]= {0};
        while (pos < (int)str.size() && 0 <= pcre_exec(re, nullptr, str.c_str(), (int)str.size(), pos, 0, ovector, 30)) {
            const unsigned int pos1 = (unsigned int)ovector[0];
            const unsigned int pos2 = (unsigned int)ovector[1];

            // jump to the end of the match for the next pcre_exec
            pos = (int)pos2;

            // determine location..
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.setfile(tokenizer.list.getSourceFilePath());
            loc.line = 0;

            std::size_t len = 0;
            for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
                len = len + 1U + tok->str().size();
                if (len > pos1) {
                    loc.setfile(tokenizer.list.getFiles().at(tok->fileIndex()));
                    loc.line = tok->linenr();
                    break;
                }
            }

            const std::list<ErrorLogger::ErrorMessage::FileLocation> callStack(1, loc);

            // Create error message
            std::string summary;
            if (rule.summary.empty())
                summary = "found '" + str.substr(pos1, pos2 - pos1) + "'";
            else
                summary = rule.summary;
            const ErrorLogger::ErrorMessage errmsg(callStack, rule.severity, summary, rule.id, false);

            // Report error
            reportErr(errmsg);
        }

        pcre_free(re);
    }
#endif
}

Settings &CppCheck::settings()
{
    return _settings;
}

void CppCheck::tooManyConfigsError(const std::string &file, const std::size_t numberOfConfigurations)
{
    if (!_settings.isEnabled("information") && !tooManyConfigs)
        return;

    tooManyConfigs = false;

    if (_settings.isEnabled("information") && file.empty())
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        ErrorLogger::ErrorMessage::FileLocation location;
        location.setfile(file);
        loclist.push_back(location);
    }

    std::ostringstream msg;
    msg << "Too many #ifdef configurations - cppcheck only checks " << _settings.maxConfigs;
    if (numberOfConfigurations > _settings.maxConfigs)
        msg << " of " << numberOfConfigurations << " configurations. Use --force to check all configurations.\n";
    if (file.empty())
        msg << " configurations. Use --force to check all configurations. For more details, use --enable=information.\n";
    msg << "The checking of the file will be interrupted because there are too many "
           "#ifdef configurations. Checking of all #ifdef configurations can be forced "
           "by --force command line option or from GUI preferences. However that may "
           "increase the checking time.";
    if (file.empty())
        msg << " For more details, use --enable=information.";


    ErrorLogger::ErrorMessage errmsg(loclist,
                                     Severity::information,
                                     msg.str(),
                                     "toomanyconfigs",
                                     false);

    reportErr(errmsg);
}

void CppCheck::purgedConfigurationMessage(const std::string &file, const std::string& configuration)
{

    tooManyConfigs = false;

    if (_settings.isEnabled("information") && file.empty())
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> loclist;
    if (!file.empty()) {
        ErrorLogger::ErrorMessage::FileLocation location;
        location.setfile(file);
        loclist.push_back(location);
    }

    ErrorLogger::ErrorMessage errmsg(loclist,
                                     Severity::information,
                                     "The configuration '" + configuration + "' was not checked because its code equals another one.",
                                     "purgedConfiguration",
                                     false);

    reportErr(errmsg);
}

//---------------------------------------------------------------------------

void CppCheck::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    if (!_settings.library.reportErrors(msg.file0))
        return;

    const std::string errmsg = msg.toString(_settings.verbose);
    if (errmsg.empty())
        return;

    // Alert only about unique errors
    if (std::find(_errorList.begin(), _errorList.end(), errmsg) != _errorList.end())
        return;

    std::string file;
    unsigned int line(0);
    if (!msg._callStack.empty()) {
        file = msg._callStack.back().getfile(false);
        line = msg._callStack.back().line;
    }

    if (_useGlobalSuppressions) {
        if (_settings.nomsg.isSuppressed(msg._id, file, line))
            return;
    } else {
        if (_settings.nomsg.isSuppressedLocal(msg._id, file, line))
            return;
    }

    if (!_settings.nofail.isSuppressed(msg._id, file, line))
        exitcode = 1;

    _errorList.push_back(errmsg);

    _errorLogger.reportErr(msg);
}

void CppCheck::reportOut(const std::string &outmsg)
{
    _errorLogger.reportOut(outmsg);
}

void CppCheck::reportProgress(const std::string &filename, const char stage[], const std::size_t value)
{
    _errorLogger.reportProgress(filename, stage, value);
}

void CppCheck::reportInfo(const ErrorLogger::ErrorMessage &msg)
{
    // Suppressing info message?
    std::string file;
    unsigned int line(0);
    if (!msg._callStack.empty()) {
        file = msg._callStack.back().getfile(false);
        line = msg._callStack.back().line;
    }
    if (_useGlobalSuppressions) {
        if (_settings.nomsg.isSuppressed(msg._id, file, line))
            return;
    } else {
        if (_settings.nomsg.isSuppressedLocal(msg._id, file, line))
            return;
    }

    _errorLogger.reportInfo(msg);
}

void CppCheck::reportStatus(unsigned int /*fileindex*/, unsigned int /*filecount*/, std::size_t /*sizedone*/, std::size_t /*sizetotal*/)
{

}

void CppCheck::getErrorMessages()
{
    Settings s(_settings);
    s.addEnabled("warning");
    s.addEnabled("style");
    s.addEnabled("portability");
    s.addEnabled("performance");
    s.addEnabled("information");

    tooManyConfigs = true;
    tooManyConfigsError("",0U);

    // call all "getErrorMessages" in all registered Check classes
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
        (*it)->getErrorMessages(this, &s);

    Preprocessor::getErrorMessages(this, &s);
}

void CppCheck::analyseWholeProgram()
{
    // Analyse the tokens
    for (std::list<Check *>::const_iterator it = Check::instances().begin(); it != Check::instances().end(); ++it)
        (*it)->analyseWholeProgram(fileInfo, _settings, *this);
}

bool CppCheck::unusedFunctionCheckIsEnabled() const
{
    return (_settings.jobs == 1 && _settings.isEnabled("unusedFunction"));
}


////////////////// MY CODE //////////////////////////
/// \brief uncoment
/// \param line
/// \return
///
///
/// ////////////////////////////////////////////////


using namespace std;

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
    /*QFile file("log.txt");
    if (file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << QString::fromStdString(error) << endl;
        file.close();
    }
    else
    {
        std::cout << "can't open log.txt to log" << std::endl;
    }*/
    ofstream fout("log.txt"); // создаём объект класса ofstream для записи и связываем его с файлом cppstudio.txt
    fout << error; // запись строки в файл
    fout.close(); // закрываем файл

    /*std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc("KIRILL.cpp", 1);
    locationList.push_back(loc);
    //loc.setfile(_tokenizer.list.getSourceFilePath());
    std::string fixedpath = Path::simplifyPath("");
    loc.setfile(fixedpath);
    const ErrorLogger::ErrorMessage errmsgKirill(locationList,
                                                 Severity::error,
                                                 "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",
                                                 "0",
                                                 false);
    reportErr(errmsgKirill);*/
}

/*std::string checkUtf8File(string file_name)
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
        file.close();
        return ss.str();
    }
    file.close();
    return "";
}
std::string checkFileName(string file_name)
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
            return ss.str();
        }
    }
    return "";
}*/
std::string checkComment(string file_name)
{
    if  (file_name.find(".h") != string::npos || file_name.find(".hpp") != string::npos)
    {
        ifstream file(file_name.c_str());
        string strline;
        if (file.is_open())
        {
            getline(file,strline);
            //std::cout << "EEEEEEEE: " << strline << " " << strline.size() << std::endl;

            if (strline.find("/*") == string::npos && strline.find("//") == string::npos)
            {
                std::stringstream ss("");
                ss << "must add comment to introduce  file";
                cout << ss.str() << endl;
                printError(ss.str());
                file.close();
                return ss.str();
            }
            else if (strline.size() < 10)
            {
                std::stringstream ss("");
                ss << "must add a bigger comment to introduce  file";
                cout << ss.str() << endl;
                printError(ss.str());
                file.close();
                return ss.str();
            }
            file.close();
        }
    }
    return "";
}
std::string checkMultipleInclude(string file_name)
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
            return ss.str();
        }
    }
    return "";
}
std::vector<std::string> CppCheck::checkFile(string file_name)
{
    std::vector<std::string> ervc;
    std::string erst = "";
    /*checkUtf8File(file_name);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    //checkFileName(file_name);
    if (erst != "")
    {
        ervc.push_back(erst);
    }*/
    checkComment(file_name);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    checkMultipleInclude(file_name);
    if (erst != "")
    {
        ervc.push_back(erst);
    }

    return ervc;
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
std::string checkIndent(string strline, string prevline, int line)
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
                return ss.str();
            }
        }
        else if(strline[fst] == '}')
        {
            //std::cout << "prevline: " << prevline << " " << indent(prevline) << std::endl;
            //std::cout << "strline: " << strline << " " << indent(strline) << std::endl << std::endl;
            if (indent(strline) != (indent(pvln)-ind))
            {
                std::stringstream ss("");
                ss << "Неровное количество отступов в } строке " << line;
                cout << ss.str() << endl;
                printError(ss.str());
                return ss.str();
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
                return ss.str();
            }
        }
    }
    return "";
}
std::string checkFuncIndent(string strline, string prevline, int line)
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
            return ss.str();
        }
    }
    return "";
}
std::vector<std::string> CppCheck::checkTwoLine(string strline, string prevline, int line)
{
    std::vector<std::string> ervc;
    std::string erst = "";
    checkIndent(strline, prevline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    checkFuncIndent(strline, prevline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }

    return ervc;
}



std::string checkLong(string strline, int line)
{
    if (strline.size() > 120)
    {
        std::stringstream ss("");
        ss << "line " << line << " is more than 120 character";
        cout << ss.str() << endl;
        printError(ss.str());
        return ss.str();
    }
    else return "";
}
std::string checkOneOperator(string strline, int line)
{
    string str = strline;//uncoment(strline);
    size_t pos = 0;
    if ((pos = str.find(";")) != string::npos)
    {
        str.erase(pos, 1);
    }


    if (str.find("for") == string::npos)
    {
        if (str.find(";") !=  string::npos)
        {
            std::stringstream ss("");
            ss << "more then 1 operator in line " << line;
            cout << ss.str() << endl;
            printError(ss.str());
            return ss.str();
        }
    }
    else
    {
        size_t pos1 = 0;
        if ((pos1 = str.find(";")) != string::npos)
        {
            str.erase(pos1, 1);
            size_t pos2 = 0;
            if ((pos2 = str.find(";")) != string::npos)
            {
                str.erase(pos2, 1);

                std::stringstream ss("");
                ss << "more then 1 operator in line " << line;
                cout << ss.str() << endl;
                printError(ss.str());
                return ss.str();
            }
        }
    }
    return "";
}
std::string checkConstantUpRegister(string strline, int line)
{
    std::vector <char*> vec;
    char *slovo;

    char *writable = new char[strline.size() + 1];
    std::copy(strline.begin(), strline.end(), writable);
    writable[strline.size()] = '\0'; // don't forget the terminating 0

    // разделяем строку на слова
    slovo=strtok(writable," ");
    while(slovo != NULL)
    {
        vec.push_back(slovo);
        slovo=strtok(NULL," ");
    }

    string str(vec[0]);
    if(str == "#define")
    {
        for(unsigned int i =0; i < strlen(vec[1]); i++)
        {
            if( (vec[1][i] != toupper(vec[1][i])) && isalpha(vec[1][i]) )
            {
                std::stringstream ss("");
                ss << "constant must be in upper register " << line << " line";
                cout << ss.str() << endl;
                printError(ss.str());
                // don't forget to free the string after finished using it
                delete[] writable;
                return ss.str();
            }
        }
    }

    // don't forget to free the string after finished using it
    delete[] writable;
    return "";
}
std::string checkInclude(string strline, int line)
{
    std::vector <char*> vec;
    char *slovo;

    char *writable = new char[strline.size() + 1];
    std::copy(strline.begin(), strline.end(), writable);
    writable[strline.size()] = '\0'; // don't forget the terminating 0

    // разделяем строку на слова
    slovo=strtok(writable," ");
    while(slovo != NULL)
    {
        vec.push_back(slovo);
        slovo=strtok(NULL," ");
    }

    string incStr = vec[0];
    if(incStr == "#include")
    {
        string str(vec[1]);
        size_t pos = str.find(".");
        size_t pos1 = str.find(".h");
        size_t pos2 = str.find(".hpp");
        if((pos != string::npos) && pos1 == string::npos && pos2 == string::npos)
        {
            std::stringstream ss("");
            ss << "include only *.h and *.hpp file " << line << " line";
            cout << ss.str() << endl;
            printError(ss.str());
            return ss.str();
        }
    }

    // don't forget to free the string after finished using it
    delete[] writable;

    return "";
}
std::string checkFloat(string strline, int line)
{
    std::vector <char*> vec;
    char *slovo;

    char *writable = new char[strline.size() + 1];
    std::copy(strline.begin(), strline.end(), writable);
    writable[strline.size()] = '\0'; // don't forget the terminating 0

    // разделяем строку на слова
    slovo=strtok(writable," ");
    while(slovo != NULL)
    {
        vec.push_back(slovo);
        slovo=strtok(NULL," ");
    }

    for (unsigned int i = 0; i < vec.size(); i++)
    {
        if (string(vec[i]) == "float")
        {
            std::stringstream ss("");
            ss << "You must use double type in line " << line;
            cout << ss.str() << endl;
            printError(ss.str());
            // don't forget to free the string after finished using it
            delete[] writable;
            return ss.str();
        }
    }

    // don't forget to free the string after finished using it
    delete[] writable;
    return "";
}
void checkSpaceOperator(string strline, int line);
std::string checkPragmaPack(string strline, int line)
{
    size_t pos = 0;
    if ((pos = strline.find("#pragma pack")) != std::string::npos)
    {
        std::stringstream ss("");
        ss << "You do not need to use pragma pack " << line;
        cout << ss.str() << endl;
        printError(ss.str());
        return ss.str();
    }
    return "";
}
std::string checkBrackets(string strline, int line)
{
    if (ifDefBrac && strline.find("{") != std::string::npos)
    {
        //std::cout << "EEEEEE brac def " << ifDefBrac << " " << brac << " " << line << std::endl;
        if (strline.find("(") != std::string::npos && !brac)
        {
            std::stringstream ss("");
            ss << "You must use the first type of brackets " << line;
            cout << ss.str() << endl;
            printError(ss.str());
            return  ss.str();
        }
        if (strline.find("(") == std::string::npos && brac)
        {
            std::stringstream ss("");
            ss << "You must use the second type of brackets " << line;
            cout << ss.str() << endl;
            printError(ss.str());
            return ss.str();
        }
    }
    return "";
}
std::string checkInt(string strline, int line)
{
    std::vector <char*> vec;
    char *slovo;

    char *writable = new char[strline.size() + 1];
    std::copy(strline.begin(), strline.end(), writable);
    writable[strline.size()] = '\0'; // don't forget the terminating 0

    // разделяем строку на слова
    slovo=strtok(writable," ");
    while(slovo != NULL)
    {
        vec.push_back(slovo);
        slovo=strtok(NULL," ");
    }

    for (unsigned int i = 0; i < vec.size(); i++)
    {
        if (string(vec[i]) == "int")
        {
            std::stringstream ss("");
            ss << "You must specify int size in line " << line;
            cout << ss.str() << endl;
            printError(ss.str());
            // don't forget to free the string after finished using it
            delete[] writable;
            return ss.str();
        }
    }

    // don't forget to free the string after finished using it
    delete[] writable;
    return "";
}
std::string checkConstantSuff(string strline, int line)
{
    std::vector <char*> vec;
    char *slovo;

    char *writable = new char[strline.size() + 1];
    std::copy(strline.begin(), strline.end(), writable);
    writable[strline.size()] = '\0'; // don't forget the terminating 0

    // разделяем строку на слова
    slovo=strtok(writable," ");
    while(slovo != NULL)
    {
        vec.push_back(slovo);
        slovo=strtok(NULL," ");
    }

    for (unsigned int i = 0; i < vec.size(); i++)
    {
        if (string(vec[i]) == "const")
        {
            if (std::string(vec[i+1]).find("int") != std::string::npos || std::string(vec[i+1]).find("double") != std::string::npos)
            {
                char *constant = vec[i + 4];
                int len = strlen(constant);
                for(unsigned int i =0; i < len; i++)
                {
                    if (isalpha(constant[i]) && constant[i] != toupper(constant[i]))
                    {
                        std::stringstream ss("");
                        ss << "constant suffix must be in upper register! " << constant[i];
                        cout << ss.str() << endl;
                        printError(ss.str());
                        // don't forget to free the string after finished using it
                        delete[] writable;
                        return ss.str();
                    }
                }
            }
        }
    }

    // don't forget to free the string after finished using it
    delete[] writable;
    return "";
}
std::vector<std::string> CppCheck::checkOneLine(string strline, int line)
{
    std::vector<std::string> ervc;
    std::string erst = "";
    erst = checkLong(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    erst = checkOneOperator(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    /*erst = checkConstantUpRegister(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }*/
    /*erst = checkInclude(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }*/
    erst = checkFloat(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    /*erst = checkSpaceOperator(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }*/
    erst = checkPragmaPack(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    erst = checkBrackets(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    erst = checkInt(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }
    checkConstantSuff(strline, line);
    if (erst != "")
    {
        ervc.push_back(erst);
    }

    return ervc;
}


void checkSpaceOperator(string strline, int line)
{
    for (unsigned int i = 0; i < strline.size(); i++)
    {
        if(strline[i] == '>')
        {
            if(strline[i+1] == '>')
            {
                if ( (strline[i-1] != ' ') || (strline[i+2] != ' ') )
                {
                    std::stringstream ss("");
                    ss << "must you space between operator " << line;
                    cout << ss.str() << endl;
                    printError(ss.str());
                }
            }
        }
        if(strline[i] == '<')
        {
            if(strline[i+1] == '<')
            {
                if ( (strline[i-1] != ' ') || (strline[i+2] != ' ') )
                {
                    std::stringstream ss("");
                    ss << "must you space between operator " << line;
                    cout << ss.str() << endl;
                    printError(ss.str());
                }
            }
        }
        if(strline[i] == '&')
        {
            if(strline[i+1] == '&')
            {
                if ( (strline[i-1] != ' ') || (strline[i+2] != ' ') )
                {
                    std::stringstream ss("");
                    ss << "must you space between operator " << line;
                    cout << ss.str() << endl;
                    printError(ss.str());
                }
            }
        }
        if(strline[i] == '|')
        {
            if(strline[i+1] == '|')
            {
                if ( (strline[i-1] != ' ') || (strline[i+2] != ' ') )
                {
                    std::stringstream ss("");
                    ss << "must you space between operator " << line;
                    cout << ss.str() << endl;
                    printError(ss.str());
                }
            }
        }
        if(strline[i] == '=')
        {
            if(strline[i+1] == '=')
            {
                if ( (strline[i-1] != ' ') || (strline[i+2] != ' ') )
                {
                    std::stringstream ss("");
                    ss << "must you space between operator " << line;
                    cout << ss.str() << endl;
                    printError(ss.str());
                }
            }
        }
        if(strline[i] == '!')
        {
            if(strline[i+1] == '=')
            {
                if ( (strline[i-1] != ' ') || (strline[i+2] != ' ') )
                {
                    std::stringstream ss("");
                    ss << "must you space between operator " << line;
                    cout << ss.str() << endl;
                    printError(ss.str());
                }
            }
        }
    }
}








void CppCheck::process(std::string filename)
{
    //std::cout << "File: " << filename << std::endl;
    std::string strline = "", prevline = "", nextline = "";

    std::ifstream file(filename.c_str());
    unsigned int line = 0;

    std::vector<std::string> vc = checkFile(filename);

    for (unsigned int i=0; i < vc.size(); i++)
    {
        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
        ErrorLogger::ErrorMessage::FileLocation loc(filename, 0);
        locationList.push_back(loc);
        //loc.setfile(_tokenizer.list.getSourceFilePath());
        std::string fixedpath = Path::simplifyPath("");
        loc.setfile(fixedpath);
        const ErrorLogger::ErrorMessage errmsg(locationList,
                                               Severity::error,
                                               vc[i],
                                               "0",
                                               false);
        reportErr(errmsg);
    }
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
                    vc = checkOneLine(strline, line);
                    //std::cout << "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM   " << vc.size() << std::endl;
                    for (unsigned int i=0; i < vc.size(); i++)
                    {
                        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                        ErrorLogger::ErrorMessage::FileLocation loc(filename, line);
                        locationList.push_back(loc);
                        //loc.setfile(_tokenizer.list.getSourceFilePath());
                        std::string fixedpath = Path::simplifyPath("");
                        loc.setfile(fixedpath);
                        const ErrorLogger::ErrorMessage errmsg(locationList,
                                                               Severity::performance,
                                                               vc[i],
                                                               "0",
                                                               false);
                        reportErr(errmsg);
                    }
                    /*vc = checkTwoLine(strline, prevline, line);
                    //std::cout << "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK   " << vc.size() << std::endl;
                    for (unsigned int i=0; i < vc.size(); i++)
                    {
                        std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                        ErrorLogger::ErrorMessage::FileLocation loc(filename, line);
                        locationList.push_back(loc);
                        //loc.setfile(_tokenizer.list.getSourceFilePath());
                        std::string fixedpath = Path::simplifyPath("");
                        loc.setfile(fixedpath);
                        const ErrorLogger::ErrorMessage errmsg(locationList,
                                                               Severity::error,
                                                               vc[i],
                                                               "0",
                                                               false);
                        reportErr(errmsg);
                    }*/
                }
                prevline = strline;
            }
        }
        file.close();
        std::cout << "Finish process" << std::endl;
    }
    else
    {
        std::cout << "Can't open file. " << std::endl;
    }
}
