# include <ctype.h>
# include <iostream>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <vector>

using namespace std;

static bool uDebug = false;
static bool uDebug2 = false;
static bool uDebug3 = false;
static bool uDebug4 = false;

// 用於標示token型態
enum TYPE {
  //       1                2    3      4    5      6
  LEFT_PAREN = 1, RIGHT_PAREN, DOT, QUOTE, INT, FLOAT,
  STRING, NIL, T, SYMBOL, ATOM, S_EXP, LS_EXP, REPLACE
  //   7    8  9      10    11     12      13       14
};

enum ERROR_READ {
  //                                  41                                       42
  ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN = 41, ERROR_UNEXPECTED_TOKEN_RIGHT_PAREN,
  ERROR_NO_CLOSING_QUOTE, ERROR_NO_MORE_INPUT
  //                  43                   44
};

enum ERROR_EVAL
{
  ERROR_UNBOUND_SYMBOY = 51,
  ERROR_NON_LIST,
  ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION,
  ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION2,
  ERROR_LEVEL_OF_CLEAN_ENVIRONMENT, ERROR_LEVEL_OF_DEFINE, ERROR_LEVEL_OF_EXIT,
  ERROR_INCORRECT_NUMBER_OF_ARGUMENTS,
  ERROR_DEFINE_FORMAT, ERROR_COND_FORMAT, ERROR_LAMBDA_FORMAT, ERROR_LET_FORMAT,
  ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE,
  ERROR_DIVISION_BY_ZERO,
  ERROR_NO_RETURN_VALUE,
  ERROR_NO_RETURN_VALUE_TO_MAIN,
  ERROR_UNBOUND_PARAMETER, ERROR_UNBOUND_CONDITION
};

struct Token_Type
{
  string original;
  int type;
  string changed;
  int tokenLine;
  int tokenColume;
  string replaced;
};

struct PL_Tree
{
  int type;
  PL_Tree *left;
  PL_Tree *right;
  Token_Type isAtomString;
  PL_Tree *parent;
};

struct Defined_Symbol
{
  string original_symbol;
  int type;
  PL_Tree *translate_PL_Tree;
  bool ifQuote;
};

struct Defined_Function
{
  string functionName;
  vector<string> functionArgument;
  int numOfArgument;
  vector<PL_Tree *> translate_PL_Tree_List;
  PL_Tree *translate_PL_Tree;
};

struct Lambda_Function
{
  int lambdaArgumentNum;
  vector<string> lambdaArgumentName;
  PL_Tree *lambda_PL_Tree;
};

static int uLine;
static int uColume;
static int uTokenLine;
static int uTokenColume;
static int uTotalLine;
static int uPrePrintLine;
static bool uUnAddTotalLine;
static bool uPrePrintSexpInSameLine;
static bool uIsReadSexp;
static bool uEOL;
static bool uUnusedCharBool;
static char uUnusedChar;
static char uLastChar;
static bool uEOF;
static bool uExit;
static Token_Type uErrorToken;
static PL_Tree *uErrorTree;
static vector<Defined_Symbol> uDefined_List;
static vector<Defined_Function> uDefined_Function_List;
static vector<string> uInternalFunction;
static int uLambdaArgumentNum;
static vector<string> uLambdaArgumentName;
static PL_Tree * uLambda_PL_Tree;
static int uLet_Defined_Num;
static bool uLambdaUse;
static vector<Lambda_Function> uLambda_Function_List;

PL_Tree *Eval( PL_Tree *originalRoot, int level, bool &ifQuote ) ;

void GetLine() {
  char temp = '\0';
  do {
    temp = getchar();
    uLastChar = temp;
  } while ( temp != '\n' && temp != EOF );

  if ( temp == EOF ) {
    throw ERROR_NO_MORE_INPUT;
  } // end if ...
  else {
    uLine++;
    uColume = 0;
    uTotalLine++;
  } // end else ...

} // GetLine()

string GetToken() {

  if ( uUnAddTotalLine ) {
    uTotalLine++;
    uUnAddTotalLine = false;
  } // end if ...

  string tempToken = "";

  // 如果有尚未處裡的separator字元就存取
  if ( uUnusedCharBool ) {
    tempToken = uUnusedChar;
    uUnusedCharBool = false;
    uPrePrintSexpInSameLine = false;
    // cout << "Line : 204 " << tempToken << endl;
    uTokenLine = uLine;
    uTokenColume = uColume;
    if ( tempToken != "\"" ) return tempToken;
  } // end if ...

  char tempChar = '\0';

  bool continuteGetToken = true;

  do {

    tempChar = getchar();
    uLastChar = tempChar;
    uColume++;

    uTokenLine = uLine;
    uTokenColume = uColume;

    if ( tempChar == EOF ) {
      // cout << "wtfbj4\n";
      if ( tempToken.empty() ) throw ERROR_NO_MORE_INPUT;
    } // end if ...

    if ( isspace( tempChar ) || tempChar == EOF ) {
      if ( tempToken.length() > 0 ) {
        if ( tempToken[0] == '"' ) {
          if ( tempChar == '\n' || tempChar == EOF ) {
            continuteGetToken = false;
            uLine = 1;
            uColume = 0;
            uUnAddTotalLine = true;
          } // end if ...
          else {
            tempToken = tempToken + tempChar;
          } // end else ...
        } // end if ...
        else {
          continuteGetToken = false;
          if ( tempChar == '\n' ) {
            uLine++;
            uColume = 0;
            uTokenColume--;
            uUnAddTotalLine = true;
          } // end if ...
          else {
            uPrePrintSexpInSameLine = true;
            uTokenColume--;
          } // end else ...
        } // end else ...
      } // end if ...
      else {
        if ( tempChar == '\n' ) {
          if ( uIsReadSexp ) {
            uLine++;
          } // end if ...
          else {
            if ( uPrePrintLine == uTotalLine ) {
              uLine = 1;
            } // end if ...
            else {
              uLine++;
            } // end else ...
          } // end else ...

          uColume = 0;
          uTotalLine++;
        } // end if ...
      } // end else ...
    } // end if ...
    else if ( tempChar == '(' || tempChar == ')' || tempChar == '\'' ) {
      if ( tempToken.length() > 0 ) {
        if ( tempToken[0] == '"' ) {
          tempToken = tempToken + tempChar;
        } // end if ...
        else {
          continuteGetToken = false;
          uUnusedCharBool = true;
          uUnusedChar = tempChar;
          uTokenColume--;
          uPrePrintSexpInSameLine = true;
        } // end else ...
      } // end if ...
      else {
        tempToken = tempToken + tempChar;
        continuteGetToken = false;
        uPrePrintSexpInSameLine = false;
      } // end else ...
    } // end else if ...
    else if ( tempChar == '"' ) {
      if ( tempToken.length() > 0 ) {
        if ( tempToken[0] == '"' ) {
          if ( tempToken[tempToken.length() - 1] != '\\' ) continuteGetToken = false;

          tempToken = tempToken + tempChar;
        } // end if ...
        else {
          continuteGetToken = false;
          uUnusedCharBool = true;
          uUnusedChar = tempChar;
        } // end else ...
      } // end if ...
      else {
        tempToken = tempToken + tempChar;
      } // end else ...
    } // end else if ...
    else if ( tempChar == ';' ) {
      if ( tempToken.length() > 0 ) {
        if ( tempToken[0] == '"' ) {
          tempToken = tempToken + tempChar;
        } // end if ...
        else {
          continuteGetToken = false;
          GetLine();

        } // end else ...
      } // end if ...
      else {
        if ( uPrePrintLine == uTotalLine && !uIsReadSexp ) {
          uLine = 0;
        } // end if ...

        GetLine();
      } // end else ...
    } // end else if ...
    else {
      tempToken = tempToken + tempChar;
    } // end else ...

  } while ( continuteGetToken );

  if ( uDebug ) {
    printf( "Get a token : %s\n", tempToken.c_str() );
    cout << uTokenLine << endl << uTokenColume << endl;
  } // end if ...

  if ( uDebug ) cout << "printLine : " << uPrePrintLine << endl;
  return tempToken;
} // GetToken()

PL_Tree *ToBeQuote( PL_Tree *root ) {

  PL_Tree *temp = new PL_Tree;
  temp->parent = NULL;
  temp->right = NULL;
  temp->type = QUOTE;
  temp->isAtomString.changed = "'";
  temp->isAtomString.type = QUOTE;

  temp->left = root;

  return temp;

} // ToBeQuote()

bool IsExit( PL_Tree *root ) {

  if ( root->type == LEFT_PAREN ) {
    if ( root->left->isAtomString.changed == "exit" ) {
      if ( root->right == NULL ) {
        uExit = true;
        return true;
      } // end if ...
      else {
        if ( root->right->isAtomString.changed == "nil" ) {
          uExit = true;
          return true;
        } // end if ...
      } // end else ...
    } // end if ...
  } // end if ...

  return false;
} // IsExit()

string ReturnType( int i ) {

  if ( i == 1 )
    return "LEFT_PAREN";
  else if ( i == 2 )
    return "RIGHT_PAREN";
  else if ( i == 3 )
    return "DOT";
  else if ( i == 4 )
    return "QUOTE";
  else if ( i == 5 )
    return "INT";
  else if ( i == 6 )
    return "FLOAT";
  else if ( i == 7 )
    return "STRING";
  else if ( i == 8 )
    return "NIL";
  else if ( i == 9 )
    return "T";
  else if ( i == 10 )
    return "SYMBOL";
  else
    return "Fuck!!!";
} // ReturnType()

bool IsINT( string original ) {

  if ( original.length() > 1 ) {
    if ( original[0] == '+' || original[0] == '-' ) {
      for ( int i = 1 ; i < original.length() ; i++ ) {
        if ( !isdigit( original[i] ) ) return false;
      } // end for ...

      return true;
    } // end if ...
    else {
      for ( int i = 0 ; i < original.length() ; i++ ) {
        if ( !isdigit( original[i] ) ) return false;
      } // end for ...

      return true;
    } // end else ...
  } // end if ...
  else if ( original.length() == 1 ) {
    if ( isdigit( original[0] ) ) return true;
    else return false;
  } // end else if ...

  return false;
} // IsINT()

bool IsFLOAT( string original ) {

  bool dotHad = false;

  if ( original.length() > 2 ) {
    if ( original[0] == '+' || original[0] == '-' ) {
      for ( int i = 1 ; i < original.length() ; i++ ) {
        if ( original[i] == '.' ) {
          if ( dotHad ) return false;
          else dotHad = true;
        } // end if ...
        else if ( !isdigit( original[i] ) ) return false;
      } // end for ...

      return true;
    } // end if ...
    else {
      for ( int i = 0 ; i < original.length() ; i++ ) {
        if ( original[i] == '.' ) {
          if ( dotHad ) return false;
          else dotHad = true;
        } // end if ...
        else if ( !isdigit( original[i] ) ) return false;
      } // end for ...

      return true;
    } // end else ...
  } // end if ...
  else if ( original.length() == 2 ) {
    for ( int i = 0 ; i < original.length() ; i++ ) {
      if ( original[i] == '.' ) {
        if ( dotHad ) return false;
        else dotHad = true;
      } // end if ...
      else if ( !isdigit( original[i] ) ) return false;
    } // end for ...

    return true;
  } // end else if ...

  return false;
} // IsFLOAT()

string ReturnRealString( string original ) {

  string temp;

  for ( int i = 0 ; i < original.length() ; i++ ) {
    if ( original[i] != '\\' ) {
      temp = temp + original[i];
    } // end if ...
    else {
      if ( i + 1 < original.length() ) {
        if ( original[i + 1] == 'n' ) temp = temp + "\n";
        else if ( original[i + 1] == 't' ) temp = temp + "\t";
        else if ( original[i + 1] == '\\' ) temp = temp + "\\";
        else if ( original[i + 1] == '"' ) temp = temp + "\"";
        else {
          temp = temp + original[i];
          i--;
        } // end else ...

        i++;
      } // end if ...
    } // end else ...
  } // end for ...

  return temp;
} // ReturnRealString()

