// Bassil Saleh
// CS280, section 007
// PA3: Interpreter
// Version 10
#include "parserInterp.h"

map<string, bool> defVar;
map<string, Token> SymTable;

map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects

static int error_count = 0;

namespace Parser {
    bool pushed_back = false;
    LexItem pushed_token;
    // We need one token for looking ahead
    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        // Refers to getNextToken() as defined in lex.cpp
        return getNextToken(in, line);
    }
    static void PushBackToken(LexItem& token) {
        // Should not push back more than once
        if (pushed_back) {abort();}
        pushed_back = true;
        pushed_token = token;
    }
}

void ParseError(int line, string msg) {
    ++error_count;
    cout << line << ": " << msg << endl;
}

int ErrCount() {return error_count;}

void SkipStmt(istream& in, int& line) {
    // A SimpleStmt either begins with:
    // IDENT for AssignStmt
    // WRITELN for WriteLnStmt
    // WRITE for WriteStmt
    // ------------------------------------
    // A StructuredStmt either begins with:
    // IF for IfStmt
    // BEGIN for CompoundStmt
    LexItem currToken;
    currToken = Parser::GetNextToken(in, line);
    if (currToken == IDENT) {
        do {
            currToken = Parser::GetNextToken(in, line);
        } while (currToken != SEMICOL);
        // After this loop breaks, the program
        // should read the token after ;
    }
    else if ((currToken == WRITELN) || (currToken == WRITE)) {
        do {
            currToken = Parser::GetNextToken(in, line);
        } while (currToken != RPAREN);
        // After this loop ends, program should
        // read the token after )
    }
    else if (currToken == IF) {
        // How do you know when you've finished skipping an expression?
        ;
    }
    else if (currToken == BEGIN) {
        do {
            currToken = Parser::GetNextToken(in, line);
        } while (currToken != END);
        // After this loop breaks, the program should
        // read the token after END
    }
//    Parser::PushBackToken(currToken);
}

