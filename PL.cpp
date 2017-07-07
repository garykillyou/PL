# include <ctype.h>
# include <iostream>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <vector>

using namespace std;

static bool uDebug = false;

// 用於標示token型態
enum TYPE {
  //       1                2    3      4    5      6
  LEFT_PAREN = 1, RIGHT_PAREN, DOT, QUOTE, INT, FLOAT,
  STRING, NIL, T, SYMBOL, ATOM, S_EXP, LS_EXP
  //   7    8  9      10    11     12      13
};

enum ERROR {
  //                                  41                                       42
  ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN = 41, ERROR_UNEXPECTED_TOKEN_RIGHT_PAREN,
  ERROR_NO_CLOSING_QUOTE, ERROR_NO_MORE_INPUT
  //                  43                   44
};

struct Token_Type
{
  string original;
  int type;
  string changed;
  int tokenLine;
  int tokenColume;
};

struct PL_Tree
{
  int type;
  PL_Tree *left;
  PL_Tree *right;
  Token_Type isAtomString;
  PL_Tree *parent;
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

void GetLine() {
  char temp;
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

  string tempToken;

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

  char tempChar;

  bool continuteGetToken = true;

  do {

    tempChar = getchar();
    uLastChar = tempChar;
    uColume++;

    uTokenLine = uLine;
    uTokenColume = uColume;

    if ( tempChar == EOF ) {
      throw ERROR_NO_MORE_INPUT;
    } // end if ...

    if ( isspace( tempChar ) ) {
      if ( tempToken.length() > 0 ) {
        if ( tempToken[0] == '"' ) {
          tempToken = tempToken + tempChar;
          if ( tempChar == '\n' ) {
            continuteGetToken = false;
            uLine = 1;
            uColume = 0;
            uUnAddTotalLine = true;
          } // end if ...
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


/*
void PrintSExp( vector<Token_Type> mVector ) {

  int temp = mVector.size();

  if ( temp == 1 ) {
    if ( mVector[0].type == SYMBOL && mVector[0].changed == ";" ) {

    } // end if ...
    else {
      cout << "\n> " << mVector[0].changed << endl;
    } // end else ...
  } // end if ...
  else if ( temp == 2 ) {
    if ( mVector[0].changed == "(" && mVector[1].changed == ")"
         && mVector[1].tokenColume - mVector[0].tokenColume == 1 ) {
      cout << "\n> nil" << endl;
      mVector.pop_back();
      mVector[0].original = "()";
      mVector[0].type = NIL;
      mVector[0].changed = "nil";
    } // end if ...
  } // end else if ...
  else {

  } // end else ...

  return;
} // PrintSExp()
*/

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

  /*
  if ( uUnusedCharBool ) tempToken_Type.tokenColume = uColume - 1;
  else tempToken_Type.tokenColume = uColume;
  */
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
      float f; // temp
      sscanf( original.c_str(), "%f", &f );
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
      nextNode->isAtomString.tokenColume = uColume;
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
      nextNode->type = ATOM;
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
      nextNode->isAtomString.tokenColume = uColume;
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

void PrintSExp( PL_Tree *root, int spaceNum ) {

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
      cout << root->isAtomString.changed;
    } // end else if ...

    if ( root->left != NULL ) PrintSExp( root->left, spaceNum );

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

      if ( root->right->isAtomString.type != NIL ) PrintSExp( root->right, spaceNum );
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

    if ( root->left != NULL ) PrintSExp( root->left, spaceNum );
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

      if ( root->right->isAtomString.type != NIL ) PrintSExp( root->right, spaceNum );
    } // end if ...

    return;
  } // end else ...
} // PrintSExp()


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

  int mTestNumber;

  if ( !uDebug ) {
    scanf( "%d", &mTestNumber );
    GetLine();
  } // end if ...

  cout << "Welcome to OurScheme!";
  // if ( mTestNumber == 2 ) cout << "stop\n";
  vector<Token_Type> tempTest;
  string tempString;
  PL_Tree *mS_exp_Root;

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

      if ( !IsExit( mS_exp_Root ) ) PrintSExp( mS_exp_Root, 0 );
      else cout << endl;

      uPrePrintLine = uTotalLine;
      uIsReadSexp = false;

    }
    catch ( ERROR error )
    {
      if ( error == ERROR_UNEXPECTED_TOKEN_ATOM_LEFT_PAREN ) {
        cout << "ERROR (unexpected token) : atom or '(' expected when token at Line ";
        printf( "%d Column %d is >>%s<<", uErrorToken.tokenLine, uErrorToken.tokenColume,
                uErrorToken.original.c_str() );
      } // end if ...
      else if ( error == ERROR_UNEXPECTED_TOKEN_RIGHT_PAREN ) {
        cout << "ERROR (unexpected token) : ')' expected when token at Line ";
        printf( "%d Column %d is >>%s<<", uErrorToken.tokenLine, uErrorToken.tokenColume,
                uErrorToken.original.c_str() );
      } // end else if ...
      else if ( error == ERROR_NO_CLOSING_QUOTE ) {
        printf( "ERROR (no closing quote) : END-OF-LINE encountered at Line %d Column %d",
                uErrorToken.tokenLine, uErrorToken.tokenColume );
      } // end else if ...
      else if ( error == ERROR_NO_MORE_INPUT ) {
        uEOF = true;
        // cout << "ERROR_no_more_inputn";
      } // end else if ...

      uUnusedCharBool = false;

      if ( error < ERROR_NO_CLOSING_QUOTE && uLastChar != '\n' ) {
        try
        {
          uPrePrintSexpInSameLine = false;
          GetLine();
        }
        catch ( ERROR error )
        {
          if ( error == ERROR_NO_MORE_INPUT ) {
            uEOF = true;
            cout << "\n\n> ";
            // cout << "ERROR_no_more_inputn";
          } // end if ...
        } // end catch ...
      } // end if ...
    } // end catch ...


  } while ( !IsExit( mS_exp_Root ) && !uEOF );

  if ( !uExit ) cout << "ERROR (no more input) : END-OF-FILE encountered\n";
  cout << "Thanks for using OurScheme!";

  // system( "pause" );
  return 0;
} // main()

