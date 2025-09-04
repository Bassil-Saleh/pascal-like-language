// Name: Bassil Saleh
// Class: CS 280, section 007
// Programming Assignment #1: Lexical Analyzer

#include "lex.h"
#include <fstream>
#include <cctype>

enum IdentChar {
	// For character types not covered in Token enum.
	// Includes valid characters for identifiers
	LETTER, DIGIT, UNDERSCORE, DOLLAR_SIGN,

	UNKNOWN
};

IdentChar checkIdent(char c) {
	// Checks for valid chars when parsing identifiers
	if (std::isdigit(c)) {
		return IdentChar::DIGIT;
	}
	else if (std::isalpha(c)) {
		return IdentChar::LETTER;
	}

	switch (c) {
		// Identifier characters
		case '_':
			return IdentChar::UNDERSCORE;
		case '$':
			return IdentChar::DOLLAR_SIGN;
	}

	return IdentChar::UNKNOWN;
}

Token checkSymbol(char c) {
	switch (c) {
		// Operators (except ASSOP)
		case '+':
			return Token::PLUS;
		case '-':
			return Token::MINUS;
		case '*':
			return Token::MULT;
		case '/':
			return Token::DIV;
		case '=':
			return Token::EQ;
		case '<':
			return Token::LTHAN;
		case '>':
			return Token::GTHAN;

		// Delimiters
		case ',':
			return Token::COMMA;
		case ';':
			return Token::SEMICOL;
		case '(':
			return Token::LPAREN;
		case ')':
			return Token::RPAREN;
		case ':':
			// Could either be COLON or ASSOP
			return Token::COLON;
		case '.':
			return Token::DOT;

        // EOF Character
        case std::ifstream::traits_type::eof():
            return Token::DONE;
		default:
			break;
	}
	// In this context, ERR does not necessarily mean
	// the character is invalid, just that it is not
	// a token represented by a single character.
	return Token::ERR;
}

bool isWhiteSpace(char c) {
	// Does not check for newline, since newline is used to increment linenum
	std::string charsToSkip = " \t\v\f\r";
	return (charsToSkip.find(c) != std::string::npos);
}

void readIdent(std::istream& in, std::string& lexeme, int& linenum, char& currChar, char& nextChar) {
	// 1st char is a letter. Append it to lexeme and peek at the next char.
	lexeme += currChar;
	nextChar = in.peek();
	// Keep reading characters as long as the next char is is a letter, digit, _, or $.
	while (checkIdent(nextChar) != IdentChar::UNKNOWN) {
		currChar = in.get();
		lexeme += currChar;
		nextChar = in.peek();
	}
	// Otherwise, stop reading the lexeme.
//	LexItem ident = LexItem(Token::IDENT, lexeme, linenum);
//	return ident;
}

LexItem readNum(std::istream& in, std::string& lexeme, int& linenum, char& currChar, char& nextChar) {
    LexItem token;
    // 1st char is a digit. Append to lexeme and peek at the next char.
    lexeme += currChar;
    nextChar = in.peek();
    // Keep reading characters as long as the next character is a digit.
    while (std::isdigit(nextChar)) {
        currChar = in.get();
        lexeme += currChar;
        nextChar = in.peek();
    }
    // If the next char is '.', token might be an RCONST.
    if (nextChar == '.') {
        currChar = in.get();
        lexeme += currChar;
        nextChar = in.peek();
        // Keep reading digits until we find a '.' or other non-numerical character
        while (std::isdigit(nextChar)) {
            currChar = in.get();
            lexeme += currChar;
            nextChar = in.peek();
        }
        // RCONST cannot have more than one '.'
        // Append the '.' to lexeme and return an ERR token.
        if (nextChar == '.') {
            currChar = in.get();
            lexeme += currChar;
            token = LexItem(Token::ERR, lexeme, linenum);
            return token;
        }
        else {
            // If next char is neither a digit nor '.',
            // stop parsing lexeme and return an RCONST token.
            token = LexItem(Token::RCONST, lexeme, linenum);
            return token;
        }
    }
    // If the next char is neither a digit nor '.', stop parsing
    // the lexeme and return it as an ICONST token.
    token = LexItem(Token::ICONST, lexeme, linenum);
    return token;
}

LexItem readStr(std::istream& in, std::string& lexeme, int& linenum, char& currChar, char& nextChar) {
    LexItem token;
    // 1st char is single quote. Append to lexeme and peek at next char.
    lexeme += currChar;
    nextChar = in.peek();
    // Keep reading characters and appending them to lexeme
    // until we find another single quote, double quotes, or newline char.
    while ((nextChar != '\n') && (nextChar != '\"') && (nextChar != '\'')) {
        currChar = in.get();
        lexeme += currChar;
        nextChar = in.peek();
    }
    switch(nextChar) {
    case '\'':
        // Append 2nd single quote to lexeme and return SCONST string.
        currChar = in.get();
        lexeme += currChar;
        token = LexItem(Token::SCONST, lexeme, linenum);
        break;
    case '\n':
        // Appending newline to lexeme might result in strange output
        // when printing out the unrecognized lexeme, but a lexeme
        // missing a 2nd single quote would look more clear.
        token = LexItem(Token::ERR, lexeme, linenum);
        break;
    default:
        // Next char is a double quote. Append to lexeme and return ERR token.
        currChar = in.get();
        lexeme += currChar;
        token = LexItem(Token::ERR, lexeme, linenum);
        break;
    }
    return token;
}