bool Factor(istream& in, int& line, Value & retVal, int sign) {
    // Factor ::= IDENT | ICONST | RCONST | SCONST | BCONST | (Expr)
    LexItem currToken;
    string varName;
    bool isFactor = false;
//    cout << "in Factor" << endl;
    // + and - only work for numeric types (ICONST, RCONST)
    // NOT only works for Boolean type (BCONST)
    currToken = Parser::GetNextToken(in, line);
    if (currToken == IDENT) {
        // If currToken is an IDENT, check if the variable has been declared.
        varName = currToken.GetLexeme();
        if (defVar.find(varName) == defVar.end()) {
            ParseError(line, "Undeclared variable.");
            return false;
        }
        // Check if the variable has been initialized before using it.
        map<string, Value>::iterator found = TempsResults.find(varName);
        Value foundVal = found->second;
        ValType foundType = foundVal.GetType();

        if (foundType == VERR) {
            ParseError(line, "Using uninitialized variable.");
            return false;
        }
        else if ((foundType == VINT) || (foundType == VREAL)) {
            // Unary NOT only applies to booleans.
            if (sign == NOT) {
                ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
                return false;
            }
        }
        else if (foundType == VBOOL) {
            if ((sign == MINUS) || (sign == PLUS)) {
                // Unary + or - only applies to integers and reals.
                ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
                return false;
            }
        }
        else if (foundType == VSTRING) {
            // String has no unary signs.
            if ((sign == MINUS) || (sign == PLUS) || (sign == NOT)) {
                ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
                return false;
            }
        }
        // Now extract the variable's value and return it via retVal
        switch(foundType) {
        case VINT:
            retVal.SetType(VINT);
            retVal.SetInt(foundVal.GetInt());
            break;
        case VREAL:
            retVal.SetType(VREAL);
            retVal.SetReal(foundVal.GetReal());
            break;
        case VBOOL:
            retVal.SetType(VBOOL);
            retVal.SetBool(foundVal.GetBool());
            break;
        case VSTRING:
            retVal.SetType(VSTRING);
            retVal.SetString(foundVal.GetString());
            break;
        default:
            break;
        }
        return true;
    }

    if ((sign == MINUS) || (sign == PLUS)) {
        if ((currToken != ICONST) && (currToken != RCONST)) {
            ParseError(line, "Unary sign only applies to numeric type operands.");
            return false;
        }
    }
    else if (sign == NOT) {
        if (currToken != BCONST) {
            ParseError(line, "Unary NOT only applies to Boolean type operand.");
            return false;
        }
    }
    // Check for left parentheses
    if (currToken == LPAREN) {
        isFactor = Expr(in, line, retVal);
        if (!isFactor) {
            ParseError(line, "Invalid expression.");
            return false;
        }
        currToken = Parser::GetNextToken(in, line);
        if (currToken != RPAREN) {
            ParseError(line, "Missing right parenthesis.");
            return false;
        }
        // currToken is RPAREN, return true
        return true;
    }
    // No parentheses, check if factor is a proper nonterminal
    isFactor = (currToken == IDENT) || (currToken == ICONST) ||
               (currToken == RCONST) || (currToken == SCONST) ||
               (currToken == BCONST);
    if (isFactor) {
        Token nonTermType = currToken.GetToken();
        string nonTermLex = currToken.GetLexeme(); // could either be int, real, string, or bool
        switch(nonTermType) {
        case IDENT:
            // IDENT should have already been assigned something
            break;
        case ICONST: {
            retVal.SetType(VINT);
            int x = stoi(nonTermLex);
            if (sign == MINUS) {
                x = -x;
            }
            retVal.SetInt(x);
            break;
        }
        case RCONST: {
            retVal.SetType(VREAL);
            double x = stod(nonTermLex);
            retVal.SetReal(x);
            break;
        }
        case SCONST: {
            retVal.SetType(VSTRING);
            retVal.SetString(nonTermLex);
            break;
        }
        case BCONST: {
            retVal.SetType(VBOOL);
            if (nonTermLex == "true") {
                retVal.SetBool(true);
            }
            else if (nonTermLex == "false") {
                retVal.SetBool(false);
            }
            break;
        }
        default:
            break;
        }
    }

    return isFactor;
}

bool SFactor(istream& in, int& line, Value & retVal) {
    // SFactor ::= [( - | + | NOT )] Factor
    LexItem currToken;
    bool isSFactor = false;
//    cout << "in SFactor" << endl;
    currToken = Parser::GetNextToken(in, line);
    if ((currToken == MINUS) || (currToken == PLUS) || (currToken == NOT)) {
        // Each token has its own integer value
        isSFactor = Factor(in, line, retVal, currToken.GetToken());
    }
    else {
        // int sign = -1 means no sign was specified for the Factor
        Parser::PushBackToken(currToken);
        isSFactor = Factor(in, line, retVal, -1);
    }
//    bool isSFactor = Factor(in, line, 0);
    return isSFactor;
}