Token_Type IsWhatType( string original ) {

  if ( uDebug ) cout << "Enter isWhatType\n";

  Token_Type tempToken_Type;
  tempToken_Type.original = original;
  tempToken_Type.type = 0;
  tempToken_Type.changed = "error";
  tempToken_Type.tokenLine = uTokenLine;
  tempToken_Type.tokenColume = uTokenColume;

  do {

    // LEFT_PAREN
    // reg = "\\("
    if ( original == "(" ) {
      tempToken_Type.type = LEFT_PAREN;
      tempToken_Type.changed = "(";
    } // end if ...
      // RIGHT_PAREN
      // reg = "\\)";
    else if ( original == ")" ) {
      tempToken_Type.type = RIGHT_PAREN;
      tempToken_Type.changed = ")";
    } // end else if ...
      // INT
      // reg = "[+-]?[0-9]+";
    else if ( IsINT( original ) ) {
      tempToken_Type.type = INT;
      int i; // temp
      sscanf( original.c_str(), "%d", &i );
      char *cs = new char[64];
      sprintf( cs, "%d", i );
      tempToken_Type.changed = cs;
    } // end else if ...
      // STRING
      // reg = "^\".*\"$";
    else if ( original.length() >= 2 && original[0] == '\"'
              && original[original.length() - 1] == '\"' ) {
      tempToken_Type.type = STRING;
      tempToken_Type.changed = ReturnRealString( original );
    } // end else if ...
      // DOT
      // reg = "\\.";
    else if ( original == "." ) {
      tempToken_Type.type = DOT;
      tempToken_Type.changed = tempToken_Type.original;
    } // end else if ...
      // FLOAT
      // reg = "([+-]?[0-9]+\\.[0-9]+)|([+-]?\\.[0-9]+)|([+-]?[0-9]+\\.)";
    else if ( IsFLOAT( original ) ) {
      tempToken_Type.type = FLOAT;
      double f; // temp
      sscanf( original.c_str(), "%lf", &f );
      char *cs = new char[64];
      sprintf( cs, "%.3f", f );
      tempToken_Type.changed = cs;
    } // end else if ...
      // NIL
      // reg = "(^nil$)|(^#f$)";
    else if ( original == "nil" || original == "#f" ) {
      tempToken_Type.type = NIL;
      tempToken_Type.changed = "nil";
    } // end else if ...
      // T
      // reg = "(^t$)|(^#t$)";
    else if ( original == "t" || original == "#t" ) {
      tempToken_Type.type = T;
      tempToken_Type.changed = "#t";
    } // end else if ...
      // QUOTE
      // reg = "^'$";
    else if ( original == "'" ) {
      tempToken_Type.type = QUOTE;
      tempToken_Type.changed = tempToken_Type.original;
    } // end else if ...
      // SYMBOL
    else {
      tempToken_Type.type = SYMBOL;
      tempToken_Type.changed = tempToken_Type.original;
    } // end else ...

  } while ( false );

  if ( uDebug ) {
    cout << tempToken_Type.original << endl;
    cout << tempToken_Type.type << endl;
    cout << tempToken_Type.changed << endl;
    cout << tempToken_Type.tokenColume << endl;
  } // end if ...

  return tempToken_Type;
} // IsWhatType()

void ReadSExp( PL_Tree *nextNode, string unusedToken ) {

  nextNode->left = NULL;
  nextNode->right = NULL;

  string tempString;
  if ( unusedToken.empty() ) {
    do {
      tempString = GetToken();
    } while ( tempString.empty() );
  } // end if ...
  else {
    tempString = unusedToken;
  } // end else ...

  Token_Type tempTest;
  tempTest = IsWhatType( tempString );

  // 進階節點

  if ( nextNode->type == S_EXP || nextNode->type == LS_EXP ) {

    if ( tempTest.type == RIGHT_PAREN ) {
      nextNode->type = ATOM;
      nextNode->left = NULL;
      nextNode->right = NULL;
      nextNode->isAtomString.original = "()";
      nextNode->isAtomString.type = NIL;
      nextNode->isAtomString.changed = "nil";
      nextNode->isAtomString.tokenLine = tempTest.tokenLine;
      nextNode->isAtomString.tokenColume = tempTest.tokenColume;
      return;
    } // end if ...
    else {
      nextNode->left = new PL_Tree;
      nextNode->left->parent = nextNode;
      ReadSExp( nextNode->left, tempString );
    } // end else ...

    do {
      tempString = GetToken();
    } while ( tempString.empty() );

    tempTest = IsWhatType( tempString );
    if ( tempTest.type == RIGHT_PAREN ) {
      return;
    } // end if ...
    else {
      nextNode->right = new PL_Tree;
      nextNode->right->parent = nextNode;
      nextNode->right->type = S_EXP;
      if ( tempTest.type == DOT ) {
        do {
          tempString = GetToken();
        } while ( tempString.empty() );

        tempTest = IsWhatType( tempString );
        if ( tempTest.type == LEFT_PAREN ) {
          nextNode->right->type = LS_EXP;
          ReadSExp( nextNode->right, "" );
        } // end if ...
        else if ( tempTest.type == RIGHT_PAREN ) {
          uErrorToken = tempTest;
          throw ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN;
        } // end else if ...
        else {
          nextNode->right->type = ATOM;
          ReadSExp( nextNode->right, tempString );
        } // end else ...
      } // end if ...
      else ReadSExp( nextNode->right, tempString );
    } // end else ...

    if ( nextNode->right->type == S_EXP ) return;

    do {
      tempString = GetToken();
    } while ( tempString.empty() );

    tempTest = IsWhatType( tempString );
    if ( tempTest.type == RIGHT_PAREN ) {
      return;
    } // end if ...
    else {
      uErrorToken = tempTest;
      throw ERROR_UNEXPECTED_TOKEN_RIGHT_PAREN; // 沒有右括號的錯誤
    } // end else ...
  } // end if ...

  // end 進階節點 ===================================================

  // 基本節點 =======================================================

  if ( tempTest.type >= INT && tempTest.type < SYMBOL ) {
    uIsReadSexp = true;
    nextNode->type = ATOM;
    nextNode->isAtomString = tempTest;
    nextNode->left = NULL;
    nextNode->right = NULL;
    return;
  } // end if ...
  else if ( tempTest.type == SYMBOL ) {
    uIsReadSexp = true;
    if ( tempTest.changed[0] == '"' ) {
      uErrorToken = tempTest;
      throw ERROR_NO_CLOSING_QUOTE; // 沒有雙引號結尾的錯誤
    } // end if ...
    else {
      nextNode->type = SYMBOL;
      nextNode->isAtomString = tempTest;
      nextNode->left = NULL;
      nextNode->right = NULL;
      return;
    } // end else ...
  } // end else if ...
  else if ( tempTest.type == LEFT_PAREN ) {
    uIsReadSexp = true;
    nextNode->isAtomString = tempTest;

    do {
      tempString = GetToken();
    } while ( tempString.empty() );

    tempTest = IsWhatType( tempString );
    if ( tempTest.type == RIGHT_PAREN ) {
      nextNode->type = ATOM;
      nextNode->left = NULL;
      nextNode->right = NULL;
      nextNode->isAtomString.original = "()";
      nextNode->isAtomString.type = NIL;
      nextNode->isAtomString.changed = "nil";
      nextNode->isAtomString.tokenLine = tempTest.tokenLine;
      nextNode->isAtomString.tokenColume = tempTest.tokenColume;
      return;
    } // end if ...
    else if ( tempTest.type == DOT ) {
      uErrorToken = tempTest;
      throw ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN;
    } // end else if ...
    else {
      nextNode->type = LEFT_PAREN;
      nextNode->left = new PL_Tree;
      nextNode->left->parent = nextNode;
      ReadSExp( nextNode->left, tempString );
    } // end else ...

    do {
      tempString = GetToken();
    } while ( tempString.empty() );

    tempTest = IsWhatType( tempString );
    if ( tempTest.type == RIGHT_PAREN ) {
      return;
    } // end if ...
    else {
      nextNode->right = new PL_Tree;
      nextNode->right->parent = nextNode;
      nextNode->right->type = S_EXP;
      if ( tempTest.type == DOT ) {
        do {
          tempString = GetToken();
        } while ( tempString.empty() );

        tempTest = IsWhatType( tempString );
        if ( tempTest.type == LEFT_PAREN ) {
          nextNode->right->type = LS_EXP;
          ReadSExp( nextNode->right, "" );
        } // end if ...
        else if ( tempTest.type == RIGHT_PAREN ) {
          uErrorToken = tempTest;
          throw ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN;
        } // end else if ...
        else {
          nextNode->right->type = ATOM;
          ReadSExp( nextNode->right, tempString );
        } // end else ...
      } // end if ...
      else ReadSExp( nextNode->right, tempString );
    } // end else ...

    if ( nextNode->right->type == S_EXP ) return;

    do {
      tempString = GetToken();
    } while ( tempString.empty() );

    tempTest = IsWhatType( tempString );
    if ( tempTest.type == RIGHT_PAREN ) {
      return;
    } // end if ...
    else {
      uErrorToken = tempTest;
      throw ERROR_UNEXPECTED_TOKEN_RIGHT_PAREN; // 沒有右括號的錯誤
    } // end else ...

  } // end else if ...
  else if ( tempTest.type == DOT ) {
    uIsReadSexp = true;
    uErrorToken = tempTest;
    throw ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN;
  } // end else if ...
  else if ( tempTest.type == RIGHT_PAREN ) {
    uIsReadSexp = true;
    uErrorToken = tempTest;
    throw ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN;
  } // end else if ...
  else if ( tempTest.type == QUOTE ) {
    nextNode->type = QUOTE;
    nextNode->isAtomString = tempTest;
    nextNode->left = new PL_Tree;
    nextNode->right = NULL;
    nextNode->left->parent = nextNode;
    ReadSExp( nextNode->left, "" );
    return;
  } // end else if ...


// end 基本節點 ===================================================

} // ReadSExp()

void PrintSExp( PL_Tree *root, int spaceNum, bool ifQuote ) {

  if ( root == NULL ) return;

  if ( root->type != S_EXP && root->type != LS_EXP ) {
    if ( root->isAtomString.type == LEFT_PAREN ) {
      cout << "( ";
      spaceNum++;
    } // end if ...
    else if ( root->isAtomString.type == QUOTE ) {
      cout << "( quote\n";
      spaceNum++;
      for ( int i = 0 ; i < spaceNum * 2 ; i++ ) {
        cout << " ";
      } // end for ...
    } // end else if ...
    else {
      if ( root->isAtomString.type == SYMBOL && !ifQuote ) {
        cout << "#<procedure " << root->isAtomString.changed << ">";
      } // end if
      else {
        if ( root->isAtomString.tokenLine == -4 ) {

          cout << root->isAtomString.replaced;
        } // end if ...
        else {
          // cout << root->isAtomString.original << endl;
          cout << root->isAtomString.changed;
        } // end else ...
      } // end else ...
    } // end else if ...

    if ( root->left != NULL ) PrintSExp( root->left, spaceNum, ifQuote );

    if ( root->right != NULL ) {
      if ( root->right->type != S_EXP && root->right->type != LS_EXP ) {
        if ( root->right->isAtomString.type != NIL ) {
          cout << endl;
          for ( int i = 0 ; i < spaceNum * 2 ; i++ ) {
            cout << " ";
          } // end for ...

          cout << ".\n";
          for ( int i = 0 ; i < spaceNum * 2 ; i++ ) {
            cout << " ";
          } // end for ...
        } // end if ...
      } // end if ...

      if ( root->right->isAtomString.type != NIL ) PrintSExp( root->right, spaceNum, ifQuote );
    } // end if ...


    if ( root->type == LEFT_PAREN || root->type == QUOTE ) {
      cout << endl;
      for ( int i = 0 ; i < ( spaceNum - 1 ) * 2 ; i++ ) {
        cout << " ";
      } // end for ...

      cout << ")";
    } // end if ...

    return;
  } // end if ...
  else {
    cout << "\n";
    for ( int i = 0 ; i < spaceNum * 2 ; i++ ) {
      cout << " ";
    } // end for ...

    if ( root->left != NULL ) PrintSExp( root->left, spaceNum, ifQuote );
    if ( root->right != NULL ) {
      if ( root->right->type != S_EXP && root->right->type != LS_EXP ) {
        if ( root->right->isAtomString.type != NIL ) {
          cout << endl;
          for ( int i = 0 ; i < spaceNum * 2 ; i++ ) {
            cout << " ";
          } // end for ...

          cout << ".\n";
          for ( int i = 0 ; i < spaceNum * 2 ; i++ ) {
            cout << " ";
          } // end for ...
        } // end if ...
      } // end if ...

      if ( root->right->isAtomString.type != NIL ) PrintSExp( root->right, spaceNum, ifQuote );
    } // end if ...

    return;
  } // end else ...
} // PrintSExp()