LexItem readAssopOrColon(std::istream& in, std::string& lexeme, int& linenum, char& currChar, char& nextChar) {
    LexItem token;
    // Append curr char ':' to lexeme and check next char
    lexeme += currChar;
    nextChar = in.peek();
    // If next char is '=', read it and append to lexeme, then return ASSOP token
    if (nextChar == '=') {
        currChar = in.get();
        lexeme += currChar;
        token = LexItem(Token::ASSOP, lexeme, linenum);
        return token;
    }
    // Otherwise, return COLON token
    token = LexItem(Token::COLON, lexeme, linenum);
    return token;
}

void skipComment(std::istream& in, int& linenum, char& currChar, char& nextChar) {
    while (currChar != std::ifstream::traits_type::eof()) {
        currChar = in.get();
        if (currChar == '\n') {
            ++linenum;
            continue;
        }
        else if (currChar == '}') {
            // We don't want } to be interpreted as an ERR token.
//            currChar = in.get();
            break;
        }
    }
}

LexItem getNextToken(istream&in, int& linenum) {
    char currChar;
	char nextChar;
	std::string lexeme = "";

    LexItem token;

	Token oneCharToken;			// In case currChar is a token in itself
	IdentChar identCharType;	// In case currChar is the beginning of an identifier

	do {
	    currChar = in.get();
	    if (currChar == '\n') {
//	        // '\n' followed by '\n' or EOF should not increment linenum
//	        nextChar = in.peek();
//	        if ((nextChar == '\n') || (nextChar == std::ifstream::traits_type::eof())) {
//                continue;
//	        }
			++linenum;
			continue;
		}
		else if (isWhiteSpace(currChar)) { continue; }
		// At this point, currChar should not be a whitespace or newline.

		// If 1st character is {, skip remaining characters until we reach } or EOF.
		if (currChar == '{') {
            skipComment(in, linenum, currChar, nextChar);
            continue;
		}
		// If the 1st character is a single quote, lexeme could be an SCONST token.
		if (currChar == '\'') {
            token = readStr(in, lexeme, linenum, currChar, nextChar);
            return token;
		}
		// If a single character is a token (operator, delimiter, etc.),
        // return a corresponding LexItem immediately.
        oneCharToken = checkSymbol(currChar);
        // break immediately if EOF is reached
        if (oneCharToken == Token::DONE) {break;}
        else if (oneCharToken == Token::COLON) {
            // Could either be COLON or ASSOP
            token = readAssopOrColon(in, lexeme, linenum, currChar, nextChar);
            return token;
        }
        else if (oneCharToken == Token::ERR) {
            // Char is either a letter, digit, _, $, single quote,
            // or some unrecognized symbol.
            // Check if char belongs in an identifier.
            identCharType = checkIdent(currChar);
            // A valid identifier must start with a letter
            if (identCharType == IdentChar::LETTER) {
                readIdent(in, lexeme, linenum, currChar, nextChar);
                // Could either be IDENT, keyword, or BCONST (keywords 'true' or 'false')
                token = id_or_kw(lexeme, linenum);
                return token;
            }
            // Number constant (real or int) starts with a digit
            else if (identCharType == IdentChar::DIGIT) {
                token = readNum(in, lexeme, linenum, currChar, nextChar);
                return token;
            }
            // Token begins with '_' or '$' (invalid), or
            // token is an unrecognized character (i.e. '?').
            // Append curr char to lexeme and return an ERR token.
            else {
                lexeme += currChar;
                token = LexItem(Token::ERR, lexeme, linenum);
                return token;
            }
        }
        // Char is not :, letter, digit, _, $, EOF, or unrecognized,
        // so it must be a one character token.
        else {
            lexeme += currChar;
            token = LexItem(oneCharToken, lexeme, linenum);
            return token;
        }

	} while (currChar != std::ifstream::traits_type::eof());

	token = LexItem(Token::DONE, lexeme, linenum);
    return token;
}

