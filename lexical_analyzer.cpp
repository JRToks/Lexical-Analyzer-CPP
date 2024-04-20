#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

// Function for checking list of keywords
bool isKeyword(string text) {
  static const unordered_set<string> keywords = {
      "auto",     "break",    "bool",   "case",    "char",   "const",
      "continue", "default",  "do",     "double",  "else",   "enum",
      "extern",   "float",    "for",    "goto",    "if",     "int",
      "long",  "namespace",   "register", "return", "short",   "signed", "sizeof",
      "static",   "struct",   "switch", "typedef", "union",  "unsigned", "using",
      "void",     "volatile", "while"};

  return keywords.find(text) != keywords.end();
}

// Function for checking bool literals #TODO fix boolen literal logic
bool isBoolLiteral(string text) {
  if ((text.substr(0, 4) == "true" && text.size() == 4) ||
      (text.substr(0, 5) == "false" && text.size() == 5)) {
    return true;
  }

  return false;
}

// Function for checking char literals
bool isCharLiteral(string text) {
  if ((text[0] == '\'' && text[text.size() - 1] == '\''))
    return true;

  return false;
}

// Function for checking string literals
bool isStringLiteral(string text) {
  if (text[0] == '"' && text[text.size() - 1] == '"')
    return true;

  return false;
}

// Function for checking integer literals
bool isIntegerLiteral(string text) {
  size_t decimalPointIndex = text.find('.');
  if (isdigit(text[0]) && decimalPointIndex == string::npos) {
    if (!isdigit(text[text.length() - 1]))
      return true;
  }

  return false;
}

// function for checking float literals
bool isFloatLiteral(string text) {
  size_t decimalPointIndex = text.find('.');
  if (isdigit(text[0]) && decimalPointIndex != string::npos &&
      decimalPointIndex != text.length() - 1) {
    size_t secondDecimalPointIndex = text.find('.', decimalPointIndex + 1);
    if (!isdigit(text[text.length() - 1]) ||
        (secondDecimalPointIndex != string::npos &&
         text[text.length() - 1] == '.'))
      return true;
  }

  return false;
}

// Function for checking identifiers
bool isIdentifier(string text) {
  if (isalpha(text[0]) ||
      text[0] == '_') { // checks if first character is a letter or underscore
    if (isalnum(text[text.size() - 1]) ||
        text[text.size() - 1] ==
            '_') { // return true if last character is a letter or an underscore
      return true;
    }
  }
  return false;
}

// Function for checking punctuations
bool isPunc(string text) {
  static const unordered_set<string> specialChar = {
      "(", ")", "()", "{", "}", "{}", "[", "]", "[]", ",", ";", ":", ".", "#"};

  return specialChar.find(text) != specialChar.end();
}

// Function for checking operators
bool isOperator(string text) {
  static const unordered_set<string> operators = {
      "+",  "-",  "*",  "/",  "=",  "<",  ">",  "<<",     ">>", "&",
      "|",  "%",  "!",  "^",  "++", "--", "+=", "-=",     "*=", "/=",
      "%=", "&=", "|=", "^=", "==", "!=", ">",  "<",      ">=", "<=",
      "&&", "||", "~",  "->", ",",  "&",  "*",  "sizeof", "?"};

  return operators.find(text) != operators.end();
}

// Function for checking preprocessor directives
bool isPreProc(string text) {
  static const unordered_set<string> preProc = {
      "#include", "#define", "#undef",  "#if",    "#elif",  "#else",
      "#endif",   "#ifdef",  "#ifndef", "#error", "#pragma"};

  return preProc.find(text) != preProc.end();
}

