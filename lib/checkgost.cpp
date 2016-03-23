#include "iostream"

#include "checkgost.h"
#include "symboldatabase.h"
#include "tokenize.h"
#include "string"
//#include "cxx11emu.h"

#include <cstring>
#include <cctype>

namespace {
CheckGost instance;
}

/*class AdapterScope  {
public:
    AdapterScope();

    std::list<AdapterScope *> nestedScope();
    void setScope(Scope scope) { _scope = scope; }

private:
    Scope _scope;
    bool _visited;
};

std::list<AdapterScope *> AdapterScope::nestedScope()
{
    std::list<AdapterScope *> list;
    std::list<Scope *> lst = _scope.nestedList;
    for (std::list<Scope *>::iterator it = lst.begin(); it != lst.end(); it++)
    {
        AdapterScope *adsp = new AdapterScope();
        Scope *scope = *it;
        adsp->setScope(*scope);
        list.push_back(adsp);
    }
    return list;
}*/

void CheckGost::gostCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        /*if (Token::Match(tok, "%type% ("))
        {
            const Function *func = tok->function();
            if (func && func­>isStatic())
            {
                //std::cout << func->name() << "    " << tok->linenr() << std::endl;
            }
        }*/
        if (Token::Match(tok, "%var%"))
        {
            std::cout << "var: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%assign%"))
        {
            std::cout << "assign: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%bool%"))
        {
            std::cout << "true: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%char%"))
        {
            std::cout << "char: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%comp%"))
        {
            std::cout << "comp: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%cop%"))
        {
            std::cout << "cop: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%name%"))
        {
            std::cout << "name: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%num%"))
        {
            std::cout << "num: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%op%"))
        {
            std::cout << "op: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%or%"))
        {
            std::cout << "or: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%oror%"))
        {
            std::cout << "oror: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%type%"))
        {
            std::cout << "type: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%str%"))
        {
            std::cout << "str: " << tok->str() << std::endl;
        }

        if (Token::Match(tok, "%varid%"))
        {
            std::cout << "varid: " << tok->str() << std::endl;
        }
    }
}

void CheckGost::indentifierCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        //std::cout << "QQQQQQQQQQQQQQQQQQQQQQQQQQQQ" << std::endl;
        if (Token::Match(tok, "%var%"))
        {
            std::string str = "";
            const Variable *var = tok->variable();
            if (var)
            {
                str = var->name();
                //std::cout << "QQQQQQQQQQQQQQQQQQQQQQQQQQQQ:  " << str << std::endl;
                if (str[0] == toupper(str[0]))
                {
                    std::cout << "identifier must be in lower rigester" << std::endl;
                    returnGostError(tok, "identifier must be in lower rigester");
                }
            }

            /*const Type *tp = var->type();
            std::string tpname = "";
            if (tp) tpname = tp->name();
            std::cout << "name: " << tpname << std::endl;*/
        }
    }
}

void CheckGost::customTypeCheck()
{
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        //std::cout << "EEEEEEEEEEEEEEE" << std::endl;
        //std::cout << tok->str() << " " << tok->isStandardType() << " " << tok->isKeyword() << std::endl;
        if (Token::Match(tok, "%type%"))
        {
            //std::cout << "KKKKKKKKKKKKKKK" << std::endl;
            const Token *tokPrev = tok->tokAt(-1);

            if(tokPrev)
            {
                std::string str = tokPrev->str();

                if (str == "сlass" || str == "enum" || str == "struct")
                {
                    if (tok->str()[0] != toupper(tok->str()[0]))
                    {
                        std::cout << "Custom type must start in upper register: " << tok->str() << std::endl;
                        std::string str = "Custom type must start in upper register: ";
                        str += tok->str();
                        returnGostError(tok, str);
                    }
                }
            }
        }
    }
}

void CheckGost::globalStaticCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (Token::Match(tok, "%var%"))
        {
            //std::cout << tok->str() << std::endl;

            const Variable *var = tok->variable();
            const std::string str = var->name();

            //std::cout << str << std::endl;

            if (var && var->isStatic())
            {
                if ( str[0] != 's' || str[1] != '_')
                {
                    std::cout << "error" << std::endl;
                    returnGostError(tok, "must you s_ prefix to static variable");
                }
            }
            if (var && var->isGlobal())
            {
                if ( str[0] != 'g' || str[1] != '_')
                {
                    std::cout << "error" << std::endl;
                    returnGostError(tok, "must you g_ prefix to static variable");
                }
            }
        }

    }
}