bool Term(istream& in, int& line, Value & retVal) {
    // Term ::= SFactor { ( * | / | DIV | MOD ) SFactor }
    LexItem currToken;
    Token op;
//    Value val1;
    Value val2;
    Value result;
    bool isTerm = false;
//    cout << "in Term" << endl;
    isTerm = SFactor(in, line, retVal);
    if (!isTerm) {
        ParseError(line, "Term is not a factor.");
        return false;
    }
    result = retVal;
    do {
        // If there's another SFactor, an operator can be applied to them
//        val1 = retVal;
        currToken = Parser::GetNextToken(in, line);
        if (currToken == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << currToken.GetLexeme() << ")" << endl;
            return false;
        }
        // If op is *, /, DIV, or MOD, get the next SFactor.
        // Otherwise, push back the token and break.
        op = currToken.GetToken();
        if ((currToken == MULT) || (currToken == DIV) || (currToken == IDIV) || (currToken == MOD)) {
            isTerm = SFactor(in, line, retVal);
            val2 = retVal;
            // int * int = int
            // int * real = real
            // real * int = real
            // real * real = real
            // ----------------------------
            // int / int = int
            // int / real = real
            // real / int = real
            // real / real = real
            // ----------------------------
            // IDIV, MOD only work with integers
            switch(op) {
            case MULT:
                result = result * val2;
//                result = val1 * val2;
                break;
            case DIV:
                // Although the Val object automatically evaluates to
                // VERR when attempting division by zero, it would
                // be ideal if the interpreter printed out a special
                // message for this error.
                if (val2.IsReal()) {
                    if (FP_ZERO == fpclassify(val2.GetReal())) {
                        ParseError(line, "Run-Time Error-Illegal division by Zero");
                        return false;
                    }
                }
                if (val2.IsInt()) {
                    if (val2.GetInt() == 0) {
                        ParseError(line, "Run-Time Error-Illegal division by Zero");
                        return false;
                    }
                }
                // If val2 is anything other than a non-zero real or int,
                // this should evaluate to a VERR.
                result = result / val2;
//                result = val1 / val2;
                break;
            case IDIV:
                // Special message for division by zero
                if (val2.IsReal()) {
                    if (FP_ZERO == fpclassify(val2.GetReal())) {
                        ParseError(line, "Run-Time Error-Illegal integer division (div) by Zero");
                        return false;
                    }
                }
                if (val2.IsInt()) {
                    if (val2.GetInt() == 0) {
                        ParseError(line, "Run-Time Error-Illegal integer division (div) by Zero");
                        return false;
                    }
                }
                // If val2 is anything other than a non-zero integer,
                // this should evaluate to a VERR.
                result = result.div(val2);
//                result = val1.div(val2);
                break;
            case MOD:
                result = result % val2;
//                result = val1 % val2;
                break;
            default:
                break;
            };
            if (result.IsErr()) {
                ParseError(line, "Illegal operand types for an arithmetic operator.");
                return false;
            }
        }
    } while ((currToken == MULT) || (currToken == DIV) || (currToken == IDIV) || (currToken == MOD));
    Parser::PushBackToken(currToken);
    retVal = result;
    return isTerm;
}

bool SimpleExpr(istream& in, int& line, Value & retVal) {
    // SimpleExpr :: Term { ( + | - ) Term }
    LexItem currToken;
    bool isSimpleExpr = false;
//    cout << "in SimpleExpr" << endl;
    do {
        isSimpleExpr = Term(in, line, retVal);
        if (!isSimpleExpr) {
//            ParseError(line, "Invalid simple expression.");
            return false;
        }
        currToken = Parser::GetNextToken(in, line);
    } while ((currToken == PLUS) || (currToken == MINUS));
    Parser::PushBackToken(currToken);
    return isSimpleExpr;
}

bool RelExpr(istream& in, int& line, Value & retVal) {
    // RelExpr ::= SimpleExpr [ ( = | < | > ) SimpleExpr ]
    LexItem currToken;
    Token op = ERR;
    Value val1;
    Value val2;
    Value result;
//    Token opType;
    bool isRelExpr = false;
//    cout << "in RelExpr" << endl;
    isRelExpr = SimpleExpr(in, line, retVal);
    if (!isRelExpr) {
//        ParseError(line, "Invalid simple expression.");
        return false;
    }
    // Save the value of the evaluated expression
    // so that it can be used for comparison.
    val1 = retVal;
    // Check for =, <, >
    currToken = Parser::GetNextToken(in, line);
    if ((currToken == EQ) || (currToken == LTHAN) || (currToken == GTHAN)) {
        op = currToken.GetToken();
        isRelExpr = SimpleExpr(in, line, retVal);
        if (!isRelExpr) {
            ParseError(line, "Invalid simple expression.");
            return false;
        }
        val2 = retVal;
        // If there is another =, < or > after 2nd SimpleExpr,
        // return a parse error (these operators cannot be cascaded).
        // Otherwise, push the token back to the stack.
        currToken = Parser::GetNextToken(in, line);
        if ((currToken == EQ) || (currToken == LTHAN) || (currToken == GTHAN)) {
            ParseError(line, "Illegal Relational Expression.");
            return false;
        }
        Parser::PushBackToken(currToken);
    }
    else {
        Parser::PushBackToken(currToken);
    }
    // Now evaluate the expression
    switch(op){
    case EQ:
        result = (val1 == val2);
        retVal = result;
        break;
    case LTHAN:
        result = (val1 < val2);
        retVal = result;
        break;
    case GTHAN:
        result = (val1 > val2);
        retVal = result;
        break;
    default:
        break;
    }
    return isRelExpr;
}