bool IsBoundOrInternalFunction( string symbol, int &tempLocal, string &internalFunction,
                                int level, bool &ifQuote ) {

  // ifQuote = false;

  for ( int i = 0 ; i < uInternalFunction.size() ; i++ ) {
    if ( symbol == uInternalFunction[i] ) {
      tempLocal = -1;
      internalFunction = uInternalFunction[i];
      return true;
    } // end if ...
  } // end for ...

  for ( int i = 0 ; i < uDefined_List.size() ; i++ ) {
    if ( symbol == uDefined_List[i].original_symbol ) {
      tempLocal = i;
      if ( uDefined_List[i].type == 1 ) {
        if ( uDefined_List[i].translate_PL_Tree->right == NULL &&
             uDefined_List[i].translate_PL_Tree->left == NULL ) {
          IsBoundOrInternalFunction( uDefined_List[i].translate_PL_Tree->isAtomString.changed,
                                     tempLocal, internalFunction, level, ifQuote );
        } // end if ...
        else {

          try {
            PL_Tree *tempPLT = Eval( uDefined_List[i].translate_PL_Tree, level, ifQuote );
            if ( tempPLT != NULL && tempPLT->isAtomString.changed == "lambda" ) {
              IsBoundOrInternalFunction( tempPLT->isAtomString.changed,
                                         tempLocal, internalFunction, level, ifQuote );

            } // end if ...
          } // end try ...
          catch ( ERROR_EVAL error_eval ) {

          } // end catch ...
        } // end else ...
      } // end if ...

      return true;
    } // end if ...
  } // end for ...

  return false;
} // IsBoundOrInternalFunction()

void FunctionLocal( string functionName, int &tempLocal ) {

  for ( int i = 0; i < uDefined_Function_List.size() ; i++ ) {
    if ( functionName == uDefined_Function_List[i].functionName ) {
      tempLocal = i;

      return;
    } // end if ...
  } // end for ...

  return;
} // FunctionLocal()

void BoundLocal( string symbol, int &tempLocal ) {

  for ( int i = 0 ; i < uDefined_List.size() ; i++ ) {
    if ( symbol == uDefined_List[i].original_symbol ) {
      tempLocal = i;

      return;
    } // end if ...
  } // end for ...

  return;
} // BoundLocal()

int HowManyArgument( PL_Tree *theFirstArgumentNode ) {

  if ( theFirstArgumentNode->right != NULL ) {
    return 1 + HowManyArgument( theFirstArgumentNode->right );
  } // end if ...
  else {
    if ( theFirstArgumentNode->isAtomString.type == NIL ) return 0;
    else return 1;
  } // end else ...

} // HowManyArgument()

bool CheckTheNumOfFunctionCorrect( string functionName, int num ) {

  if ( functionName == "quote" || functionName == "'" || functionName == "car" || functionName == "cdr" ||
       functionName == "atom?" || functionName == "pair?" || functionName == "list?" ||
       functionName == "null?" || functionName == "integer?" || functionName == "real?" ||
       functionName == "number?" || functionName == "string?" || functionName == "boolean?" ||
       functionName == "symbol?" || functionName == "not" ) {
    if ( num == 1 ) return true;
  } // end if ...
  else if ( functionName == "cons" || functionName == "eqv?" || functionName == "equal?" ) {
    if ( num == 2 ) return true;
  } // end else if ...
  else if ( functionName == "list" ) {
    if ( num >= 0 ) return true;
  } // end else if ...
  else if ( functionName == "+" || functionName == "-" || functionName == "*" ||
            functionName == "/" || functionName == "and" || functionName == "or" || functionName == ">" ||
            functionName == ">=" || functionName == "<" || functionName == "<=" || functionName == "=" ||
            functionName == "string-append" || functionName == "string>?" || functionName == "string<?" ||
            functionName == "string=?" || functionName == "let" || functionName == "define" ) {
    if ( num >= 2 ) return true;
  } // end else if ...
  else if ( functionName == "begin" || functionName == "cond" ) {
    if ( num >= 1 ) return true;
  } // end else if ...
  else if ( functionName == "if" ) {
    if ( num == 2 || num == 3 ) return true;
  } // end else if ...
  else if ( functionName == "clean-environment" || functionName == "exit" ) {
    if ( num == 0 ) return true;
  } // end else if ...
  else if ( functionName == "lambda" ) {
    if ( !uLambdaUse && num >= 2 ) return true;
    else if ( uLambdaUse &&
              num == uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentNum )
      return true;
  } // end else if ...
  else {
    for ( int i = 0; i < uDefined_Function_List.size() ; i++ ) {
      if ( functionName == uDefined_Function_List[i].functionName &&
           num == uDefined_Function_List[i].numOfArgument ) {
        return true;
      } // end if ...
    } // end for ...
  } // end else ...

  return false;
} // CheckTheNumOfFunctionCorrect()

PL_Tree *ReturnASpecificTree( int type ) {

  PL_Tree *tempRoot = new PL_Tree;
  tempRoot->parent = NULL;
  tempRoot->left = NULL;
  tempRoot->right = NULL;
  tempRoot->type = ATOM;

  if ( type == NIL ) {
    tempRoot->isAtomString.changed = "nil";
    tempRoot->isAtomString.type = NIL;
  } // end if ...
  else if ( type == T ) {
    tempRoot->isAtomString.changed = "#t";
    tempRoot->isAtomString.type = T;
  } // end else if ...
  else if ( type == INT ) {
    tempRoot->isAtomString.type = INT;
  } // end else if ...
  else if ( type == FLOAT ) {
    tempRoot->isAtomString.type = FLOAT;
  } // end else if ...
  else if ( type == STRING ) {
    tempRoot->isAtomString.type = STRING;
  } // end else if ...
  else if ( type == SYMBOL ) {
    tempRoot->type = SYMBOL;
    tempRoot->isAtomString.type = SYMBOL;
  } // end else if ...

  if ( uDebug2 ) cout << tempRoot->isAtomString.type << endl;
  return tempRoot;
} // ReturnASpecificTree()

bool IsList( PL_Tree *root ) {

  PL_Tree *temp = root;

  if ( temp->type != LEFT_PAREN ) return false;
  else {
    while ( temp->right != NULL ) temp = temp->right;

    if ( temp->type != LEFT_PAREN && temp->type != S_EXP && temp->type != LS_EXP &&
         temp->isAtomString.type != NIL ) return false;
  } // else ...

  return true;
} // IsList()

PL_Tree *CopyTree( PL_Tree *originalPL_Tree ) {

  if ( originalPL_Tree == NULL ) return NULL;

  int tempI;
  Token_Type tempTT;
  PL_Tree *copyPL_Tree = new PL_Tree;

  tempI = originalPL_Tree->type;
  copyPL_Tree->type = tempI;

  tempTT.original = originalPL_Tree->isAtomString.original;
  tempTT.changed = originalPL_Tree->isAtomString.changed;
  tempTT.type = originalPL_Tree->isAtomString.type;
  copyPL_Tree->isAtomString = tempTT;

  copyPL_Tree->parent = NULL;

  copyPL_Tree->left = CopyTree( originalPL_Tree->left );

  copyPL_Tree->right = CopyTree( originalPL_Tree->right );

  return copyPL_Tree;
} // CopyTree()

bool IsEqv( PL_Tree *one, PL_Tree *two ) {

  if ( one == two ) return true;

  if ( one->type != ATOM || two->type != ATOM ) return false;
  else {
    if ( one->isAtomString.type == STRING || two->isAtomString.type == STRING ) return false;
    else if ( one->isAtomString.type != two->isAtomString.type ) return false;
    else {
      if ( one->isAtomString.changed != two->isAtomString.changed ) return false;
    } // end else ...
  } // end else ...

  return true;
} // IsEqv()

void IsEqualContent( PL_Tree *one, PL_Tree *two ) {

  if ( one == NULL && two == NULL ) return;

  if ( one != NULL && two != NULL ) {
    // if ( one->type != two->type ) throw false;

    // if ( one->type == ATOM ) {
    if ( one->isAtomString.changed != two->isAtomString.changed ) {
      throw false;
    } // end if ...
    // } // end if ...

    IsEqualContent( one->left, two->left );

    if ( one->right == NULL && two->right != NULL ) {
      if ( two->right->isAtomString.changed == "nil" ) return;
    } // end if ...
    else if ( one->right != NULL && two->right == NULL ) {
      if ( one->right->isAtomString.changed == "nil" ) return;
    } // end else if ...

    IsEqualContent( one->right, two->right );

  } // end if ...
  else {
    /*
    cout << "33\n";
    if ( one == NULL ) cout << "1 NULL\n";
    else cout << "1 " << one->isAtomString.changed << endl;
    if ( two == NULL ) cout << "2 NULL\n";
    else cout << "2 " << two->isAtomString.changed << endl;
    */
    throw false;
  } // end else ...

  return;

} // IsEqualContent()

bool IsEqual( PL_Tree *one, PL_Tree *two ) {

  try {

    IsEqualContent( one, two );
  }
  catch ( bool YN ) {

    return false;
  } // end catch ...

  return true;
} // IsEqual()

void CleanEnvironment() {

  uDefined_List.clear();
  uDefined_Function_List.clear();
  /*
  Defined_Symbol tempDS;
  tempDS.original_symbol = "else";
  tempDS.type = SYMBOL;
  tempDS.translate_PL_Tree = ReturnASpecificTree( T );
  uDefined_List.push_back( tempDS );
  */

} // CleanEnvironment()

bool ReplaceExistedSymbol( string existedSymbol, PL_Tree *existedTree, PL_Tree *node ) {

  if ( node == NULL ) return false;

  if ( node->isAtomString.type == SYMBOL && node->isAtomString.changed == existedSymbol ) {
    if ( uDebug2 ) cout << "L1033 : replace\n";
    return true;
  } // end if ...

  if ( ReplaceExistedSymbol( existedSymbol, existedTree, node->left ) )
    node->left = existedTree;
  if ( ReplaceExistedSymbol( existedSymbol, existedTree, node->right ) )
    node->right = existedTree;

  return false;

} // ReplaceExistedSymbol()

bool ReplaceArgumentInFunction( vector<PL_Tree *> argumentList, int functionLocal, PL_Tree *node,
                                int &argumentLocal ) {

  if ( node == NULL ) return false;

  if ( node->left != NULL && node->left->isAtomString.changed == "lambda" ) return false;

  if ( node->isAtomString.type == SYMBOL ) {
    for ( int i = 1; i < uDefined_Function_List[functionLocal].functionArgument.size() ; i++ ) {
      if ( node->isAtomString.changed == uDefined_Function_List[functionLocal].functionArgument[i] ) {
        argumentLocal = i;
        return true;
      } // end if ...
    } // end for ...

    return false;
  } // end if ...

  // bool isQuote = false;
  // PL_Tree * temp;
  if ( ReplaceArgumentInFunction( argumentList, functionLocal, node->left, argumentLocal ) ) {
    /*
    temp = Eval( argumentList[argumentLocal], 2, isQuote );
    if ( isQuote ) node->left = argumentList[argumentLocal];
    else node->left = temp;
    */
    node->left = CopyTree( argumentList[argumentLocal] );
    node->left->isAtomString.tokenLine = -4;
    node->left->isAtomString.replaced =
      uDefined_Function_List[functionLocal].functionArgument[argumentLocal];
  } // end if ...

  // isQuote = false;
  if ( ReplaceArgumentInFunction( argumentList, functionLocal, node->right, argumentLocal ) ) {
    /*
    temp = Eval( argumentList[argumentLocal], 2, isQuote );
    if ( isQuote ) node->right = argumentList[argumentLocal];
    else node->right = temp;
    */
    node->right = CopyTree( argumentList[argumentLocal] );
    node->right->isAtomString.tokenLine = -4;
    node->right->isAtomString.replaced =
      uDefined_Function_List[functionLocal].functionArgument[argumentLocal];
  } // end if ...

  return false;
} // ReplaceArgumentInFunction()

