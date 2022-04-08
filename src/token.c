#include "token.h"

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
    int32_t startPos;
    string_t subview;
    while(true) {
        if(self->position >= self->str.length)
            return false;
        switch(self->str.buffer[self->position]) {
            case ' ': break;
            case '(': goto L_PAREN_OPEN;
            case ')': goto L_PAREN_CLOSE;
            default: goto L_BREAK_WHILE;
        }
        self->position++;
    } L_BREAK_WHILE:


    startPos = self->position;
    bool isString = self->str.buffer[self->position] == '"';
    bool isNum = isNumChar(self->str.buffer[self->position]);
    self->position++;

    while(self->position < self->str.length) {
        char cur = self->str.buffer[self->position];
        switch(cur) {
            case ' ': case '(': case ')': if (!isString) goto L_OTHER; else break;
            case '"': if (isString) { self->position++; goto L_OTHER; } else goto L_FAIL;
            default: if(isNum && !isNumChar(cur)) goto L_FAIL; else break;
        }
        self->position++;
    }

    if(self->position != startPos) {
        if(self->position - startPos == 1 && isString)
            goto L_FAIL;
        goto L_OTHER;
    } else goto L_FAIL;

L_PAREN_OPEN:
    out_token->tokenKind = TOKEN_PAREN_OPEN;
    out_token->value.strValue.buffer = NULL;
    self->position++;
    return true;

L_PAREN_CLOSE:
    out_token->tokenKind = TOKEN_PAREN_CLOSE;
    out_token->value.strValue.buffer = NULL;
    self->position++;
    return true;

L_OTHER:
    string_substring_shallow(&subview, &self->str, startPos, self->position - startPos);
    if(isNum) {
        out_token->tokenKind = TOKEN_NUMERIC;
        string_copy(&out_token->value.strValue, &subview);
    } else  if(isString) {
        out_token->tokenKind = TOKEN_STRING;
        string_substring_deep(&out_token->value.strValue, &subview, 1, subview.length - 2);
    } else {
        out_token->tokenKind = TOKEN_SYMBOL;
        string_copy(&out_token->value.strValue, &subview);
    }
    return true;
L_FAIL:
    self->position++;
    return false;
}