void CheckGost::literalCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->isLiteral())
        {
            std::string strTok = tok->str();
            if (isdigit(strTok[0]))
            {
                for (unsigned int i; i < strTok.size(); i++)
                {
                    if (isalpha(strTok[i]))
                    {
                        std::cout << "you must use upper case of literal suffix" << std::endl;
                        returnGostError(tok, "you must use upper case of literal suffix");
                    }
                }
            }
        }
    }
}

void CheckGost::structNameCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        std::string str = tok->str();
        //std::cout << str << std::endl;
        if (str =="struct")
        {
            //std::cout << "Struct!!!" << std::endl;
            const Scope *sp = tok->next()->next()->scope();
            //std::cout << "Next: " << tok->next()->next()->str() << std::endl;
            const Scope::ScopeType tp = sp->type;
            //std::cout << "Type: " << tp << std::endl;

            std::string first = getFirst(tok->next()->str());
            //std::cout << "First: " << first << std::endl;
            std::list<Variable> lst = sp->varlist;
            for (std::list<Variable>::iterator it = lst.begin(); it != lst.end(); it++)
            {
                //std::cout << it->name() << std::endl;
                std::string name = it->name();
                unsigned int i = 0;
                for (; i < first.size(); i++)
                {
                    if(first[i] != name[i])
                    {
                        std::cout << "Struct-member name must start like a struct name" << std::endl;
                        returnGostError(tok, "Struct-member name must start like a struct name");
                    }
                }
                if (name[i] != '_')
                {
                    std::cout << "Struct-member name must start like a struct name" << std::endl;
                    returnGostError(tok, "Struct-member name must start like a struct name");
                }
            }
        }
    }
}

std::string CheckGost::getFirst(std::string structName)
{
    std::string first = "";
    for (unsigned int i = 0;  i < structName.size(); i++)
    {
        if (structName[i] == toupper(structName[i]))
        {
            first += structName[i];
        }
    }

    return first;
}

void CheckGost::structMemberCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        std::string str = tok->str();
        //std::cout << str << std::endl;
        if (str =="struct")
        {
            //std::cout << "Struct!!!" << std::endl;
            //std::cout << "Type: " << tok->scope()->type << std::endl;
            //std::cout << "Type: " << tok->next()->scope()->type << std::endl;
            const Scope *sp = tok->next()->next()->scope();
            //std::cout << "Next: " << tok->next()->next()->str() << std::endl;
            const Scope::ScopeType tp = sp->type;
            //std::cout << "Type: " << tp << std::endl;

            std::list<Variable> lst = sp->varlist;
            for (std::list<Variable>::iterator it = lst.begin(); it != lst.end(); it++)
            {
                //std::cout << it->name() << std::endl;
                std::string name = it->name();

                if (it->isPointer() || it->isReference())
                {
                    std::cout << "Struct-member must not be pointer or reference" << std::endl;
                    returnGostError(tok, "Struct-member must not be pointer or reference");
                }
            }
        }
    }
}

void CheckGost::constructorDestructorCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Is there a function call here?
        if (Token::Match(tok, "%type% ("))
        {
            // Get function information about the called function
            const Function *function = tok->function();
            // Check if function is constructor..
            if (function && function->isVirtual())
            {
                const Scope *sp = tok->scope();
                const Scope::ScopeType tp = sp->type;
                //std::cout << "Type: " << tp << "     " << tok->previous()->str() << std::endl;

                const Token *tokDef = sp->classDef;
                if (tokDef)
                {
                    //std::cout << "TokDef: " << tokDef->str() << "    " << tokDef->next()->str() << "    " << sp->classStart->str() << std::endl;
                    if (Token::Match(tokDef, "%type% ("))
                    {
                        //std::cout << "EEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
                        const Function *func = tokDef->function();

                        if (func && func->isConstructor())
                        {
                            //std::cout << "Constructor: " << tokDef->str() << std::endl;
                            returnGostError(tok, "Constructor must be without virtual function");
                        }
                        // Check if function is destructor..
                        if (func && func->isDestructor())
                        {
                            //std::cout << "Destructor: " << tokDef->str() << std::endl;
                            returnGostError(tok, "Destructor must be without virtual function");
                        }
                    }
                }

            }
        }
    }
}