bool ReplaceArgumentInLambda( vector<PL_Tree *> argumentList, PL_Tree *node, int &argumentLocal ) {

  if ( node == NULL ) return false;
  if ( node->left != NULL && node->left->isAtomString.changed == "lambda" ) return false;
  if ( node->left != NULL && node->left->isAtomString.changed == "let" ) return false;

  if ( node->isAtomString.type == SYMBOL ) {
    for ( int i = 0; i < uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentNum ;
          i++ ) {
      if ( node->isAtomString.changed ==
           uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentName[i] ) {
        argumentLocal = i + 1;
        return true;
      } // end if ...
    } // end for ...

    return false;
  } // end if ...

  if ( ReplaceArgumentInLambda( argumentList, node->left, argumentLocal ) ) {
    node->left = CopyTree( argumentList[argumentLocal] );

    node->left->isAtomString.tokenLine = -4;
    node->left->isAtomString.replaced =
      uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentName[argumentLocal - 1];
  } // end if ...

  if ( ReplaceArgumentInLambda( argumentList, node->right, argumentLocal ) ) {
    node->right = CopyTree( argumentList[argumentLocal] );

    node->right->isAtomString.tokenLine = -4;
    node->right->isAtomString.replaced =
      uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentName[argumentLocal - 1];
  } // end if ...

  return false;
} // ReplaceArgumentInLambda()

bool RepairLetFunction( vector<Defined_Symbol> argumentList, PL_Tree *node, int &argumentLocal, 
                        int letNum ) {

  if ( node == NULL ) return false;
  
  if ( node->isAtomString.tokenLine == -4 ) {
    
    for ( int i = 0; i < letNum ; i++ ) {
      if ( node->isAtomString.replaced == argumentList[i].original_symbol ) {
        if ( uDebug4 ) cout << "error1254\n";
        argumentLocal = i;
        return true;
      } // end if ...
    } // end for ...
  } // end if ...

  if ( RepairLetFunction( argumentList, node->left, argumentLocal, letNum ) ) {
    node->left = CopyTree( argumentList[argumentLocal].translate_PL_Tree );
    node->left->isAtomString.tokenLine = -4;
    node->left->isAtomString.replaced = argumentList[argumentLocal].original_symbol;
  } // end if ...

  if ( RepairLetFunction( argumentList, node->right, argumentLocal, letNum ) ) {
    node->right = CopyTree( argumentList[argumentLocal].translate_PL_Tree );
    node->right->isAtomString.tokenLine = -4;
    node->right->isAtomString.replaced = argumentList[argumentLocal].original_symbol;
  } // end if ...

  return false;
} // RepairLetFunction()

int CheckDefineFirstArgument( PL_Tree *root, vector<string> &functionArgument ) {

  PL_Tree *temp = root;
  bool ifQuote = false;

  int tempLocal = -2;
  string internalFunction;

  if ( temp->isAtomString.type == SYMBOL ) return 1;
  else if ( temp->isAtomString.type != LEFT_PAREN ) {
    if ( uDebug3 ) cout << "CheckDefineFirstArgument 1\n";
    return -1;
  } // end else if ...
  else {
    do {

      if ( temp->left->isAtomString.type != SYMBOL ) {
        if ( uDebug3 ) cout << "CheckDefineFirstArgument 2\n";
        return -1;
      } // end if ...
      else {
        /*
        IsBoundOrInternalFunction( temp->left->isAtomString.changed, tempLocal, internalFunction,
                                   0, ifQuote );
        if ( tempLocal == -1 ) {
          if ( uDebug3 ) cout << "CheckDefineFirstArgument 3\n";
          return -1;
        } // end if ...
        else {
          functionArgument.push_back( temp->left->isAtomString.changed );
        } // end else ...
        */
        functionArgument.push_back( temp->left->isAtomString.changed );
      } // end else ...

      temp = temp->right;
    } while ( temp != NULL && temp->isAtomString.type != NIL );

  } // end else ...

  return functionArgument.size();

} // CheckDefineFirstArgument()

void CheckLetDefineArgument( PL_Tree *root, int &let_Defined_Num,
                             vector<Defined_Symbol> &let_Defined_List ) {

  PL_Tree *temp = root;
  int level = 2;
  bool ifQuote = false;
  Defined_Symbol tempDS;

  int argumentNum = HowManyArgument( temp );

  if ( argumentNum != 2 ) {
    if ( uDebug3 ) cout << "ERROR_LET_FORMAT 1\n";
    throw 4;
  } // end if ...

  if ( temp->isAtomString.type == LEFT_PAREN ) {
    if ( temp->left->isAtomString.tokenLine == -4 ) {
      string tempS = temp->left->isAtomString.replaced;
      temp->left = ReturnASpecificTree( SYMBOL );
      temp->left->isAtomString.changed = tempS;
    } // end if ...

    if ( temp->left->isAtomString.type != SYMBOL ) {
      if ( uDebug3 ) cout << "ERROR_LET_FORMAT 2\n";
      throw 4;
    } // end if ...

    tempDS.original_symbol = temp->left->isAtomString.changed;
    if ( uDebug3 ) cout << tempDS.original_symbol << endl;
    tempDS.type = 1;
    temp = temp->right;
    // Eval( temp->left, level, ifQuote );
    // tempDS.translate_PL_Tree = Eval( temp->left, level, ifQuote );
    tempDS.translate_PL_Tree = Eval( temp->left, level, ifQuote );
    if ( tempDS.translate_PL_Tree->isAtomString.changed == "(" )
      tempDS.translate_PL_Tree = ToBeQuote( tempDS.translate_PL_Tree );
    if ( tempDS.translate_PL_Tree->isAtomString.changed == "lambda" )
      tempDS.translate_PL_Tree = temp->left;
  } // end if ...
  else {
    if ( uDebug3 ) cout << "ERROR_LET_FORMAT 3\n";
    throw 4;
  } // end else ...

  int tempLocal = -1;
  for ( int i = 0; i < let_Defined_Num ; i++ ) {
    if ( tempDS.original_symbol == let_Defined_List[i].original_symbol ) tempLocal = i;
  } // end for ...

  if ( tempLocal > -1 ) {
    let_Defined_List.erase( let_Defined_List.begin() + tempLocal );
    let_Defined_Num--;
  } // end if ...

  let_Defined_List.insert( let_Defined_List.begin(), tempDS );
  let_Defined_Num++;

} // CheckLetDefineArgument()

void InitializeReplace( PL_Tree *root ) {

  if ( root == NULL ) return;

  root->isAtomString.tokenLine = -1;
  root->isAtomString.replaced = "";

  InitializeReplace( root->left );
  InitializeReplace( root->right );

} // InitializeReplace()