bool LogANDExpr(istream& in, int& line, Value & retVal) {
    // LogAndExpr ::= RelExpr {AND RelExpr }
    LexItem currToken;
    bool isLogANDExpr = false;
    Value result;
    Value val2;
//    cout << "in LogANDExpr" << endl;
    isLogANDExpr = RelExpr(in, line, retVal);
    if (!isLogANDExpr) { return false; }
    // retVal should be a VBOOL
    result = retVal;
    do {
//        isLogANDExpr = RelExpr(in, line, retVal);
//        if (!isLogANDExpr) {
////            ParseError(line, "Invalid relational expression.");
//            return false;
//        }
        currToken = Parser::GetNextToken(in, line);
        if (currToken == AND) {
            isLogANDExpr = RelExpr(in, line, retVal);
            if (!isLogANDExpr) { return false; }
            // retVal should be a VBOOL
            val2 = retVal;
            result = result && val2;
        }
    } while (currToken == AND);
    Parser::PushBackToken(currToken);
    retVal = result;
//    bool isLogANDExpr = RelExpr(in, line);
    return isLogANDExpr;
}

bool Expr(istream& in, int& line, Value & retVal) {
    // Expr ::= LogOrExpr ::= LogAndExpr { OR LogAndExpr }
    LexItem currToken;
    bool isExpr = false;
//    cout << "in Expr" << endl;
    do {
        isExpr = LogANDExpr(in, line, retVal);
        if (!isExpr) {
//            ParseError(line, "Invalid expression.");
            return false;
        }
        currToken = Parser::GetNextToken(in, line);
    } while (currToken == OR);
    Parser::PushBackToken(currToken);
    return isExpr;
}

bool ExprList(istream& in, int& line) {
    // ExprList ::= Expr { , Expr }
    LexItem currToken;
    Value retVal;
    bool isExprList = false;
//    cout << "in ExprList" << endl;
    do {
        isExprList = Expr(in, line, retVal);
        if (!isExprList) {
            ParseError(line, "Invalid expression.");
            return false;
        }
        // retVal should be a non-error value at this point.
        ValQue->push(retVal);
        // Check for comma in between expressions
        currToken = Parser::GetNextToken(in, line);
        if (currToken == ERR) {
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << currToken.GetLexeme() << ")" << endl;
            return false;
        }
    } while (currToken == COMMA);
    Parser::PushBackToken(currToken);
    return isExprList;
}

bool Var(istream& in, int& line, LexItem & idtok) {
    // Var ::= IDENT
//    LexItem currToken;
//    cout << "in Var" << endl;
    idtok = Parser::GetNextToken(in, line);
    if (idtok != IDENT) {
        ParseError(line, "Lexeme (" + idtok.GetLexeme() + ") is not a valid variable name.");
        return false;
    }
    // currToken is IDENT
    if (defVar.find(idtok.GetLexeme()) == defVar.end()) {
        ParseError(line, "Undeclared Variable");
        return false;
    }
    // At this point, idtok should be a valid identifier.
    return true;
}

