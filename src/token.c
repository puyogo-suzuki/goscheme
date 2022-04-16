#include "token.h"
#include <stdio.h>

tokenizer_t
tokenizer_new(string_t str) {
    return (tokenizer_t){ 0, str };
}

bool
isNumChar(char ch) {
    return ch >= '0' && ch <= '9';
}

bool
tokenizer_next(tokenizer_t * self, token_t * out_token) {
    size_t startPos;
    string_t subview;
    char cur = '\0';
    while(true) {
        if(string_getAt(&self->str, self->position, &cur) != ERR_SUCCESS)
            return false;
        switch(cur) {
            case ' ': case '\n': break;
            case '(':
                out_token->tokenKind = TOKEN_PAREN_OPEN;
                goto L_PAREN;
            case ')':
                out_token->tokenKind = TOKEN_PAREN_CLOSE;
                goto L_PAREN;
            case '\'':
                self->position++;
                if(string_getAt(&self->str, self->position, &cur) != ERR_SUCCESS)
                    return false;
                if(cur != '(') goto L_FAIL;
                self->position++;
                out_token->tokenKind = TOKEN_QUOTE_PAREN_OPEN;
                goto L_PAREN;
            default: goto L_BREAK_WHILE;
        }
        self->position++;
    } L_BREAK_WHILE:


    startPos = self->position;
    bool isString = cur == '"';
    bool isNum = isNumChar(cur);
    self->position++;

    while(string_getAt(&self->str, self->position, &cur) == ERR_SUCCESS) {
        switch(cur) {
            case '\n': goto L_OTHER;
            case ' ': case '(': case ')': if (!isString) goto L_OTHER; else break;
            case '"': if (isString) goto L_STRING; else goto L_FAIL;
            default: if(isNum && !isNumChar(cur)) goto L_FAIL; else break;
        }
        self->position++;
    }

    if(self->position != startPos) {
        if(self->position - startPos == 1 && isString)
            goto L_FAIL;
        goto L_OTHER;
    } else goto L_FAIL;

L_PAREN:
    string_default(&out_token->value.strValue);
    self->position++;
    return true;

L_STRING:
    self->position++;
    out_token->tokenKind = TOKEN_STRING;
    return string_substring_deep(&out_token->value.strValue, &self->str, startPos + 1, self->position - startPos - 2) == ERR_SUCCESS;

L_OTHER:
    string_substring_shallow(&subview, &self->str, startPos, self->position - startPos);
    if (isNum) {
        out_token->tokenKind = TOKEN_NUMERIC;
        return string_parseInt(&subview, &out_token->value.numValue) == ERR_SUCCESS;
    } else {
        out_token->tokenKind = TOKEN_SYMBOL;
        return string_copy(&out_token->value.strValue, &subview) == ERR_SUCCESS;
    }

L_FAIL:
    self->position++;
    return false;
}

error_t
token_toString(token_t * self, string_t * out) {
    char str_str[] = "TOKEN_STRING(";
    char str_sym[] = "TOKEN_SYMBOL(";
    int str_first_size = self->tokenKind == TOKEN_STRING ? sizeof(str_str) - 1 : sizeof(str_sym) - 1;
    char * str_first = self->tokenKind == TOKEN_STRING ? str_str : str_sym;
    switch (self->tokenKind) {
    case TOKEN_PAREN_OPEN: return string_new_deep2(out, "TOKEN_PAREN_OPEN");
    case TOKEN_QUOTE_PAREN_OPEN: return string_new_deep2(out, "TOKEN_QUOTE_PAREN_OPEN");
    case TOKEN_PAREN_CLOSE: return string_new_deep2(out, "TOKEN_PAREN_CLOSE");
    case TOKEN_NUMERIC: {
        char buf[40]; // 40あれば十分でしょ
        sprintf(buf, "TOKEN_NUMERIC(%d)", self->value.numValue);
        return string_new_deep2(out, buf);
    }
    case TOKEN_STRING:
    case TOKEN_SYMBOL: {
        string_t first;
        string_new_shallow(&first, str_first, str_first_size);
        string_t last;
        string_new_shallow(&last, ")", 1);
        CHKERROR(string_new(out, str_first_size + 1 + string_getLength(&self->value.strValue)))
        string_overWrite(out, &first, 0);
        string_overWrite(out, &self->value.strValue, str_first_size);
        string_overWrite(out, &last, string_getLength(out) - 1);
        return ERR_SUCCESS;
    }
    default: return ERR_ILLEGAL_STATE;
    }
}
