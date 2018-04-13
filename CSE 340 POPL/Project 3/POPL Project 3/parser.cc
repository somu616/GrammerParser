/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"

using namespace std;

void Parser::syntax_error()
{
    cout << "Syntax Error\n";
    exit(1);
}



struct scopeList * currScope = NULL;
struct scopeList * prevScope = NULL;
TokenType leftType;
TokenType currentType;


struct typeList {
    string name;
    TokenType varOrType;
    TokenType typeName;
    int lineNumber;
};

struct symbolList {
    string name;
    int lineNoDeclared;
    int lineNoUsed;
};

struct scopeList {
    vector<typeList> list;
    scopeList * parent = NULL;
};

vector<typeList> list1;
vector<string> error;
vector<symbolList> symbolList1;

void createAndAddNode(Token t) {
    vector<typeList> allList = currScope->list;
    for (int i = 0; i< allList.size(); i++) {
        if (t.lexeme == allList[i].name) {
            if (allList[i].varOrType == currentType) {
                if (currentType == VAR) {
                    cout <<"ERROR CODE 2.1 "<<t.lexeme;
                    exit(1);
                }
                else if (currentType == TYPE) {
                    cout <<"ERROR CODE 1.1 "<<t.lexeme;
                    exit(1);
                }
            }
            else {
                if (currentType == VAR) {
                    cout <<"ERROR CODE 1.2 "<<t.lexeme;
                    exit(1);
                }
                else if (currentType == TYPE) {
                    cout << "ERROR CODE 2.2 " << t.lexeme;
                    exit(1);
                }
            }
        }
    }
    typeList node1;
    node1.name = t.lexeme;
    node1.lineNumber = t.line_no;
    node1.varOrType = currentType;
    list1.push_back(node1);
}

TokenType lookupInSymbolTable(Token tt, scopeList * local_scope) {
    if (local_scope == NULL) {
        cout <<"ERROR CODE 1.4 "<<tt.lexeme;
        exit(1);
    }
    vector<typeList> tempList = local_scope->list;
    for (int i = 0; i< tempList.size();i++) {
        if(tempList[i].name == tt.lexeme) {
            if (tempList[i].varOrType == VAR) {
                cout <<"ERROR CODE 2.3 "<<tt.lexeme;
                exit(1);
            }
            else {
                symbolList newNode;
                newNode.name = tt.lexeme;
                newNode.lineNoDeclared = tempList[i].lineNumber;
                newNode.lineNoUsed = tt.line_no;
                symbolList1.push_back(newNode);
                return tempList[i].typeName;
            }
        }
    }
    return lookupInSymbolTable(tt, local_scope->parent);
}