bool AssignStmt(istream& in, int& line) {
    // AssignStmt ::= Var := Expr
    LexItem currToken;
    Value retVal;
    Token assignType;
    ValType retValType;
    string varName;
    bool isAssignStmt = false;
//    cout << "in AssignStmt" << endl;
    isAssignStmt = Var(in, line, currToken);
    if (!isAssignStmt) {
        ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
        return false;
    }
    // At this point, currToken should be a valid variable
    varName = currToken.GetLexeme();
    currToken = Parser::GetNextToken(in, line);
    if (currToken != ASSOP) {
        ParseError(line, "Missing Assignment Operator");
        return false;
    }
    isAssignStmt = Expr(in, line, retVal);
    if (!isAssignStmt) {
        ParseError(line, "Missing Expression in Assignment statement");
        return false;
    }
    // retVal and variable being assigned must have a compatible type
    assignType = SymTable[varName];
    retValType = retVal.GetType();
    if (((assignType == INTEGER) && (retValType == VINT))
        || ((assignType == REAL) && (retValType == VREAL))
        || ((assignType == BOOLEAN) && (retValType == VBOOL))
        || ((assignType == STRING) && (retValType == VSTRING)))
        {
            // int := int
            // real := real
            // string := string
            // bool := bool
            TempsResults[varName] = retVal;
        }
    // Implicit conversions:
    else if ((assignType == INTEGER) && (retValType == VREAL)) {
        // int := real becomes int := int
        int x = static_cast<int>(retVal.GetReal());
        retVal.SetType(VINT);
        retVal.SetInt(x);
        TempsResults[varName] = retVal;
    }
    else if ((assignType == REAL) && (retValType == VINT)) {
        // real := int becomes real := real
        double x = static_cast<double>(retVal.GetInt());
        retVal.SetType(VREAL);
        retVal.SetReal(x);
        TempsResults[varName] = retVal;
    }
    else {
        ParseError(line, "Illegal mixed-mode assignment operation");
        return false;
    }
    return isAssignStmt;
}

bool IfStmt(istream& in, int& line) {
    // IfStmt ::= IF Expr THEN Stmt [ ELSE Stmt ]
    // Assumes IF keyword was read prior to calling this function
    LexItem currToken;
    Value retVal;
    bool isIfStmt = false;
//    cout << "in IfStmt" << endl;
    isIfStmt = Expr(in, line, retVal);
    if (!isIfStmt) {
        ParseError(line, "Missing conditional expression in If-Stmt");
        return false;
    }
    if (!retVal.IsBool()) {
        ParseError(line, "Run-Time Error-Illegal Type for If statement condition");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != THEN) {
        ParseError(line, "Missing then keyword in If-Stmt");
        return false;
    }
    if (retVal.GetBool()) {
        // Run THEN Stmt and skip ELSE Stmt.
        isIfStmt = Stmt(in, line);
        if (!isIfStmt) {
            ParseError(line, "Missing statement for If-Stmt Then-Part");
            return false;
        }
        currToken = Parser::GetNextToken(in, line);
        if (currToken == ELSE) {
            // Skip the Stmt after ELSE.
            SkipStmt(in, line);
        }
        else {
            Parser::PushBackToken(currToken);
        }
    }
    else {
        // Skip THEN Stmt.
        SkipStmt(in, line);
        // If ELSE Stmt exists, run that instead.
        currToken = Parser::GetNextToken(in, line);
        if (currToken == ELSE) {
            isIfStmt = Stmt(in, line);
            if (!isIfStmt) {
                ParseError(line, "Missing statement after else");
                return false;
            }
        }
        else {
            Parser::PushBackToken(currToken);
        }
    }
//    if (currToken == ELSE) {
//        isIfStmt = Stmt(in, line);
//        if (!isIfStmt) {
//            ParseError(line, "Missing statement after else");
//            return false;
//        }
//    }
//    else {
//        Parser::PushBackToken(currToken);
//    }
    // Should be a semicolon at this point
//    Parser::PushBackToken(currToken);
    return isIfStmt;
}