// Function to tokenize input text
void tokenize(string &text) {
  vector<pair<string, string>> tokens;
  int len;

  do {
    len = text.size(); // dynamically update the length of the text string
    try {
      bool foundPreproc = false, foundLib = false;
      string temp = "";
      int Mtemplength = 0;
      if (len > 0) {
        if (isspace(text[0])) { // check if whitespace then erase whitespace
          text.erase(0, 1);
        } else if (isPunc(text.substr(
                       0,
                       1))) { // check for punctuations and preprocessors token
          for (int i = 1; i <= len; ++i) {
            string candidate = text.substr(0, i);
            if (isPreProc(candidate)) { // Check if the candidate substring is a preprocessor directive
              foundPreproc = true;
              temp = candidate;
              Mtemplength = temp.length() + 2;
            } else if (i == len) { // Reached end of string if i == len
              break;
            } else if (foundPreproc) { // If a preprocessor directive is found. Check if the current candidate substring indicates a library inclusion
              if ((isspace(candidate[Mtemplength - 1]) &&
                   candidate.length() <= Mtemplength) ||
                  (candidate[i - 1] == '<' &&
                   candidate.length() <= Mtemplength)) {
                if (candidate[Mtemplength - 1] == '<' ||
                    candidate[i - 1] == '<') { // if < is found then set flag for library inclusion found
                  foundLib = true;
                }
              } else if (foundLib) { // If a library inclusion is found Check if the end of the library inclusion is reached
                if (candidate[i - 1] == '>') {
                  tokens.emplace_back("PREPROCESSOR", candidate);
                  text.erase(0, i);
                  break;
                }
              } else if ((!foundLib && candidate.length() >= Mtemplength) ||
                         (!foundLib && isalnum(candidate[i - 1]))) {
                // If no library inclusion is found, add the preprocessor directive to tokens
                tokens.emplace_back("PREPROCESSOR", temp);
                text.erase(0, temp.length());
                break;
              }
            }
          }
          // If no preprocessor
          if (!foundPreproc) {
            if (isPunc(text.substr(0, 2))) {
              tokens.emplace_back("SYMBOL", text.substr(0, 2));
              text.erase(0, 2);
            } else {
              tokens.emplace_back("SYMBOL", text.substr(0, 1));
              text.erase(0, 1);
            }
          }
        } else if (isOperator(text.substr(0, 1))) { // Check for operator token
          if (isOperator(text.substr(0, 2))) {
            tokens.emplace_back("OPERATOR", text.substr(0, 2));
            text.erase(0, 2);
          } else {
            tokens.emplace_back("OPERATOR", text.substr(0, 1));
            text.erase(0, 1);
          }
        } else { // iteratively finds each word in the string then store to temp variable
          for (int i = 1; i <= len; i++) {
            string candidate = text.substr(0, i);
            int cdsize = candidate.size();

            bool isSingleChar = cdsize != 1;
            bool isAlphanumStartQuoteEnd =
                (candidate[0] != '"' && candidate[0] != '\'') &&
                (candidate[i - 1] == '\'' || candidate[i - 1] == '"');
            bool isValidCandidate =
                (candidate[cdsize - 1] == ' ' ||
                 (isPunc(string(1, candidate[cdsize - 1])) &&
                  !isdigit(candidate[0])) ||
                 isOperator(string(1, candidate[cdsize - 1]))) &&
                (candidate[0] != '"' && candidate[0] != '\'');
            bool isQuotedAtEnd =
                candidate.size() > 2 &&
                (candidate[cdsize - 2] == '"' || candidate[cdsize - 2] == '\'');
            bool isNewline = candidate[cdsize - 1] == '\n';

            if (isSingleChar && isIntegerLiteral(candidate) ||
                isFloatLiteral(candidate) || isAlphanumStartQuoteEnd ||
                isValidCandidate || isQuotedAtEnd || isNewline) {
              temp = candidate.substr(0, i - 1);
              break;
            }
          }

          if (isKeyword(temp)) {
            tokens.emplace_back("KEYWORD", temp);
            text.erase(0, temp.size());
          } else if (isBoolLiteral(temp)) {
            tokens.emplace_back("BOOLEAN_LITERAL", temp);
            text.erase(0, temp.size());
          } else if (isCharLiteral(temp)) {
            tokens.emplace_back("CHAR_LITERAL", temp);
            text.erase(0, temp.size());
          } else if (isStringLiteral(temp)) {
            tokens.emplace_back("STRING_LITERAL", temp);
            text.erase(0, temp.size());
          } else if (isdigit(temp[0]) && temp.find('.') == string::npos) {
            tokens.emplace_back("INT_LITERAL", temp);
            text.erase(0, temp.size());
          } else if (temp.find('.') != string::npos) {
            tokens.emplace_back("FLOAT_LITERAL", temp);
            text.erase(0, temp.size());
          } else if (isOperator(temp)) { // Checks "sizeof" operator
            tokens.emplace_back("OPERATOR", temp);
            text.erase(0, temp.size());
          } else if (isIdentifier(temp)) {
            tokens.emplace_back("IDENTIFIER", temp);
            text.erase(0, temp.size());
          } else if (temp.size() == len - 1) {
            tokens.emplace_back("OTHER", temp);
            text.erase(0, temp.size());
          }
        }
      }
    } catch (exception e) {
      /* Do nothing */
    }
  } while (text.size() > 0);

  // Print each token separately
  for (const auto &token : tokens) {
    cout << "Token(Type: " << token.first << ", Value: '" << token.second
         << "')\n";
  }

  int totalTokens = tokens.size();
  cout << "\nTotal number of tokens: " << totalTokens << endl;
}

int main() {
  int choice;
  string code, filename;
  cout << "--------------------------------\n";
  cout << "\tLexical Analyzer\n";
  cout << "--------------------------------\n";
  cout << "1. Read from file\n";
  cout << "2. Read from user input\n";
  cout << "Enter your choice: ";
  cin >> choice;
  if (choice == 1) {
    // use file as an input
    cin.ignore();
    cout << "--------------------------------\n";
    cout << "Enter file name (include extension): ";
    getline(cin, filename);
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
      cerr << "Failed to open " << filename << endl;
      return 1;
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();
    code = buffer.str();

    tokenize(code);
    inputFile.close();
  } else if (choice == 2) {
    // use cin as an input
    cin.ignore();
    cout << "--------------------------------\n";
    cout << "Enter your code below (press Enter followed by Ctrl+D to "
            "finish):\n";
    getline(cin, code, '\0');
    tokenize(code);
  } else {
    cout << "Invalid choice. Please try again.\n";
  }

  return 0;
}