LexItem id_or_kw(const string& lexeme, int linenum) {
    LexItem token;
    Token foundKeyword;
    std::map<std::string, Token> StrToKeyword = {
        {"and", Token::AND},
        {"begin", Token::BEGIN},
        {"boolean", Token::BOOLEAN},
        {"div", Token::IDIV},       // symbol '/' is associated with the DIV token
        {"end", Token::END},
        {"else", Token::ELSE},
        {"false", Token::FALSE},    // treated as BCONST
        {"if", Token::IF},
        {"integer", Token::INTEGER},
        {"mod", Token::MOD},
        {"not", Token::NOT},
        {"or", Token::OR},
        {"program", Token::PROGRAM},
        {"real", Token::REAL},
        {"string", Token::STRING},
        {"true", Token::TRUE},      // treated as BCONST
        {"then", Token::THEN},
        {"write", Token::WRITE},
        {"writeln", Token::WRITELN},
        {"var", Token::VAR}
    };
    auto keywordMatch = StrToKeyword.find(lexeme);
    if (keywordMatch != StrToKeyword.end()) {
        foundKeyword = keywordMatch->second;
        // Keywords 'true' or 'false' are both recognized as BCONST
        if ((foundKeyword == Token::TRUE) || (foundKeyword == Token::FALSE)) {
            token = LexItem(Token::BCONST, lexeme, linenum);
            return token;
        }
        // Lexeme is a keyword
        token = LexItem(foundKeyword, lexeme, linenum);
        return token;
    }
    // Lexeme is an identifier
    token = LexItem(Token::IDENT, lexeme, linenum);
    return token;
}

std::string trimStr(std::string lexeme) {
    // Remove single quotes from SCONST.
    // Returns a new string instead of directly
    // modifying the lexeme, in case the original
    // lexeme should be preserved.
    std::string newStr = lexeme;
    if ((newStr.front() == '\'') && (newStr.back() == '\'')) {
        newStr.erase(0,1);  // 1st char
        newStr.pop_back();
    }
    return newStr;
}

ostream& operator<<(ostream& out, const LexItem& tok) {
    std::map<Token, std::string> TokToStr = {
		// Keywords or reserved words
		{Token::IF, "IF"},
		{Token::ELSE, "ELSE"},
		{Token::WRITELN, "WRITELN"},
		{Token::WRITE, "WRITE"},
		{Token::INTEGER, "INTEGER"},
		{Token::REAL, "REAL"},
		{Token::BOOLEAN, "BOOLEAN"},
		{Token::STRING, "STRING"},
		{Token::BEGIN, "BEGIN"},
		{Token::END, "END"},
		{Token::VAR, "VAR"},
		{Token::THEN, "THEN"},
		{Token::PROGRAM, "PROGRAM"},

		// Identifiers
		{Token::IDENT, "IDENT"},
		{Token::TRUE, "TRUE"},
		{Token::FALSE, "FALSE"},

		// Integer, real, string, boolean constants
		{Token::ICONST, "ICONST"},
		{Token::RCONST, "RCONST"},
		{Token::SCONST, "SCONST"},
		{Token::BCONST, "BCONST"},

		// Arithmetic, logical, relational operators
		{Token::PLUS, "PLUS"},
		{Token::MINUS, "MINUS"},
		{Token::MULT, "MULT"},
		{Token::DIV, "DIV"},
		{Token::IDIV, "IDIV"},
		{Token::MOD, "MOD"},
		{Token::ASSOP, "ASSOP"},
		{Token::EQ, "EQ"},
		{Token::GTHAN, "GTHAN"},
		{Token::LTHAN, "LTHAN"},
		{Token::AND, "AND"},
		{Token::OR, "OR"},
		{Token::NOT, "NOT"},

		// Delimiters
		{Token::COMMA, "COMMA"},
		{Token::SEMICOL, "SEMICOL"},
		{Token::LPAREN, "LPAREN"},
		{Token::RPAREN, "RPAREN"},
		{Token::DOT, "DOT"},
		{Token::COLON, "COLON"},

		// Not sure if ERR and DONE need their own strings...
		// Error
		{Token::ERR, "ERR"},

		// End of file
		{Token::DONE, "DONE"}
	};
	Token type = tok.GetToken();
	std::string lexeme = tok.GetLexeme();
	std::string tokStr = "";

	auto foundMatch = TokToStr.find(type);
	if (foundMatch != TokToStr.end()) {
        tokStr = foundMatch->second;
	}

	if ((type == Token::IDENT) || (type == Token::ICONST) || (type == Token::RCONST) || (type == Token::BCONST)) {
	    out << tokStr << ": \"" << lexeme << "\"\n";
	}
	else if (type == Token::SCONST) {
	    out << tokStr << ": \"" << trimStr(lexeme) << "\"\n";
	}
	else if (type == Token::ERR) {
        out << "Error in line " << tok.GetLinenum() << ": Unrecognized Lexeme {" << lexeme << "}\n";
        exit(1);
	}
	else if (type == Token::DONE) {
        // If <-v> is not used, then printSummary() should print '\n' first.
        ;
	}
	else {
        out << tokStr << '\n';
	}

    return out;
}