bool WriteStmt(istream& in, int& line) {
    // WriteStmt ::= WRITE (ExprList)
    LexItem currToken;
    // Reset ValQue on each call to WriteStmt
    ValQue = new queue<Value>;
    bool isWriteStmt = false;
//    cout << "in WriteStmt" << endl;
    currToken = Parser::GetNextToken(in, line);
    if (currToken != WRITE) {
        ParseError(line, "Missing WRITE keyword.");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    isWriteStmt = ExprList(in, line);
    if (!isWriteStmt) {
        ParseError(line, "Missing expression list for Write statement");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    // Print out each evaluated expression
    while (!ValQue->empty()) {
        Value nextVal = ValQue->front();
        cout << nextVal;
        ValQue->pop();
    }
    // No newline here
    return isWriteStmt;
}

bool WriteLnStmt(istream& in, int& line) {
    // WriteLnStmt ::= WRITELN (ExprList)
    LexItem currToken;
    // Reset ValQue on each call to WriteLnStmt
    ValQue = new queue<Value>;
    bool isWriteLnStmt = false;
//    cout << "in WriteLnStmt" << endl;
    currToken = Parser::GetNextToken(in, line);
    if (currToken != WRITELN) {
        ParseError(line, "Missing WRITELN keyword.");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    isWriteLnStmt = ExprList(in, line);
    if (!isWriteLnStmt) {
        ParseError(line, "Missing expression list for WriteLn statement");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    // Print out each evaluated expression
    while(!ValQue->empty()) {
        Value nextVal = ValQue->front();
        cout << nextVal;
        ValQue->pop();
    }
    cout << '\n';   // What the "Ln" in "WriteLnStmt" means.
    return isWriteLnStmt;
}

bool SimpleStmt(istream& in, int& line) {
    // SimpleStmt ::= AssignStmt | WriteLnStmt | WriteStmt
    LexItem currToken;
    bool isSimpleStmt = false;
//    cout << "in SimpleStmt" << endl;
    currToken = Parser::GetNextToken(in, line);
    // AssignStmt begins with Var, which is IDENT
    if (currToken == IDENT) {
        Parser::PushBackToken(currToken);
        isSimpleStmt = AssignStmt(in, line);
    }
    // WriteLnStmt begins with WRITELN
    else if (currToken == WRITELN) {
        Parser::PushBackToken(currToken);
        isSimpleStmt = WriteLnStmt(in, line);
    }
    // WriteStmt begins with WRITE
    else if (currToken == WRITE) {
        Parser::PushBackToken(currToken);
        isSimpleStmt = WriteStmt(in, line);
    }
    if (!isSimpleStmt) {
        ParseError(line, "Incorrect simple statement.");
        return false;
    }
    return isSimpleStmt;
}

bool CompoundStmt(istream& in, int& line) {
    // CompoundStmt ::= BEGIN Stmt {; Stmt } END
    LexItem currToken;
    bool isCompoundStmt = false;
//    cout << "in CompoundStmt" << endl;
    currToken = Parser::GetNextToken(in, line);
    if (currToken != BEGIN) {
        ParseError(line, "Missing BEGIN keyword in compound statement.");
        return false;
    }
    // Keep reading Stmt as long as there is
    // a semicolon after each Stmt nonterminal.
    do {
        isCompoundStmt = Stmt(in, line);
        if (!isCompoundStmt) {
            ParseError(line, "Invalid compound statement.");
            return false;
        }
        currToken = Parser::GetNextToken(in, line);
    } while (currToken == SEMICOL);
    // Last token should be END terminal
    if (currToken != END) {
        ParseError(line, "Missing end of compound statement.");
        return false;
    }
    return isCompoundStmt;
}

bool StructuredStmt(istream& in, int& line) {
    // StructuredStmt ::= IfStmt | CompoundStmt
    // IfStmt begins with IF
    // CompoundStmt begins with BEGIN
    LexItem currToken;
    bool isStructuredStmt = false;
//    cout << "in StructuredStmt" << endl;
    currToken = Parser::GetNextToken(in, line);
    if (currToken == IF) {
//        Parser::PushBackToken(currToken);
        isStructuredStmt = IfStmt(in, line);
    }
    else if (currToken == BEGIN) {
        Parser::PushBackToken(currToken);
        isStructuredStmt = CompoundStmt(in, line);
    }
    if (!isStructuredStmt) {
        ParseError(line, "Invalid structured statement.");
        return false;
    }
    return isStructuredStmt;
}

bool Stmt(istream& in, int& line) {
    // Stmt ::= SimpleStmt | StructuredStmt
    LexItem currToken;
    bool isStmt = false;
//    cout << "in Stmt" << endl;
    // A SimpleStmt either begins with:
    // IDENT for AssignStmt
    // WRITELN for WriteLnStmt
    // WRITE for WriteStmt
    // ------------------------------------
    // A StructuredStmt either begins with:
    // IF for IfStmt
    // BEGIN for CompoundStmt
    currToken = Parser::GetNextToken(in, line);
    if ((currToken == IDENT) || (currToken == WRITE) || (currToken == WRITELN)) {
        Parser::PushBackToken(currToken);
        isStmt = SimpleStmt(in, line);
    }
    else if ((currToken == IF) || (currToken == BEGIN)) {
        Parser::PushBackToken(currToken);
        isStmt = StructuredStmt(in, line);
    }
    else if (currToken == DOT) {
            // testprog6
        currToken = Parser::GetNextToken(in, line);
        if ((currToken == DONE) || (currToken == ERR)) {
            // Missing end of compound statement
            return false;
        }
        else {
            Parser::PushBackToken(currToken);
        }
    }
    else {
//        ParseError(line, "Missing beginning keyword of statement.");
        return false;
    }

    return isStmt;
}

bool DeclStmt(istream& in, int& line) {
    // DeclStmt ::= IDENT {, IDENT } : Type [:= Expr]
    LexItem currToken;
    Value retVal;               // error type by default
    string varName;
    Token varType;
    vector<string> declLine;    // variables declared on a single DeclStmt
    int varCount = 0;           // used to check if 1st IDENT token was passed
    bool isDeclStmt = false;
    bool syntaxError = false;   // missing comma, variable redefinition, etc.
//    cout << "in DeclStmt" << endl;

    do {
        currToken = Parser::GetNextToken(in, line);
        varName = currToken.GetLexeme();
        if (currToken != IDENT) {
            if (varCount < 1) {
                ParseError(line, "Missing identifier in declaration statement.");
                return false;
            }
            // Variable names must adhere to identifier lexeme rules.
            ParseError(line, "Lexeme (" + varName + ") is not a valid identifier.");
            return false;
        }
        if (defVar.find(varName) != defVar.end()) {
            // Variable cannot be defined more than once.
            ParseError(line, "Variable redefinition.");
            syntaxError = true;
            break;
//            ParseError(line, "Incorrect identifiers list in declaration statement.");
//            return false;
        }
        else {
            // Variable has not been declared yet. Add an entry to defVar and SymTable.
            defVar[varName] = true;
            declLine.push_back(varName);
//            SymTable[varName] = ERR;    // Should be changed after type is verified
            ++varCount;
        }
        // IDENT followed by another IDENT without
        // a comma in between is a syntax error.
        currToken = Parser::GetNextToken(in, line);
        if (currToken == IDENT) {
            ParseError(line, "Missing comma in declaration statement.");
            syntaxError = true;
            break;
//            return false;
        }
    } while (currToken == COMMA);

    if (syntaxError) {
        // Missing comma, variable redefinition, etc.
        ParseError(line, "Incorrect identifiers list in declaration statement.");
        return false;
    }

    if (currToken != COLON) {
        ParseError(line, "Syntax error in declaration statement.");
        return false;
    }
    // currToken is colon, check if next token is a valid type.
    currToken = Parser::GetNextToken(in, line);
    isDeclStmt = ((currToken == INTEGER) || (currToken == REAL) || (currToken == BOOLEAN) || (currToken == STRING));
    if (!isDeclStmt) {
        ParseError(line, "Invalid declaration type.");
        return false;
    }
    // currToken should have a valid type, so extract its type
    // and pair it with each declared variable in SymTable.
    varType = currToken.GetToken();
    for (int i = 0; i < declLine.size(); ++i) {
        varName = declLine.at(i);
        SymTable[varName] = varType;
    }
//    map<string, Token>::iterator it;
//    for (it = SymTable.begin(); it != SymTable.end(); ++it) {
//        // Newly declared variables are assumed to be ERR before their type is verified.
//        if (it->second == ERR) {
//            it->second = currToken.GetToken();
//        }
//    }
    currToken = Parser::GetNextToken(in, line);
    // Next token might be :=
    if (currToken == ASSOP) {
        // Call <Expr> nonterminal
        isDeclStmt = Expr(in, line, retVal);
        if (!isDeclStmt) {
//            ParseError(line, "Invalid expression in declaration statement.");
            return false;
        }
    }
    else {
        // DeclPart EBNF rule checks if ; exists
        Parser::PushBackToken(currToken);
    }
    // At this point, if retVal is ERR type, then the variables should be uninitialized.
    // If not, then Expr() must have assigned retVal something valid.
    // Either way, assign the value of retVal to each variable in DeclStmt.
    for (int i = 0; i < declLine.size(); ++i) {
        varName = declLine.at(i);
        TempsResults[varName] = retVal;
    }
//    for (it = SymTable.begin(); it != SymTable.end(); ++it) {
//        TempsResults[it->first] = retVal;
//    }
    return isDeclStmt;
}

bool DeclPart(istream& in, int& line) {
    // DeclPart ::= VAR DeclStmt; { DeclStmt ; }
    LexItem currToken;
//    cout << "in DeclPart" << endl;
    currToken = Parser::GetNextToken(in, line);
    if (currToken != VAR) {
        ParseError(line, "Missing VAR keyword in declaration statement.");
        return false;
    }
    bool isDeclStmt = false;

    do {
        isDeclStmt = DeclStmt(in, line);
        if (!isDeclStmt) {
            ParseError(line, "Syntax error in declaration statement.");
            return false;
        }
        currToken = Parser::GetNextToken(in, line);
        if (currToken != SEMICOL) {
            ParseError(line, "Missing semicolon in declaration statement.");
            return false;
        }
        // If next token is an IDENT, it could be the beginning of another DeclStmt.
        currToken = Parser::GetNextToken(in, line);
        if (currToken == IDENT) {
            Parser::PushBackToken(currToken);
        }
    } while (currToken == IDENT);
    Parser::PushBackToken(currToken);
    return isDeclStmt;
}

bool Prog(istream& in, int& line) {
    // Prog ::= PROGRAM IDENT ; DeclPart CompoundStmt .
    LexItem currToken;
//    cout << "in Prog before DeclPart & CompoundStmt" << endl;
    currToken = Parser::GetNextToken(in, line);

    if (currToken != PROGRAM) {
        ParseError(line, "Missing PROGRAM Keyword.");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != IDENT) {
        ParseError(line, "Missing Program Name.");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != SEMICOL) {
        ParseError(line, "Missing semicolon.");
        return false;
    }
    // Call DeclPart
    bool isProg = DeclPart(in, line);
    if (!isProg) {
        ParseError(line, "Incorrect declaration section.");
        return false;
    }
    // Call CompoundStmt
    isProg = CompoundStmt(in, line);
    if (!isProg) {
        ParseError(line, "Incorrect program body.");
        return false;
    }
    currToken = Parser::GetNextToken(in, line);
    if (currToken != DOT) {
        ParseError(line, "Missing period after end keyword.");
        return false;
    }

    return isProg;
}
