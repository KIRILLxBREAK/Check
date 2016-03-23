//---------------------------------------------------------------------------
#ifndef checkgostH
#define checkgostH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"

class CPPCHECKLIB CheckGost : public Check {
public:

    CheckGost() : Check(myName()) {
    }

    CheckGost(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger) {
    }

    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        std::cout << "\ngost\n";

        CheckGost check(tokenizer, settings, errorLogger);

        //check.gostCheck();
        check.indentifierCheck();
        check.globalStaticCheck();
        check.literalCheck();
        check.customTypeCheck();
        check.structMemberCheck();
        check.constructorDestructorCheck();
        check.switchCheck();
        check.qualCheck();
        check.nestedCheck();
        check.functionSize();

        std::cout << "\nfinfish gost check\n";
    }

    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        /*std::cout << "\ngost\n";
        CheckGost check(tokenizer, settings, errorLogger);
        //check.gostCheck();
        check.indentifierCheck();
        std::cout << "\nfinfish gost check\n";*/
    }

protected:
    void gostCheck();
    void indentifierCheck();
    void globalStaticCheck();
    void literalCheck();
    void customTypeCheck();
    void structMemberCheck();
    void structNameCheck();
    void constructorDestructorCheck();
    void switchCheck();
    void qualCheck();
    void nestedCheck();
    void functionSize();

    std::string getFirst(std::string structName);
    int DFS(const Scope *scope, int i);



private:

    void returnGostError(const Token *tok, const std::string& errstr);

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        CheckGost c(0, settings, errorLogger);
        c.returnGostError(0, "Error String");
    }

    static std::string myName() {
        return "Gost";
    }

    std::string classInfo() const {
        return "Verifies compliance with GOST.\n";
    }

};
//---------------------------------------------------------------------------
#endif // checkgostH
