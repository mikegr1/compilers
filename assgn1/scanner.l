
/* definitions */

%{
#include<stdio.h>
#include"tokendef.h"

int scancol = 1;
int yycol = 1;
int scanlineno = 1;

char* yyerror;

void updateCol();       //This function keeps track of column numbers
void countLines();      //This function keeps track of line numbers
int processString();    //This function checks for illegal escape sequences in string literals and print out escape sequences correctly.
%}

newline        \n
whitespace     [ \t]
integer        0|[1-9][0-9]*
integerlead0   0[0-9]+
character      \'(\\?.)\'

/* String can contain any characters between the double quotes other than a newline or
// unescaped doublequotes.
*/
string         \"([^"\n]|\\.)*\"

/* multiline string has \n somewhere in the middle */
multlnstring   ["]([^"]|\\.)*["]

/* If the end quote is found, one of the two regexes above will be matched. Otherwise,
// the string is unterminated.
*/
untermstring   \"[^"]+

comment        \/*"([^*]|\*+[^*/])*\*+"/"
multlncomment  \/\*[*]*([^*\/][^*]*[*]+)*\/
untermcomment  \/\*([^*][^\/]*|[*][\/])

identifier     [A-Za-z][A-Za-z0-9_]*
illidentifier  [0-9]*[A-Za-z_][A-Za-z0-9_]*

yylineno = 1;

%%

 /* rules */

 /* Keywords */
if     {updateCol();return KWD_IF;}
else   {updateCol();return KWD_ELSE;}
while  {updateCol();return KWD_WHILE;}
int    {updateCol();return KWD_INT;}
string {updateCol();return KWD_STRING;}
char   {updateCol();return KWD_CHAR;}
return {updateCol();return KWD_RETURN;}
void   {updateCol();return KWD_VOID;}  

 /* Operators */
\+      {updateCol();return OPER_ADD;}
\-      {updateCol();return OPER_SUB;}
\*      {updateCol();return OPER_MUL;}
\/      {updateCol();return OPER_DIV;}
\%      {updateCol();return OPER_MOD;}
\<      {updateCol();return OPER_LT;}
\>      {updateCol();return OPER_GT;}
\>\=    {updateCol();return OPER_GTE;}
\<\=    {updateCol();return OPER_LTE;}
\=\=    {updateCol();return OPER_EQ;}
\!\=    {updateCol();return OPER_NEQ;}
\=      {updateCol();return OPER_ASGN;}
\!      {updateCol();return OPER_NOT;}
\@      {updateCol();return OPER_AT;}
\&\&    {updateCol();return OPER_AND;}
\|\|    {updateCol();return OPER_OR;}
\+\+    {updateCol();return OPER_INCR;}
\-\-    {updateCol();return OPER_DECR;}

 /* Punctuation */
\[      {updateCol();return LSQ_BRKT;}
\]      {updateCol();return RSQ_BRKT;}
\{      {updateCol();return LCRLY_BRKT;}
\}      {updateCol();return RCRLY_BRKT;}
\(      {updateCol();return LPAREN;}
\)      {updateCol();return RPAREN;}
\;      {updateCol();return SEMICLN;}
\,      {updateCol();return COMMA;}

 /* Identifiers */
{identifier}    {updateCol();return ID;}
{illidentifier} {updateCol();yyerror = "Identifiers may not start with a number(0-9)"; return ERROR;}

 /* Constants */
{integer}       {updateCol();return INTCONST;}
{integerlead0}  {updateCol();yyerror = "Integers may not have leading zeros"; return ERROR;}
{character}     {updateCol();return CHARCONST;}
{string}        {updateCol(); return processString();}
{untermstring}  {updateCol(); yyerror = "Unterminated string"; return ERROR;}
{multlnstring}  {countLines(); yyerror = "String spans multiple lines"; return ERROR;}

 /* Comments */

{multlncomment} {countLines();/* Do Nothing */}
{untermcomment} {updateCol(); yyerror = "Unterminated Comment"; return ERROR;}

 /* Other */
{newline}       {scanlineno++;scancol=1;}
{whitespace}    {scancol++;}
.               {return ILLEGAL_TOK;}

%%

/* user routines */

void updateCol()
{
        yycol = scancol;
        yylineno = scanlineno;
        scancol = scancol + yyleng;
}

void countLines(){
        
        yylineno = scanlineno;
        yycol = scancol;
        int num = 0;
    
	while(yytext[num] != '\0'){
        if(yytext[num]=='\n')
        {
               scanlineno++;
               scancol = 1;
        }
        else
        {
              scancol++;
        }
        num++;
        }

}

int processString(){

        int num = 0;
        int num2 = 0;
        char * str;
        str = (char *) malloc(yyleng + 1);
        
        while(yytext[num] != '\0'){
                if(yytext[num] == '\\') 
                {
                        num++;
                        if(yytext[num] == 'n')
                            str[num2] = '\n';
                        else if(yytext[num] == 't')
                            str[num2] = '\t';
                        else if(yytext[num] == '"')
                            str[num2] = '\"';
                        else if(yytext[num] =='\\')
                            str[num2] = '\\';
                        else{
                        	yycol += num;
                            yyerror ="Unrecognized escape character in String";
                               return ERROR;
                        }                
                }
                else
                {
                        str[num2] = yytext[num];
                }
                num++;
                num2++;
        }

        str[num2] = '\0';
        strcpy(yytext, str);
        free(str);
	return STRCONST;
	
}