PL_Tree *Eval( PL_Tree *originalRoot, int level, bool &ifQuote ) {

  int tempLocal = -2;
  string internalFunction = "";
  int argumentNum = 0;
  bool useDefinedFunction = false;


  PL_Tree *otherArgument = NULL;
  vector<PL_Tree *> otherArgumentList;
  otherArgumentList.clear();

  if ( originalRoot->isAtomString.type >= INT && originalRoot->isAtomString.type <= T ) {
    if ( uDebug2 ) cout << "return a ATOM " << originalRoot->isAtomString.changed << endl;
    return originalRoot;
  } // end if ...
  else if ( originalRoot->isAtomString.type == SYMBOL ) {

    if ( !IsBoundOrInternalFunction( originalRoot->isAtomString.changed, tempLocal, internalFunction,
                                     level, ifQuote ) ) {
      uErrorToken = originalRoot->isAtomString;
      throw ERROR_UNBOUND_SYMBOY;
    } // end if ...
    else {
      if ( tempLocal == -1 ) {
        otherArgument = ReturnASpecificTree( SYMBOL );
        otherArgument->isAtomString.changed = internalFunction;
        return otherArgument;
      } // end if ...
      else {
        if ( uDefined_List[tempLocal].type == 1 ) {
          try {
            return Eval( uDefined_List[tempLocal].translate_PL_Tree, level, ifQuote );
          } // end try ...
          catch ( ERROR_EVAL error_eval ) {
            return uDefined_List[tempLocal].translate_PL_Tree;
          } // end catch ...
        } // end if ...
        else if ( uDefined_List[tempLocal].type == 2 ) {
          otherArgument = ReturnASpecificTree( SYMBOL );
          otherArgument->isAtomString.changed = uDefined_List[tempLocal].original_symbol;
          return otherArgument;
        } // end else if ...
      } // end else ...
    } // end else ...
  } // end else if ...
  else if ( originalRoot->type == QUOTE ) {
    argumentNum = HowManyArgument( originalRoot ) - 1;
    if ( argumentNum == 0 ) {
      otherArgument = originalRoot;
      ifQuote = true;
      return otherArgument->left;
    } // end if ...
    else {
      uErrorToken.changed = internalFunction;
      throw ERROR_INCORRECT_NUMBER_OF_ARGUMENTS;
    } // end else ...
  } // end else if ...
  else {
    if ( originalRoot->type == LEFT_PAREN ) {
      if ( !IsList( originalRoot ) ) {
        uErrorTree = originalRoot;
        throw ERROR_NON_LIST;
      } // end if ...
      else if ( originalRoot->left->type == ATOM ) {
        uErrorToken = originalRoot->left->isAtomString;
        throw ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION;
      } // end else if ...
      else if ( originalRoot->left->isAtomString.type == SYMBOL ) {
        if ( IsBoundOrInternalFunction( originalRoot->left->isAtomString.changed, tempLocal,
                                        internalFunction, level, ifQuote ) ) {
          if ( tempLocal == -1 ) {
            if ( level > 1 ) {
              if ( internalFunction == "clean-environment" )
                throw ERROR_LEVEL_OF_CLEAN_ENVIRONMENT;
              else if ( internalFunction == "define" )
                throw ERROR_LEVEL_OF_DEFINE;
              else if ( internalFunction == "exit" )
                throw ERROR_LEVEL_OF_EXIT;
            } // end if ...

            if ( internalFunction == "define" || internalFunction == "set!" ||
                 internalFunction == "let" || internalFunction == "cond" ||
                 internalFunction == "lambda" ) {
              if ( internalFunction == "define" ) {
                // ================================ define ================================
                argumentNum = HowManyArgument( originalRoot ) - 1;
                if ( !CheckTheNumOfFunctionCorrect( internalFunction, argumentNum ) ) {
                  uErrorTree = originalRoot;
                  if ( uDebug3 ) cout << "ERROR_DEFINE_FORMAT 1\n";
                  if ( uDebug3 ) cout << argumentNum << endl;
                  throw ERROR_DEFINE_FORMAT;
                } // end if ...

                otherArgument = originalRoot->right;
                int tempSwitch;
                vector<string> functionArgument;
                functionArgument.clear();
                tempSwitch = CheckDefineFirstArgument( otherArgument->left, functionArgument );

                if ( tempSwitch == -1 ) {
                  uErrorTree = originalRoot;
                  if ( uDebug3 ) cout << "ERROR_DEFINE_FORMAT 2\n";
                  throw ERROR_DEFINE_FORMAT;
                } // end if ...
                else if ( tempSwitch == 1 ) {

                  if ( argumentNum > 2 ) {
                    uErrorTree = originalRoot;
                    if ( uDebug3 ) cout << "ERROR_DEFINE_FORMAT 5\n";
                    if ( uDebug3 ) cout << argumentNum << endl;
                    throw ERROR_DEFINE_FORMAT;
                  } // end if ...

                  tempLocal = -2;
                  if ( IsBoundOrInternalFunction( otherArgument->left->isAtomString.changed, tempLocal,
                                                  internalFunction, level, ifQuote ) ) {
                    if ( tempLocal == -1 ) {
                      BoundLocal( otherArgument->left->isAtomString.changed, tempLocal );
                      if ( tempLocal == -1 ) {
                        uErrorTree = originalRoot;
                        if ( uDebug3 ) cout << "ERROR_DEFINE_FORMAT 3\n";
                        throw ERROR_DEFINE_FORMAT;
                      } // end if ...
                    } // end if ...
                  } // end if ...

                  Defined_Symbol tempDD;
                  tempDD.original_symbol = otherArgument->left->isAtomString.changed;
                  tempDD.type = 1;
                  otherArgument = otherArgument->right;
                  // Eval( otherArgument->left, 2, ifQuote );
                  if ( tempLocal >= 0 ) {

                    ReplaceExistedSymbol( tempDD.original_symbol, uDefined_List[tempLocal].translate_PL_Tree,
                                          otherArgument->left );
                    tempDD.translate_PL_Tree = Eval( otherArgument->left, 2, ifQuote );

                    if ( tempDD.translate_PL_Tree->isAtomString.changed == "lambda" || ifQuote )
                      tempDD.translate_PL_Tree = otherArgument->left;
                    BoundLocal( tempDD.original_symbol, tempLocal );
                    uDefined_List.erase( uDefined_List.begin() + tempLocal );
                  } // end if ...
                  else {
                    tempDD.translate_PL_Tree = Eval( otherArgument->left, 2, ifQuote );
                    if ( tempDD.translate_PL_Tree->isAtomString.changed == "lambda" || ifQuote )
                      tempDD.translate_PL_Tree = otherArgument->left;
                  } // end else ...

                  uDefined_List.push_back( tempDD );

                  cout << tempDD.original_symbol << " defined";

                } // end else if ...
                else {
                  otherArgument = otherArgument->right;
                  Defined_Function tempDF;
                  tempDF.functionName = functionArgument[0];
                  tempDF.functionArgument = functionArgument;
                  tempDF.numOfArgument = functionArgument.size() - 1;
                  do {
                    tempDF.translate_PL_Tree_List.push_back( otherArgument->left );
                    otherArgument = otherArgument->right;
                  } while ( otherArgument != NULL );

                  tempLocal = -2;
                  if ( IsBoundOrInternalFunction( tempDF.functionName, tempLocal,
                                                  internalFunction, level, ifQuote ) ) {
                    if ( tempLocal == -1 ) {
                      BoundLocal( tempDF.functionName, tempLocal );
                      if ( tempLocal == -1 ) {
                        uErrorTree = originalRoot;
                        if ( uDebug3 ) cout << "ERROR_DEFINE_FORMAT 4\n";
                        throw ERROR_DEFINE_FORMAT;
                      } // end if ...
                    } // end if ...
                  } // end if ...

                  if ( tempLocal >= 0 ) {
                    uDefined_List.erase( uDefined_List.begin() + tempLocal );
                    Defined_Symbol tempDS;
                    tempDS.original_symbol = tempDF.functionName;
                    tempDS.type = 2;
                    tempDS.translate_PL_Tree = ReturnASpecificTree( SYMBOL );
                    tempDS.translate_PL_Tree->isAtomString.changed = tempDF.functionName;
                    uDefined_List.push_back( tempDS );

                    tempLocal = -1;
                    for ( int i = 0; i < uDefined_Function_List.size() ; i++ ) {
                      if ( tempDF.functionName == uDefined_Function_List[i].functionName )
                        tempLocal = i;
                    } // end for ...

                    if ( tempLocal == -1 )
                      uDefined_Function_List.push_back( tempDF );
                    else {
                      uDefined_Function_List.erase( uDefined_Function_List.begin() + tempLocal );
                      uDefined_Function_List.push_back( tempDF );
                    } // end else ...
                  } // end if ...
                  else {
                    Defined_Symbol tempDS;
                    tempDS.original_symbol = tempDF.functionName;
                    tempDS.type = 2;
                    tempDS.translate_PL_Tree = ReturnASpecificTree( SYMBOL );
                    tempDS.translate_PL_Tree->isAtomString.changed = tempDF.functionName;
                    uDefined_List.push_back( tempDS );
                    uDefined_Function_List.push_back( tempDF );
                  } // end else ...

                  cout << tempDF.functionName << " defined";
                } // end else

                return NULL;
              } // end if ...
              // ================================ define ================================
              else if ( internalFunction == "cond" ) {
                argumentNum = HowManyArgument( originalRoot ) - 1;
                if ( CheckTheNumOfFunctionCorrect( internalFunction, argumentNum ) ) {
                  otherArgument = originalRoot->right;

                  while ( otherArgument != NULL ) {
                    if ( HowManyArgument( otherArgument->left ) < 2 ) {
                      uErrorTree = originalRoot;
                      throw ERROR_COND_FORMAT;
                    } // end if ...

                    otherArgument = otherArgument->right;
                  } // end while ...

                  otherArgument = originalRoot->right;
                  PL_Tree *tempPLT;
                  while ( otherArgument != NULL ) {
                    tempPLT = otherArgument;
                    if ( HowManyArgument( tempPLT->left ) >= 2 ) {
                      tempPLT = tempPLT->left;

                      if ( otherArgument->right == NULL ) {
                        if ( tempPLT->left->isAtomString.changed == "else" ) {
                          while ( tempPLT->right != NULL ) tempPLT = tempPLT->right;
                          return Eval( tempPLT->left, 2, ifQuote );
                        } // end if ...
                        else if ( Eval( tempPLT->left, 2, ifQuote )->isAtomString.type != NIL ) {
                          while ( tempPLT->right != NULL ) {
                            tempPLT = tempPLT->right;
                            Eval( tempPLT->left, 2, ifQuote );
                          } // end while ...

                          return Eval( tempPLT->left, 2, ifQuote );
                        } // end else if ...
                        else {
                          uErrorTree = originalRoot;
                          throw ERROR_NO_RETURN_VALUE;
                        } // end else ...
                      } // end if ...
                      else if ( Eval( tempPLT->left, 2, ifQuote )->isAtomString.type != NIL ) {
                        while ( tempPLT->right != NULL ) {
                          tempPLT = tempPLT->right;
                          try {
                            Eval( tempPLT->left, 2, ifQuote );
                          } // end try ...
                          catch ( ERROR_EVAL error_eval ) {
                            if ( error_eval == ERROR_NO_RETURN_VALUE ) ;
                            else throw error_eval;
                          } // end catch ...
                        } // end while ...

                        return Eval( tempPLT->left, 2, ifQuote );
                      } // end else if ...
                    } // end if ...
                    else {
                      uErrorTree = originalRoot;
                      throw ERROR_COND_FORMAT;
                    } // end else ...

                    otherArgument = otherArgument->right;
                  } // end while

                  uErrorTree = originalRoot;
                  throw ERROR_COND_FORMAT;
                } // end if ...
                else {
                  uErrorTree = originalRoot;
                  throw ERROR_COND_FORMAT;
                } // end else ...

              } // end else if ...
              else if ( internalFunction == "lambda" ) {
                if ( !uLambdaUse ) {
                  argumentNum = HowManyArgument( originalRoot ) - 1;
                  if ( argumentNum < 2 ) {
                    uErrorTree = originalRoot;
                    if ( uDebug3 ) cout << "ERROR_LAMBDA_FORMAT 1\n";
                    throw ERROR_LAMBDA_FORMAT;
                  } // end if ...

                  otherArgument = originalRoot->right;

                  if ( otherArgument->left->isAtomString.type == NIL ) {
                    Lambda_Function lF;
                    uLambdaArgumentName.clear();
                    uLambdaArgumentNum = 0;
                    lF.lambdaArgumentName.clear();
                    lF.lambdaArgumentNum = 0;
                    while ( otherArgument->right != NULL ) otherArgument = otherArgument->right;
                    uLambda_PL_Tree = otherArgument->left;
                    lF.lambda_PL_Tree = otherArgument->left;
                    uLambdaUse = true;
                    uLambda_Function_List.push_back( lF );

                    otherArgument = ReturnASpecificTree( SYMBOL );
                    otherArgument->isAtomString.changed = "lambda";
                    return otherArgument;
                  } // end else if ...
                  else if ( otherArgument->left->isAtomString.type == LEFT_PAREN ) {
                    int tempSwitch;
                    Lambda_Function lF;
                    uLambdaArgumentName.clear();
                    lF.lambdaArgumentName.clear();
                    tempSwitch = CheckDefineFirstArgument( otherArgument->left, uLambdaArgumentName );
                    if ( tempSwitch == -1 ) {
                      uErrorTree = originalRoot;
                      if ( uDebug3 ) cout << "ERROR_LAMBDA_FORMAT 1\n";
                      throw ERROR_LAMBDA_FORMAT;
                    } // end if ...

                    lF.lambdaArgumentName = uLambdaArgumentName;
                    uLambdaArgumentNum = tempSwitch;
                    lF.lambdaArgumentNum = tempSwitch;
                    while ( otherArgument->right != NULL ) otherArgument = otherArgument->right;
                    uLambda_PL_Tree = otherArgument->left;
                    lF.lambda_PL_Tree = otherArgument->left;
                    uLambdaUse = true;
                    uLambda_Function_List.push_back( lF );

                    otherArgument = ReturnASpecificTree( SYMBOL );
                    otherArgument->isAtomString.changed = "lambda";
                    return otherArgument;
                  } // end if ...
                  else {
                    uErrorTree = originalRoot;
                    if ( uDebug3 ) cout << "ERROR_LAMBDA_FORMAT 2\n";
                    throw ERROR_LAMBDA_FORMAT;
                  } // end else ...

                  return NULL;
                } // end if ...
                else {
                  argumentNum = HowManyArgument( originalRoot ) - 1;
                  if ( argumentNum != uLambda_Function_List[uLambda_Function_List.size() - 1].
                       lambdaArgumentNum ) {
                    if ( uDebug3 ) cout << argumentNum << "  " <<
                      uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentNum <<
                      endl;
                    uErrorToken.changed = "lambda";
                    throw ERROR_INCORRECT_NUMBER_OF_ARGUMENTS;
                  } // end if ...

                  otherArgument = ReturnASpecificTree( SYMBOL );
                  otherArgument->isAtomString.changed = "lambda";
                  otherArgumentList.push_back( otherArgument );
                  uLambdaUse = false;
                } // end else
              } // end else if
              else if ( internalFunction == "let" ) {
                argumentNum = HowManyArgument( originalRoot ) - 1;
                if ( !CheckTheNumOfFunctionCorrect( internalFunction, argumentNum ) ) {
                  uErrorTree = originalRoot;
                  if ( uDebug3 ) cout << "ERROR_LET_FORMAT 4\n";
                  throw ERROR_LET_FORMAT;
                } // end if ...

                otherArgument = originalRoot->right;
                int let_Defined_Num = 0;
                vector<Defined_Symbol> let_Defined_List;
                let_Defined_List.clear();

                if ( otherArgument->left->isAtomString.type == NIL ) {
                  // nothing ...
                } // end if ...
                else if ( otherArgument->left->isAtomString.type == LEFT_PAREN ) {
                  otherArgument = otherArgument->left;
                  PL_Tree *tempOA = otherArgument;
                  while ( tempOA != NULL ) {
                    if ( HowManyArgument( tempOA->left ) != 2 ) {
                      uErrorTree = originalRoot;
                      if ( uDebug3 ) cout << "ERROR_LET_FORMAT 6\n";
                      throw ERROR_LET_FORMAT;
                    } // end if ...

                    tempOA = tempOA->right;
                  } // end while ...

                  delete tempOA;

                  do {
                    try {
                      CheckLetDefineArgument( otherArgument->left, let_Defined_Num, let_Defined_List );
                    } // end try ...
                    catch ( int G ) {
                      if ( G == 4 ) {
                        uErrorTree = originalRoot;
                        if ( uDebug3 ) cout << "ERROR_LET_FORMAT 1~3\n";
                        throw ERROR_LET_FORMAT;
                      } // end if ...
                    } // end catch ...
                    catch ( ERROR_EVAL errorEval ) {
                      if ( errorEval == ERROR_NO_RETURN_VALUE ) {
                        throw ERROR_NO_RETURN_VALUE_TO_MAIN;
                      } // end if ...
                      else throw errorEval;
                    } // end catch ...
                    otherArgument = otherArgument->right;
                  } while ( otherArgument != NULL );

                  Defined_Symbol tempDS;
                  while ( !let_Defined_List.empty() ) {
                    tempDS.original_symbol = let_Defined_List.back().original_symbol;
                    tempDS.type = let_Defined_List.back().type;
                    tempDS.translate_PL_Tree = let_Defined_List.back().translate_PL_Tree;
                    uDefined_List.insert( uDefined_List.begin(), tempDS );
                    let_Defined_List.pop_back();
                  } // end while ...

                } // end else if ...
                else {
                  uErrorTree = originalRoot;
                  if ( uDebug3 ) cout << "ERROR_LET_FORMAT 5\n";
                  throw ERROR_LET_FORMAT;
                } // end else ...

                int tempUse = -1;
                otherArgument = originalRoot->right->right;
                while ( otherArgument->right != NULL ) {
                  try {
                    RepairLetFunction( uDefined_List, otherArgument->left, tempUse, let_Defined_Num );
                    Eval( otherArgument->left, 2, ifQuote );
                  } // end try ...
                  catch ( ERROR_EVAL error_eval ) {
                    if ( error_eval == ERROR_NO_RETURN_VALUE ) ;
                    else throw error_eval;
                  } // end catch ...
                  otherArgument = otherArgument->right;
                } // end while ...

                try {
                  RepairLetFunction( uDefined_List, otherArgument->left, tempUse, let_Defined_Num );
                  otherArgument = Eval( otherArgument->left, 2, ifQuote );
                } // end try ...
                catch ( ERROR_EVAL error_eval ) {
                  if ( let_Defined_Num > 0 ) {
                    uDefined_List.erase( uDefined_List.begin(), uDefined_List.begin() + let_Defined_Num );
                  } // end if ...

                  throw error_eval;
                } // end catch ...

                if ( uDebug3 ) cout << "let_Defined_Num : " << let_Defined_Num << endl;
                if ( let_Defined_Num > 0 ) {
                  uDefined_List.erase( uDefined_List.begin(), uDefined_List.begin() + let_Defined_Num );
                } // end if ...

                let_Defined_Num = 0;
                if ( uDebug3 ) cout << otherArgument->isAtomString.changed << endl;
                return otherArgument;

                return NULL;
              } // end else if ...
            } // end if ...
            else if ( internalFunction == "if" || internalFunction == "and" || internalFunction == "or" ) {

              argumentNum = HowManyArgument( originalRoot ) - 1;

              if ( CheckTheNumOfFunctionCorrect( internalFunction, argumentNum ) ) {
                if ( internalFunction == "if" ) {
                  otherArgument = originalRoot->right;
                  PL_Tree *tempTT = NULL;

                  tempTT = Eval( otherArgument->left, level + 1, ifQuote );

                  if ( tempTT->isAtomString.type != NIL ) {
                    otherArgument = otherArgument->right;
                    return Eval( otherArgument->left, level + 2, ifQuote );
                  } // end if ...
                  else {
                    if ( argumentNum == 3 ) {
                      otherArgument = otherArgument->right->right;
                      return Eval( otherArgument->left, level + 3, ifQuote );
                    } // end if ...
                    else {
                      uErrorTree = originalRoot;
                      throw ERROR_NO_RETURN_VALUE;
                    } // end else ...
                  } // end else ...
                } // end if ...
                else if ( internalFunction == "and" ) {
                  otherArgument = originalRoot->right;
                  PL_Tree *tempTT = NULL;

                  bool continuteOtherArgumentList = true;
                  do {
                    level = level + 1;
                    try {
                      tempTT = Eval( otherArgument->left, level, ifQuote );
                    } // end try ...
                    catch ( ERROR_EVAL error_eval ) {
                      if ( error_eval == ERROR_NO_RETURN_VALUE ) {
                        uErrorTree = otherArgument->left;
                        throw ERROR_UNBOUND_CONDITION;
                      } // end if ...
                      else {
                        throw error_eval;
                      } // end else
                    } // end catch ...

                    if ( tempTT->isAtomString.type == NIL ) return ReturnASpecificTree( NIL );

                    if ( otherArgument->right != NULL ) {
                      otherArgument = otherArgument->right;
                    } // end if ...
                    else continuteOtherArgumentList = false;
                  } while ( continuteOtherArgumentList );

                  return Eval( otherArgument->left, level, ifQuote );
                } // end else if ...
                else if ( internalFunction == "or" ) {
                  otherArgument = originalRoot->right;
                  PL_Tree *tempTT = NULL;

                  bool continuteOtherArgumentList = true;
                  do {
                    level = level + 1;
                    tempTT = Eval( otherArgument->left, level, ifQuote );

                    if ( tempTT->isAtomString.type != NIL ) return tempTT;

                    if ( otherArgument->right != NULL ) {
                      otherArgument = otherArgument->right;
                    } // end if ...
                    else continuteOtherArgumentList = false;
                  } while ( continuteOtherArgumentList );

                  return Eval( otherArgument->left, level, ifQuote );
                } // end else if ...
              } // end if ...
              else {
                uErrorToken.changed = internalFunction;
                throw ERROR_INCORRECT_NUMBER_OF_ARGUMENTS;
              } // end else ...
            } // end else if ...
            else if ( internalFunction == "quote" ) {
              argumentNum = HowManyArgument( originalRoot ) - 1;
              if ( CheckTheNumOfFunctionCorrect( internalFunction, argumentNum ) ) {
                otherArgument = originalRoot->right;
                ifQuote = true;
                return otherArgument->left;
              } // end if ...
              else {
                uErrorToken.changed = internalFunction;
                throw ERROR_INCORRECT_NUMBER_OF_ARGUMENTS;
              } // end else ...
            } // end else if ...
            else {
              // SYM is a known function name 'abc', which is neither
              // 'define' nor 'let' nor 'cond' nor 'lambda'
              argumentNum = HowManyArgument( originalRoot ) - 1;
              if ( uDebug2 ) cout << "SYM is a known function name 'abc', which is neither" << endl;

              if ( CheckTheNumOfFunctionCorrect( internalFunction, argumentNum ) ) {
                otherArgument = new PL_Tree;
                otherArgument->parent = NULL;
                otherArgument->left = NULL;
                otherArgument->right = NULL;
                otherArgument->type = SYMBOL;
                otherArgument->isAtomString.changed = internalFunction;
                otherArgumentList.push_back( otherArgument );
                if ( uDebug2 ) cout << "checkTheNumOfFunctionCorrect is over\n" << endl;
              } // end if ...
              else {
                uErrorToken.changed = internalFunction;
                throw ERROR_INCORRECT_NUMBER_OF_ARGUMENTS;
              } // end else ...
            } // end else ...
          } // end if ...
          else {
            if ( tempLocal == -1 ) {
              uErrorToken.changed = internalFunction;
              throw ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION;
            } // end if ...
            else if ( uDefined_List[tempLocal].type == 1 ) {
              uErrorToken = Eval( uDefined_List[tempLocal].translate_PL_Tree, 2,
                                  ifQuote )->isAtomString;
              throw ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION;
            } // end if ...
            // SYM is 'abc', which is not the name of a known function
            else if ( uDefined_List[tempLocal].type == 2 ) {
              FunctionLocal( uDefined_List[tempLocal].original_symbol, tempLocal );
              argumentNum = HowManyArgument( originalRoot ) - 1;

              if ( argumentNum == uDefined_Function_List[tempLocal].numOfArgument ) {
                otherArgument = ReturnASpecificTree( SYMBOL );
                otherArgument->isAtomString.changed = uDefined_Function_List[tempLocal].functionName;
                otherArgumentList.push_back( otherArgument );
                useDefinedFunction = true;
              } // end if ...
              else {
                uErrorToken.changed = uDefined_Function_List[tempLocal].functionName;
                throw ERROR_INCORRECT_NUMBER_OF_ARGUMENTS;
              } // end else ...
            } // end if ...
            else {
              uErrorToken = uDefined_List[tempLocal].translate_PL_Tree->isAtomString;
              throw ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION;
            } // end else ...
          } // end else ...
        } // end if ...
        else {
          // SYM is 'abc', which is not the name of a bound function
          uErrorToken = originalRoot->left->isAtomString;
          throw ERROR_UNBOUND_SYMBOY;
        } // end else ...
      } // end else if
      else {
        // the first argument of ( ... ) is ( 。。。 ), i.e., it is ( ( 。。。 ) ...... )
        try {
          otherArgumentList.push_back( Eval( originalRoot->left, level, ifQuote ) );
        } // end try ...
        catch ( ERROR_EVAL error_eval ) {
          if ( error_eval == ERROR_NO_RETURN_VALUE ) {
            uErrorTree = originalRoot->left;
            throw ERROR_NO_RETURN_VALUE_TO_MAIN;
          } // end if ...
          else throw error_eval;
        } // end catch ...

        if ( IsBoundOrInternalFunction( otherArgumentList[0]->isAtomString.changed, tempLocal,
                                        internalFunction, level, ifQuote ) ) {
          argumentNum = HowManyArgument( originalRoot ) - 1;
          if ( !CheckTheNumOfFunctionCorrect( otherArgumentList[0]->isAtomString.changed, argumentNum ) ) {
            uErrorToken.changed = otherArgumentList[0]->isAtomString.changed;

            if ( uDebug3 ) {
              cout << "ERROR_INCORRECT_NUMBER_OF_ARGUMENTS 1\n";
              cout << uLambdaUse << endl;
              cout << argumentNum << endl;
              cout << uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentNum <<
                endl;
            } // end if ...

            throw ERROR_INCORRECT_NUMBER_OF_ARGUMENTS;
          } // end if ...
          else {
            if ( tempLocal != -1 && uDefined_List[tempLocal].type == 2 ) {
              useDefinedFunction = true;
            } // end if ...
          } // end else ...

        } // end if ...
        else {
          if ( otherArgumentList[0]->isAtomString.type >= INT &&
               otherArgumentList[0]->isAtomString.type <= T ) {
            uErrorToken.changed = otherArgumentList[0]->isAtomString.changed;
            throw ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION;
          } // end if ...
          else {
            uErrorTree = otherArgumentList[0];
            throw ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION2;
          } // end else ...
        } // end else
      } // end else ...
    } // end if ...

    if ( originalRoot->right != NULL ) {
      otherArgument = originalRoot->right;
      PL_Tree *tempTT = NULL;

      bool continuteOtherArgumentList = true;
      do {

        uLambdaUse = false;
        try {
          if ( useDefinedFunction ) {
            tempTT = Eval( otherArgument->left, 0, ifQuote );
            if ( tempTT->isAtomString.changed == "(" )
              tempTT = ToBeQuote( tempTT );
            // tempTT = otherArgument->left;
          } // end if ...
          else tempTT = Eval( otherArgument->left, 0, ifQuote );
        } // end try ...
        catch ( ERROR_EVAL error_eval ) {

          if ( error_eval == ERROR_NO_RETURN_VALUE ) {
            /*
            if ( useDefinedFunction ) {
              uErrorTree = otherArgument->left;
              throw ERROR_UNBOUND_PARAMETER;
            } // end if ...
            else {
              if ( otherArgumentList[0]->isAtomString.changed == "and" ||
                   otherArgumentList[0]->isAtomString.changed == "or" ) {
                uErrorTree = otherArgument->left;
                throw ERROR_UNBOUND_CONDITION;
              } // end if ...
              else {
                if ( otherArgument->right != NULL ) ;
                else throw error_eval;
              } // end else ...
            } // end else ...
            */
            if ( otherArgumentList[0]->isAtomString.changed == "begin" ) {
              if ( otherArgument->right != NULL ) ;
              else throw error_eval;
            } // end if ...
            else {
              uErrorTree = otherArgument->left;
              throw ERROR_UNBOUND_PARAMETER;
            } // end else ...
          } // end if ...
          else throw error_eval;

        } // end catch

        otherArgumentList.push_back( tempTT );
        if ( otherArgument->right != NULL ) {
          otherArgument = otherArgument->right;
        } // end if ...
        else continuteOtherArgumentList = false;
      } while ( continuteOtherArgumentList );
    } // end if ...

    if ( otherArgumentList.size() > 0 ) {

      string firstArgument;
      firstArgument = otherArgumentList[0]->isAtomString.changed;

      if ( useDefinedFunction ) {
        for ( int i = 0; i < uDefined_Function_List.size() ; i++ ) {
          if ( firstArgument == uDefined_Function_List[i].functionName ) tempLocal = i;
        } // end for ()
        /*
        Defined_Symbol tempFunction_DS;
        int tempArgumentSize = uDefined_Function_List[tempLocal].functionArgument.size();
        for ( int i = 1; i < tempArgumentSize ; i++ ) {
          tempFunction_DS.original_symbol = uDefined_Function_List[tempLocal].functionArgument[i];
          tempFunction_DS.type = 1;
          tempFunction_DS.translate_PL_Tree = otherArgumentList[i];
          uDefined_List.insert( uDefined_List.begin(), tempFunction_DS );
        } // end for ...

        tempArgumentSize--;
        */
        int tempF_List = 0;

        PL_Tree *temp = NULL;

        do {

          temp = CopyTree( uDefined_Function_List[tempLocal].translate_PL_Tree_List[tempF_List] );
          ReplaceArgumentInFunction( otherArgumentList, tempLocal, temp, argumentNum );

          try {
            otherArgument = Eval( temp, level, ifQuote );
          } //
          catch ( ERROR_EVAL error_eval ) {

            if ( error_eval == ERROR_NO_RETURN_VALUE ) {
              if ( tempF_List < uDefined_Function_List[tempLocal].translate_PL_Tree_List.size() - 1 ) {
                if ( uDebug3 ) cout << "ERROR_NO_RETURN_VALUE : " << tempF_List << " - " <<
                  uDefined_Function_List[tempLocal].translate_PL_Tree_List.size() << endl;
              } // end if ...
              else {
                /*
                if ( tempArgumentSize > 0 )
                  uDefined_List.erase( uDefined_List.begin(), uDefined_List.begin() + tempArgumentSize );
                */
                uErrorToken.changed = firstArgument;
                uErrorTree = originalRoot;
                throw error_eval;
              } // end else ...
            } // end if ...
            else {
              /*
              if ( tempArgumentSize > 0 )
                uDefined_List.erase( uDefined_List.begin(), uDefined_List.begin() + tempArgumentSize );
              */
              throw error_eval;
            } // end else ...
          } // end catch ...

          tempF_List++;
        } while ( tempF_List < uDefined_Function_List[tempLocal].translate_PL_Tree_List.size() );
        /*
        if ( tempArgumentSize > 0 )
          uDefined_List.erase( uDefined_List.begin(), uDefined_List.begin() + tempArgumentSize );
        */
        return otherArgument;
      } // end if ...
      else if ( firstArgument == "lambda" ) {
        /*
        Defined_Symbol tempFunction_DS;
        int tempArgumentSize = uLambda_Function_List[uLambda_Function_List.size() - 1].lambdaArgumentNum;
        for ( int i = 0; i < tempArgumentSize ; i++ ) {
          tempFunction_DS.original_symbol = uLambda_Function_List[uLambda_Function_List.size() - 1].
            lambdaArgumentName[i];
          tempFunction_DS.type = 1;
          tempFunction_DS.translate_PL_Tree = otherArgumentList[i + 1];
          uDefined_List.insert( uDefined_List.begin(), tempFunction_DS );
        } // end for ...
        */
        PL_Tree *temp = CopyTree( uLambda_Function_List[uLambda_Function_List.size() - 1].
                                  lambda_PL_Tree );

        ReplaceArgumentInLambda( otherArgumentList, temp, argumentNum );
        // PrintSExp( temp, 0, true );
        uLambdaUse = false;
        uLambda_Function_List.pop_back();
        try {
          otherArgument = Eval( temp, level + 1, ifQuote );
        } // end try
        catch ( ERROR_EVAL errorEval ) {
          if ( errorEval == ERROR_NO_RETURN_VALUE ) {
            uErrorTree = originalRoot;
            throw ERROR_NO_RETURN_VALUE;
          } // end if ...
          else {
            throw errorEval;
          } // end else
        } // end catch ...

        /*
        try {
          otherArgument = Eval( temp, level+1, ifQuote );
        } // end try ...
        catch ( ERROR_EVAL error_eval ) {
          if ( tempArgumentSize > 0 )
            uDefined_List.erase( uDefined_List.begin(), uDefined_List.begin() + tempArgumentSize );

          throw error_eval;
        } // end catch ...

        if ( tempArgumentSize > 0 )
          uDefined_List.erase( uDefined_List.begin(), uDefined_List.begin() + tempArgumentSize );
        */
        return otherArgument;
      } // end if ...
      else if ( firstArgument == "clean-environment" ) {
        CleanEnvironment();
        cout << "environment cleaned";
        return NULL;
      } // end if ...
      else if ( firstArgument == "cons" ) {
        otherArgument = new PL_Tree;
        otherArgument->parent = NULL;
        otherArgument->left = otherArgumentList[1];

        if ( otherArgumentList[2] == NULL ) otherArgument->right = NULL;
        else if ( otherArgumentList[2]->type == LEFT_PAREN ) {
          PL_Tree *temp = CopyTree( otherArgumentList[2] );
          temp->type = S_EXP;
          otherArgument->right = temp;
        } // end else if ...
        else {
          otherArgument->right = otherArgumentList[2];
        } // end else ...

        otherArgument->type = LEFT_PAREN;
        otherArgument->isAtomString.type = LEFT_PAREN;
        otherArgument->isAtomString.changed = "(";


        return otherArgument;
      } // end else if ...
      else if ( firstArgument == "list" ) {

        if ( otherArgumentList.size() == 1 )
          return ReturnASpecificTree( NIL );

        otherArgument = new PL_Tree;
        otherArgument->parent = NULL;
        otherArgument->left = otherArgumentList[1];
        otherArgument->type = LEFT_PAREN;
        otherArgument->isAtomString.changed = "(";
        otherArgument->isAtomString.type = LEFT_PAREN;

        PL_Tree *root = otherArgument;
        for ( int i = 2 ; i < otherArgumentList.size() ; i++ ) {
          otherArgument->right = new PL_Tree;
          otherArgument->right->parent = otherArgument;
          otherArgument = otherArgument->right;
          otherArgument->type = S_EXP;
          otherArgument->left = otherArgumentList[i];
        } // end for ...

        otherArgument->right = NULL;
        return root;
      } // end else if ...
      else if ( firstArgument == "quote" ) {

        ifQuote = true;
        return otherArgumentList[1];

      } // end else if ...
      else if ( firstArgument == "car" ) {

        if ( otherArgumentList[1] == NULL ) return NULL;

        if ( otherArgumentList[1]->type == ATOM ||
             otherArgumentList[1]->type == SYMBOL ) {
          uErrorToken.changed = "car";
          uErrorTree = otherArgumentList[1];
          throw ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE;
        } // end if

        return otherArgumentList[1]->left;

      } // end else if ...
      else if ( firstArgument == "cdr" ) {

        if ( otherArgumentList[1] == NULL ) return NULL;

        if ( otherArgumentList[1]->type == ATOM ||
             otherArgumentList[1]->type == SYMBOL ) {
          uErrorToken.changed = "cdr";
          uErrorTree = otherArgumentList[1];
          throw ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE;
        } // end if

        if ( otherArgumentList[1]->right == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->right->type == S_EXP ||
             otherArgumentList[1]->right->type == LS_EXP ) {
          PL_Tree *temp = CopyTree( otherArgumentList[1]->right );
          temp->type = LEFT_PAREN;
          temp->isAtomString.type = LEFT_PAREN;
          temp->isAtomString.changed = "(";
          return temp;
        } // end if ...
        else {
          return otherArgumentList[1]->right;
        } // end else ...

      } // end else if ...
      else if ( firstArgument == "atom?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->type == ATOM ) return ReturnASpecificTree( T );
        else if ( otherArgumentList[1]->isAtomString.type == SYMBOL )
          return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "pair?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->type == ATOM ) return ReturnASpecificTree( NIL );
        else return ReturnASpecificTree( T );

      } // end else if ...
      else if ( firstArgument == "list?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( IsList( otherArgumentList[1] ) ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "null?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( T );

        if ( otherArgumentList[1]->isAtomString.type == NIL ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "integer?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->isAtomString.type == INT ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "real?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->isAtomString.type == INT ||
             otherArgumentList[1]->isAtomString.type == FLOAT ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "number?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->isAtomString.type == INT ||
             otherArgumentList[1]->isAtomString.type == FLOAT ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "string?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->isAtomString.type == STRING ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "boolean?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->isAtomString.type == NIL ||
             otherArgumentList[1]->isAtomString.type == T ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "symbol?" ) {

        if ( otherArgumentList[1] == NULL ) return ReturnASpecificTree( NIL );

        if ( otherArgumentList[1]->isAtomString.type == SYMBOL ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "+" || firstArgument == "-" || firstArgument == "*" ||
                firstArgument == "/" ) {

        bool ifEverFloat = false;
        int firstI = 0;
        int secondI = 0;
        double firstF = 0;
        double secondF = 0;

        for ( int i = 1; i < otherArgumentList.size() ; i++ ) {

          if ( otherArgumentList[i]->isAtomString.type != INT &&
               otherArgumentList[i]->isAtomString.type != FLOAT ) {
            uErrorToken.changed = firstArgument;
            uErrorTree = otherArgumentList[i];
            throw ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE;
          } // end if ...

          if ( ifEverFloat ) {
            sscanf( otherArgumentList[i]->isAtomString.changed.c_str(), "%lf", &secondF );
            if ( firstArgument == "+" ) firstF = firstF + secondF;
            else if ( firstArgument == "-" ) firstF = firstF - secondF;
            else if ( firstArgument == "*" ) firstF = firstF * secondF;
            else if ( firstArgument == "/" ) {
              if ( secondF == 0 ) throw ERROR_DIVISION_BY_ZERO;
              firstF = firstF / secondF;
            } // end else if ...
          } // end if ...
          else {
            if ( otherArgumentList[i]->isAtomString.type == FLOAT ) {
              ifEverFloat = true;
              firstF = firstI;
              sscanf( otherArgumentList[i]->isAtomString.changed.c_str(), "%lf", &secondF );
              if ( i == 1 ) {
                firstF = secondF;
              } // end if ...
              else {
                if ( firstArgument == "+" ) firstF = firstF + secondF;
                else if ( firstArgument == "-" ) firstF = firstF - secondF;
                else if ( firstArgument == "*" ) firstF = firstF * secondF;
                else if ( firstArgument == "/" ) {
                  if ( secondF == 0 ) throw ERROR_DIVISION_BY_ZERO;
                  firstF = firstF / secondF;
                } // end else if ...
              } // else ...
            } // end if ...
            else {
              sscanf( otherArgumentList[i]->isAtomString.changed.c_str(), "%d", &secondI );
              if ( uDebug3 ) cout << i << "--" << secondI << endl;
              if ( i == 1 ) {
                firstI = secondI;
              } // end if ...
              else {
                if ( firstArgument == "+" ) firstI = firstI + secondI;
                else if ( firstArgument == "-" ) firstI = firstI - secondI;
                else if ( firstArgument == "*" ) firstI = firstI * secondI;
                else if ( firstArgument == "/" ) {
                  if ( secondI == 0 ) throw ERROR_DIVISION_BY_ZERO;
                  firstI = firstI / secondI;
                } // end else if ...
              } // end else ...
            } // end else ...
          } // end else ...
        } // end for ...

        char *cs = new char[64];
        if ( ifEverFloat ) {
          otherArgument = ReturnASpecificTree( FLOAT );
          sprintf( cs, "%.3f", firstF );
        } // end if ...
        else {
          otherArgument = ReturnASpecificTree( INT );
          sprintf( cs, "%d", firstI );
        } // end else ...

        otherArgument->isAtomString.changed = cs;

        return otherArgument;

      } // end else if ...
      else if ( firstArgument == "not" ) {

        if ( otherArgumentList[1]->isAtomString.type == NIL ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // else if ...
      else if ( firstArgument == ">" || firstArgument == ">=" || firstArgument == "<" ||
                firstArgument == "<=" || firstArgument == "=" ) {

        for ( int i = 1; i < otherArgumentList.size() ; i++ ) {
          if ( otherArgumentList[i]->isAtomString.type != INT &&
               otherArgumentList[i]->isAtomString.type != FLOAT ) {
            uErrorToken.changed = firstArgument;
            uErrorTree = otherArgumentList[i];
            throw ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE;
          } // end if ...
        } // end for ...

        double firstF = 0;
        double secondF = 0;

        for ( int i = 2; i < otherArgumentList.size() ; i++ ) {

          sscanf( otherArgumentList[i - 1]->isAtomString.changed.c_str(), "%lf", &firstF );
          sscanf( otherArgumentList[i]->isAtomString.changed.c_str(), "%lf", &secondF );

          if ( firstArgument == ">" ) {
            if ( uDebug3 ) {
              cout << "first> : " << firstF << endl;
              cout << "second> : " << secondF << endl;
            } // end if ...

            if ( firstF <= secondF ) return ReturnASpecificTree( NIL );
          } // end if ...
          else if ( firstArgument == ">=" ) {
            if ( firstF < secondF ) return ReturnASpecificTree( NIL );
          } // end else if ...
          else if ( firstArgument == "<" ) {
            if ( firstF >= secondF ) return ReturnASpecificTree( NIL );
          } // end else if ...
          else if ( firstArgument == "<=" ) {
            if ( firstF > secondF ) return ReturnASpecificTree( NIL );
          } // end else if ...
          else if ( firstArgument == "=" ) {
            if ( uDebug3 ) {
              cout << "first= : " << firstF << endl;
              cout << "second= : " << secondF << endl;
            } // end if ...

            if ( firstF != secondF ) return ReturnASpecificTree( NIL );
          } // end else if ...
        } // end for ...

        return ReturnASpecificTree( T );

      } // end else if ...
      else if ( firstArgument == "string-append" ) {

        for ( int i = 1; i < otherArgumentList.size() ; i++ ) {
          if ( otherArgumentList[i]->isAtomString.type != STRING ) {
            uErrorToken.changed = firstArgument;
            uErrorTree = otherArgumentList[i];
            throw ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE;
          } // end if ...
        } // end for ...

        string temp = "";
        string temp2 = "";
        for ( int i = 1; i < otherArgumentList.size() ; i++ ) {
          temp2 = otherArgumentList[i]->isAtomString.changed;
          temp2.erase( temp2.begin() );
          temp2.erase( temp2.end() - 1 );
          temp = temp + temp2;
        } // end for ...

        temp.insert( 0, "\"" );
        temp.insert( temp.length(), "\"" );

        otherArgument = NULL;
        otherArgument = ReturnASpecificTree( STRING );
        otherArgument->isAtomString.changed = temp;

        return otherArgument;

      } // end else if ...
      else if ( firstArgument == "string>?" || firstArgument == "string<?" || firstArgument == "string=?" ) {

        for ( int i = 1; i < otherArgumentList.size() ; i++ ) {
          if ( otherArgumentList[i]->isAtomString.type != STRING ) {
            uErrorToken.changed = firstArgument;
            uErrorTree = otherArgumentList[i];
            throw ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE;
          } // end if ...
        } // end for ...

        string firstS;
        string secondS;

        for ( int i = 2; i < otherArgumentList.size() ; i++ ) {

          firstS = otherArgumentList[i - 1]->isAtomString.changed;
          secondS = otherArgumentList[i]->isAtomString.changed;

          if ( firstArgument == "string>?" ) {
            if ( firstS <= secondS )
              return ReturnASpecificTree( NIL );
          } // end if ...
          else if ( firstArgument == "string<?" ) {
            if ( firstS >= secondS )
              return ReturnASpecificTree( NIL );
          } // end else if ...
          else if ( firstArgument == "string=?" ) {
            if ( firstS != secondS )
              return ReturnASpecificTree( NIL );
          } // end else if ...

        } // end for ...

        return ReturnASpecificTree( T );

      } // end else if ...
      else if ( firstArgument == "eqv?" ) {

        if ( uDebug2 ) cout << "L1941 : enter eqv?\n";

        if ( IsEqv( otherArgumentList[1], otherArgumentList[2] ) ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "equal?" ) {

        if ( uDebug2 ) cout << "L1494 : enter equal?\n";

        if ( IsEqual( otherArgumentList[1], otherArgumentList[2] ) ) return ReturnASpecificTree( T );
        else return ReturnASpecificTree( NIL );

      } // end else if ...
      else if ( firstArgument == "begin" ) {

        return otherArgumentList[otherArgumentList.size() - 1];

      } // end else if ...
    } // end if ...
  } // end else ...

  return NULL;
} // Eval()

void InputInternalFunction() {

  uInternalFunction.clear();

  uInternalFunction.push_back( "cons" );
  uInternalFunction.push_back( "list" );
  uInternalFunction.push_back( "quote" );
  uInternalFunction.push_back( "'" );
  uInternalFunction.push_back( "define" );
  uInternalFunction.push_back( "lambda" );
  uInternalFunction.push_back( "let" );
  uInternalFunction.push_back( "car" );
  uInternalFunction.push_back( "cdr" );
  uInternalFunction.push_back( "atom?" );
  uInternalFunction.push_back( "pair?" );
  uInternalFunction.push_back( "list?" );
  uInternalFunction.push_back( "null?" );
  uInternalFunction.push_back( "integer?" );
  uInternalFunction.push_back( "real?" );
  uInternalFunction.push_back( "number?" );
  uInternalFunction.push_back( "string?" );
  uInternalFunction.push_back( "boolean?" );
  uInternalFunction.push_back( "symbol?" );
  uInternalFunction.push_back( "+" );
  uInternalFunction.push_back( "-" );
  uInternalFunction.push_back( "*" );
  uInternalFunction.push_back( "/" );
  uInternalFunction.push_back( "not" );
  uInternalFunction.push_back( "and" );
  uInternalFunction.push_back( "or" );
  uInternalFunction.push_back( ">" );
  uInternalFunction.push_back( ">=" );
  uInternalFunction.push_back( "<" );
  uInternalFunction.push_back( "<=" );
  uInternalFunction.push_back( "=" );
  uInternalFunction.push_back( "string-append" );
  uInternalFunction.push_back( "string>?" );
  uInternalFunction.push_back( "string<?" );
  uInternalFunction.push_back( "string=?" );
  uInternalFunction.push_back( "eqv?" );
  uInternalFunction.push_back( "equal?" );
  uInternalFunction.push_back( "begin" );
  uInternalFunction.push_back( "if" );
  uInternalFunction.push_back( "cond" );
  uInternalFunction.push_back( "clean-environment" );
  uInternalFunction.push_back( "exit" );


} // InputInternalFunction()

int main()
{

  uTotalLine = 1;
  uEOL = false; // End of line
  uUnusedCharBool = false;
  uUnusedChar = '\0';
  uLastChar = '\0';
  uEOF = false;
  uExit = false;
  uIsReadSexp = false;
  uPrePrintLine = 1;
  uPrePrintSexpInSameLine = false;
  uLet_Defined_Num = 0;
  uLambda_Function_List.clear();
  InputInternalFunction(); // Initialize uInternalFunction
  CleanEnvironment();

  int mTestNumber; // Question No.

  if ( !uDebug && !uDebug2 && !uDebug3 ) {
    scanf( "%d", &mTestNumber );
    GetLine();
  } // end if ...

  cout << "Welcome to OurScheme!";
  // if ( mTestNumber == 2 ) cout << "stop\n";
  vector<Token_Type> tempTest;
  string tempString;
  PL_Tree *mS_exp_Root = NULL;
  PL_Tree *tempReturn = NULL;
  bool ifQuote = false;
  do {

    cout << "\n\n> ";

    mS_exp_Root = new PL_Tree;
    mS_exp_Root->parent = NULL;

    try
    {

      uLine = 1;
      if ( !uPrePrintSexpInSameLine ) uColume = 0;
      else {
        uColume = 1;
        uPrePrintSexpInSameLine = false;
      } // end else ...

      ReadSExp( mS_exp_Root, "" );

      if ( uDebug ) {
        cout << "Have  " << tempTest.size() << "  tokens :\n";
        for ( int i = 0 ; i < tempTest.size() ; i++ ) {
          cout << tempTest[i].changed << "\ntype :" << ReturnType( tempTest[i].type ) << endl;
        } // end for ...

        cout << endl;
      } // end if ...

      ifQuote = false;
      if ( !IsExit( mS_exp_Root ) ) {
        tempReturn = Eval( mS_exp_Root, 1, ifQuote );
        InitializeReplace( tempReturn );
        PrintSExp( tempReturn, 0, ifQuote );
      } // end if ...
      else cout << endl;

      uPrePrintLine = uTotalLine;
      uIsReadSexp = false;
      uLambdaUse = false;
      uLambda_Function_List.clear();

    }
    catch ( ERROR_READ errorRead )
    {
      if ( errorRead == ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN ) {
        cout << "ERROR (unexpected token) : atom or '(' expected when token at Line ";
        printf( "%d Column %d is >>%s<<", uErrorToken.tokenLine, uErrorToken.tokenColume,
                uErrorToken.original.c_str() );
      } // end if ...
      else if ( errorRead == ERROR_UNEXPECTED_TOKEN_RIGHT_PAREN ) {
        cout << "ERROR (unexpected token) : ')' expected when token at Line ";
        printf( "%d Column %d is >>%s<<", uErrorToken.tokenLine, uErrorToken.tokenColume,
                uErrorToken.original.c_str() );
      } // end else if ...
      else if ( errorRead == ERROR_NO_CLOSING_QUOTE ) {
        printf( "ERROR (no closing quote) : END-OF-LINE encountered at Line %d Column %d",
                uErrorToken.tokenLine, uErrorToken.tokenColume );
      } // end else if ...
      else if ( errorRead == ERROR_NO_MORE_INPUT ) {
        uEOF = true;
      } // end else if ...

      uUnusedCharBool = false;

      if ( errorRead < ERROR_NO_CLOSING_QUOTE && uLastChar != '\n' ) {
        try
        {
          uPrePrintSexpInSameLine = false;
          GetLine();
        }
        catch ( ERROR_READ error )
        {
          if ( error == ERROR_NO_MORE_INPUT ) {
            uEOF = true;
            cout << "\n\n> ";
          } // end if ...
        } // end catch ...
      } // end if ...
    } // end catch ...
    catch ( ERROR_EVAL errorEval ) {

      uPrePrintLine = uTotalLine;
      uIsReadSexp = false;
      uLambdaUse = false;
      uLambda_Function_List.clear();

      if ( errorEval == ERROR_UNBOUND_SYMBOY ) {
        cout << "ERROR (unbound symbol) : " << uErrorToken.changed;
      } // end if ...
      else if ( errorEval == ERROR_NON_LIST ) {
        cout << "ERROR (non-list) : ";
        InitializeReplace( uErrorTree );
        PrintSExp( uErrorTree, 0, true );
      } // end else if ...
      else if ( errorEval == ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION ) {
        cout << "ERROR (attempt to apply non-function) : " << uErrorToken.changed;
      } // end else if ...
      else if ( errorEval == ERROR_ATTEMPT_TO_APPLY_NON_FUNCTION2 ) {
        cout << "ERROR (attempt to apply non-function) : ";

        PrintSExp( uErrorTree, 0, false );
      } // end else if ...
      else if ( errorEval == ERROR_LEVEL_OF_CLEAN_ENVIRONMENT ) {
        cout << "ERROR (level of CLEAN-ENVIRONMENT)";
      } // end else if ...
      else if ( errorEval == ERROR_LEVEL_OF_DEFINE ) {
        cout << "ERROR (level of DEFINE)";
      } // end else if ...
      else if ( errorEval == ERROR_LEVEL_OF_EXIT ) {
        cout << "ERROR (level of EXIT)";
      } // end else if ...
      else if ( errorEval == ERROR_INCORRECT_NUMBER_OF_ARGUMENTS ) {
        cout << "ERROR (incorrect number of arguments) : " << uErrorToken.changed;
      } // end else if ...
      else if ( errorEval == ERROR_DEFINE_FORMAT ) {
        cout << "ERROR (DEFINE format) : ";
        PrintSExp( uErrorTree, 0, true );
      } // end else if ...
      else if ( errorEval == ERROR_COND_FORMAT ) {
        cout << "ERROR (COND format) : ";
        // InitializeReplace( uErrorTree );
        PrintSExp( uErrorTree, 0, true );
      } // end else if ...
      else if ( errorEval == ERROR_LAMBDA_FORMAT ) {
        cout << "ERROR (LAMBDA format) : ";
        PrintSExp( uErrorTree, 0, true );
      } // end else if
      else if ( errorEval == ERROR_LET_FORMAT ) {
        cout << "ERROR (LET format) : ";
        PrintSExp( uErrorTree, 0, true );
      } // end else if
      else if ( errorEval == ERROR_XXX_WITH_INCORRECT_ARGUMENT_TYPE ) {
        cout << "ERROR (" << uErrorToken.changed << " with incorrect argument type) : ";
        InitializeReplace( uErrorTree );
        PrintSExp( uErrorTree, 0, ifQuote );
      } // end else if ERROR_DIVISION_BY_ZERO
      else if ( errorEval == ERROR_DIVISION_BY_ZERO ) {
        cout << "ERROR (division by zero) : /";
      } // end else if
      else if ( errorEval == ERROR_NO_RETURN_VALUE ) {
        cout << "ERROR (no return value) : ";
        PrintSExp( uErrorTree, 0, true );
      } // end else if
      else if ( errorEval == ERROR_NO_RETURN_VALUE_TO_MAIN ) {
        cout << "ERROR (no return value) : ";
        PrintSExp( uErrorTree, 0, true );
      } // end else if
      else if ( errorEval == ERROR_UNBOUND_PARAMETER ) {
        cout << "ERROR (unbound parameter) : ";
        PrintSExp( uErrorTree, 0, true );
      } // end else if
      else if ( errorEval == ERROR_UNBOUND_CONDITION ) {
        cout << "ERROR (unbound condition) : ";
        PrintSExp( uErrorTree, 0, true );
      } // end else if

      InitializeReplace( uErrorTree );
    } // end catch ...

  } while ( !IsExit( mS_exp_Root ) && !uEOF );

  if ( uEOF ) cout << "ERROR (no more input) : END-OF-FILE encountered\n";
  cout << "Thanks for using OurScheme!";

  // system( "pause" );
  return 0;
} // main()