void CheckGost::switchCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Is there a switch call here?
        if (tok->str() == "switch")
        {
            //std::cout << "SWITCH EEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
            const Token *tokBrac = tok->next()->next()->next()->next();
            //std::cout << tokBrac->str() << std::endl;

            const Scope *sp = tokBrac->scope();
            const Scope::ScopeType tp = sp->type;
            //std::cout << "Type: " << tp << std::endl;

            const Token *token = sp->classStart;
            //std::cout << token->str() << "    " << sp->classEnd->str() << std::endl;
            bool defTok = false;
            while (token != sp->classEnd)
            {
                if (token->str() == "default")
                {
                    defTok = true;
                }
                token = token->next();
            }
            if (!defTok)
            {
                std::cout << "Switch operator must has a default section" << std::endl;
                returnGostError(tok, "Switch operator must has a default section");
            }
        }
    }
}

void CheckGost::qualCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "class")
        {
            const Token *token = tok->next()->next();
            const Scope *sp = token->scope();
            //std::cout << "Scope: " << sp->type << "    " << token->str() << std::endl;

            const Token *tokenStart = sp->classStart;
            //std::cout << tokenStart->str() << "    " << sp->classEnd->str() << std::endl;
            bool pubTok = false, protTok = false, privTok = false;
            while (tokenStart != sp->classEnd)
            {
                if (tokenStart->str() == "public" || tokenStart->str() == "public:")
                {
                    pubTok = true;
                    //std::cout << "public true" << std::endl;
                }

                if (tokenStart->str() == "protected" || tokenStart->str() == "protected:")
                {
                    protTok = true;
                    //std::cout << "protected true" << std::endl;
                }

                if (tokenStart->str() == "private" || tokenStart->str() == "private:")
                {
                    privTok = true;
                    //std::cout << "private true" << std::endl;
                }

                if (protTok && !pubTok)
                {
                    std::cout << "qualifer must be in right order!" << std::endl;
                    returnGostError(tok, "qualifer must be in right order!");
                }
                if (privTok && (!protTok || !pubTok))
                {
                    std::cout << "qualifer must be in right order!" << std::endl;
                    returnGostError(tok, "qualifer must be in right order!");
                }
                //std::cout << tokenStart->str() << std::endl;
                tokenStart = tokenStart->next();
            }
        }
    }
}

void CheckGost::nestedCheck()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        // Is there a function call here?
        if (Token::Match(tok, "%type% ("))
        {
            // Get function information about the called function
            const Function *function = tok->function();
            // Check if function is constructor..
            if (function)
            {
                const Scope *sp = tok->scope();
                const Scope::ScopeType tp = sp->type;
                //std::cout << "Type: " << tp << "     " << tok->str() << std::endl;

                DFS(sp, 0);

                std::list<Scope *> lst = sp->nestedList;

                for (std::list<Scope *>::iterator it = lst.begin(); it != lst.end(); it++)
                {
                    const Scope::ScopeType type = (*it)->type;
                    //std::cout << type;

                    std::list<Scope *> list = (*it)->nestedList;
                    //std::cout << "    " << list.size() << std::endl;
                }
            }
        }
    }
}

int CheckGost::DFS(const Scope *scope, int i)
{
    if ( i> 20)
    {
        std::cout << "Cyclomatic complexity of a program must be less than 20" << std::endl;
        //eturnGostError(tok, "Cyclomatic complexity of a program must be less than 20");
    }
    std::list<Scope *> lst = scope->nestedList;
    for (std::list<Scope *>::iterator it = lst.begin(); it != lst.end(); it++)
    {
        DFS(*it, i+1);
    }
}

void CheckGost::functionSize()
{
    // Iterate through all tokens in the token list
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next())
    {
        const Scope *sp = tok->scope();
        const Scope::ScopeType tp = sp->type;
        if (tp == Scope::ScopeType(Scope::eFunction) && tok == sp->classStart)
        {
            const Token *token = sp->classStart;
            int semicolonCount = 0;
            while (token != sp->classEnd)
            {
                if (token->str() == ";")
                {
                    semicolonCount++;
                }
                if (token->str() == "for")
                {
                    semicolonCount -= 2;
                }
                token = token->next();
            }
            if (semicolonCount >200)
            {
                std::cout << "Function size must be less than 200" << std::endl;
                returnGostError(tok, "Function size must be less than 200");
            }
        }
    }
}

void CheckGost::returnGostError(const Token *tok, const std::string &errstr)
{
    reportError(tok, Severity::error,
                "Gost Error.",
                errstr);
}