TokenType lookupForType(Token tt, scopeList * local_scope) {
    if (local_scope == NULL) {
        cout <<"ERROR CODE 2.4 "<<tt.lexeme;
        exit(1);
    }
    vector<typeList> tempList = local_scope->list;
    for (int i = 0; i< tempList.size();i++) {
        if(tempList[i].name == tt.lexeme) {
            if (tempList[i].varOrType == TYPE) {
                cout <<"ERROR CODE 1.3 "<<tt.lexeme;
                exit(1);
            }
            else {
                symbolList newNode;
                newNode.name = tt.lexeme;
                newNode.lineNoDeclared = tempList[i].lineNumber;
                newNode.lineNoUsed = tt.line_no;
                symbolList1.push_back(newNode);
                return tempList[i].typeName;
            }
        }
    }
    return lookupForType(tt, local_scope->parent);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

// Parsing

void Parser::parse_program()
{
    // program -> scope

    parse_scope();
}

void Parser::parse_scope()
{
    // scope -> { scope_list }
    expect(LBRACE);
    if (currScope != NULL) {
        prevScope = currScope;
    }
    currScope=new scopeList();
    currScope->parent = prevScope;
    parse_scope_list();
    expect(RBRACE);
    if (prevScope != NULL) {
        currScope= prevScope;
        prevScope = currScope->parent;
    }
}


void Parser::parse_scope_list()
{
    // scope_list -> stmt
    // scope_list -> scope
    // scope_list -> declaration
    // scope_list -> stmt scope_list
    // scope_list -> scope scope_list
    // scope_list -> declaration scope_list

    // TODO
    if(peek().token_type == WHILE || peek().token_type == ID) {
        parse_stmt();
        if (peek().token_type == LBRACE || peek().token_type == ID || peek().token_type == WHILE || peek().token_type == TYPE || peek().token_type == VAR) {
            parse_scope_list();
        }
    }
    else if(peek().token_type == TYPE || peek().token_type == VAR) {
        // scope_list -> declaration
        // scope_list -> declaration scope_list
        parse_declaration();
        if (peek().token_type == LBRACE || peek().token_type == ID || peek().token_type == WHILE || peek().token_type == TYPE || peek().token_type == VAR) {
            parse_scope_list();
        }
    }
    else if(peek().token_type == LBRACE ) {
        // scope_list -> scope
        // scope_list -> scope scope_list
        parse_scope();
        if (peek().token_type == LBRACE || peek().token_type == ID || peek().token_type == WHILE || peek().token_type == TYPE || peek().token_type == VAR) {
            parse_scope_list();
        }
    }
    else
        syntax_error();

}

void Parser::parse_declaration()
{
    // declaration -> type_decl
    // declaration -> var_decl

    Token t = peek();
    if (t.token_type == TYPE)
        parse_type_decl();
    if (t.token_type == VAR)
        parse_var_decl();
}

void assignSymbolTypes(TokenType type) {
    for(int x=0;x<list1.size();x++) {
        list1[x].typeName = type;
        currScope->list.push_back(list1[x]);
    }
}

void Parser::parse_type_decl()
{
    // type_decl -> TYPE id_list COLON type_name SEMICOLON

    expect(TYPE);
    currentType = TYPE;
    parse_id_list();
    expect(COLON);
    parse_type_name();
    expect(SEMICOLON);
    list1.clear();
    currentType = ERROR;
}

void Parser::parse_type_name()
{
    // type_name -> REAL
    // type_name -> INT
    // type_name -> BOOLEAN
    // type_name -> STRING
    // type_name -> LONG
    // type_name -> ID

    // TODO
    Token tt = peek();
    if(tt.token_type == REAL) {
        expect(REAL);
        assignSymbolTypes(tt.token_type);
    }
    else if(tt.token_type == INT) {
        expect(INT);
        assignSymbolTypes(tt.token_type);
    }
    else if(tt.token_type == BOOLEAN) {
        expect(BOOLEAN);
        assignSymbolTypes(tt.token_type);
    }
    else if(tt.token_type == STRING) {
        expect(STRING);
        assignSymbolTypes(tt.token_type);
    }
    else if(tt.token_type == LONG) {
        expect(LONG);
        assignSymbolTypes(tt.token_type);
    }
    else if (tt.token_type == ID)
    {
        expect(ID);
        TokenType tp = lookupInSymbolTable(tt, currScope);
        assignSymbolTypes(tp);
    }
    else
    {
        syntax_error();
    }


}


void Parser::parse_var_decl()
{
    // var_decl -> VAR id_list COLON type_name SEMICOLON

    expect(VAR);
    currentType = VAR;
    parse_id_list();
    expect(COLON);
    parse_type_name();
    expect(SEMICOLON);
    list1.clear();
    currentType=ERROR;
}

void Parser::parse_id_list()
{
    // id_list -> ID
    // id_list -> ID COMMA id_list

    // TODO
    Token tt = expect(ID);
    createAndAddNode(tt);
    if(peek().token_type == COMMA){
        expect(COMMA);
        parse_id_list();
    }
    else if(peek().token_type == COLON){

    }
    else
        syntax_error();
}

void Parser::parse_stmt_list()
{
    // stmt_list -> stmt
    // stmt_list -> stmt stmt_list

    parse_stmt();
    Token t = peek();
    if (t.token_type == WHILE || t.token_type == ID)
    {
        // stmt_list -> stmt stmt_list
        parse_stmt_list();
    }
    else if (t.token_type == RBRACE)
    {
        // stmt_list -> stmt
    }
    else
    {
        syntax_error();
    }
}

void Parser::parse_stmt()
{
    // stmt -> assign_stmt
    // stmt -> while_stmt
    // TODO
    Token t = peek();
    if (t.token_type == ID)
    {
        // stmt -> assign_stmt
        parse_assign_stmt();
    }
    else if (t.token_type == WHILE)
    {
        // stmt -> while_stmt
        parse_while_stmt();
    }
    else {
        syntax_error();
    }
}
bool assign = false;
void Parser::parse_assign_stmt()
{
    // assign_stmt -> ID EQUAL expr SEMICOLON
    // TODO
    Token tt = expect(ID);

    leftType = tt.token_type;
    lookupForType(tt, currScope);
    expect(EQUAL);
    assign = true;
    parse_expr();
    expect(SEMICOLON);
    assign = false;
}

void Parser::parse_while_stmt()
{
    // while_stmt -> WHILE condition LBRACE stmt list RBRACE
    // TODO
    expect(WHILE);
    parse_condition();
    expect(LBRACE);
    parse_stmt_list();
    expect(RBRACE);
}

void Parser::parse_expr()
{
    // expr -> term
    // expr -> term + expr

    // TODO
    parse_term();
    Token t = peek();
    if (t.token_type == PLUS) {
        expect(PLUS);
        parse_expr();
        // expr -> term + expr
    }
    else if (t.token_type == RPAREN || t.token_type == SEMICOLON) {
    }
    else {
        syntax_error();
    }
}

void Parser::parse_term()
{
    // term -> factor
    // term -> factor MULT term

    // TODO
    parse_factor();
    if(peek().token_type == MULT) {
        // term -> factor MULT term
        expect(MULT);
        parse_term();
    }
    else if(peek().token_type == SEMICOLON || peek().token_type == RPAREN || peek().token_type == PLUS ) {
        // term -> factor
    }
    else {
        syntax_error();
    }
}

void Parser::parse_factor()
{
    // factor -> LPAREN expr RPAREN
    // factor -> NUM
    // factor -> REALNUM
    // factor -> ID

    // TODO

    Token tt = peek();
    if(tt.token_type == LPAREN) {
        expect(LPAREN);
        parse_expr();
        expect(RPAREN);
    }
    else if(tt.token_type == NUM)
        expect(NUM);
    else if(tt.token_type == REALNUM)
        expect(REALNUM);
    else if(tt.token_type == ID) {
        tt = expect(ID);
        TokenType ttt = lookupForType(tt, currScope);
//        if(assign && leftType != ttt){
//            cout<<"TYPE MISMATCH "<<tt.line_no<<" C1";
//            exit(1);
//        }
        //factor -> ID
    }
    else if(tt.token_type == MULT || tt.token_type == PLUS) {

    }
    else {
        syntax_error();
    }

}

void Parser::parse_condition()
{
    // condition -> ID
    // condition -> primary relop primary

    // TODO
    Token t = lexer.GetToken();
    if (t.token_type == ID) {
        TokenType ttt = lookupForType(t, currScope);
//        if(ttt != BOOLEAN) {
//            cout<<"TYPE MISMATCH "<<t.line_no<<" C4";
//            exit(1);
//        }
        Token t2 = peek();

        if(t2.token_type == GREATER || t2.token_type == GTEQ || t2.token_type == LESS || t2.token_type == NOTEQUAL || t2.token_type == LTEQ) {
            lexer.UngetToken(t);
            parse_primary();
            parse_relop();
            parse_primary();
            // condition -> primary relop primary
        }

        else if (t2.token_type == LBRACE || t2.token_type == SEMICOLON) {
            // condition -> ID
        }
        else {
//
        }
    } else if (t.token_type == NUM || t.token_type == REALNUM) {
        lexer.UngetToken(t);
        parse_primary();
        parse_relop();
        parse_primary();
        // condition -> primary relop primary
    }
}

void Parser::parse_primary()
{
    // primary -> ID
    // primary -> NUM
    // primary -> REALNUM
    // TODO

    Token t = peek();
    if (t.token_type == ID) {
        expect(ID);
        lookupForType(t, currScope);
    }
    else if (t.token_type == NUM) {
        expect(NUM);
    }
    else if (t.token_type == REALNUM) {
        expect(REALNUM);

    }
    else {
        syntax_error();
    }
}

void Parser::parse_relop()
{
    // relop -> GREATER
    // relop -> GTEQ
    // relop -> LESS
    // relop -> NOTEQ
    // relop -> LTEQ

    // TODO
    Token t = peek();
    if (t.token_type == GREATER) {
        expect(GREATER);
    }
    else if (t.token_type == GTEQ) {
        expect(GTEQ);
    }
    else if (t.token_type == GTEQ) {
        expect(GTEQ);
    }
    else if (t.token_type == GTEQ) {
        expect(GTEQ);
    }
    else if (t.token_type == LTEQ) {
        expect(LTEQ);
    } else {
        syntax_error();
    }
}

void Parser::ParseInput()
{
    parse_program();
    expect(END_OF_FILE);
}

int main()
{
    Parser parser;
    parser.ParseInput();
    //printing for successfull cases
    for (int i = 0; i< symbolList1.size(); i++) {
        cout << symbolList1[i].name << " " << symbolList1[i].lineNoUsed << " " << symbolList1[i].lineNoDeclared << endl;
    }
}