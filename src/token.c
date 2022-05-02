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

size_t
count_length_for_string_literal(string_t * str) {
    size_t ret = 0;
    bool prev = false;
    char ch = 0;
    for(int i = 0; string_getAt(str, i, &ch) == ERR_SUCCESS; ++i) {
        if(prev == true) prev = false;
        else {
            ret++;
            if(ch == '\\') prev = true;
        }
    }
    return ret;
}

bool
tokenizer_next(tokenizer_t * self, token_t * out_token) {
    size_t startPos;
    string_t subview;
    char cur = '\0';
    char prevcur = '\0';
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
                out_token->tokenKind = TOKEN_QUOTE;
                goto L_PAREN;
            case ';':
                while(true) {
                    if(string_getAt(&self->str, self->position, &cur) != ERR_SUCCESS)
                        return false;
                    if(cur == '\n') goto L_OUTER_WHILE_CONTINUE;
                }
            default: goto L_BREAK_WHILE;
        }
        L_OUTER_WHILE_CONTINUE:
        self->position++;
    } L_BREAK_WHILE:


    startPos = self->position;
    bool isString = cur == '"';
    bool firstNegate = cur == '-';
    bool isNum = firstNegate|| isNumChar(cur);
    self->position++;

    while(string_getAt(&self->str, self->position, &cur) == ERR_SUCCESS) {
        L_RETRY:
        switch(cur) {
            case '\n': goto L_OTHER;
            case ' ': case '(': case ')': if (!isString) goto L_OTHER; else break;
            case '"': if (prevcur != '\\') { if (isString) goto L_STRING; else goto L_FAIL; } break;
            default:
                if(isNum && !isNumChar(cur)) {
                    if(firstNegate) {
                        isNum = false;
                        goto L_RETRY;
                    } else goto L_FAIL;
                } else break;
        }
        self->position++;
        prevcur = cur;
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
    string_t tmp;
    CHKERROR(string_substring_shallow(&tmp, &self->str, startPos + 1, self->position - startPos - 2))
    size_t outlength = count_length_for_string_literal(&tmp);
    CHKERROR(string_new(&out_token->value.strValue, outlength));
    bool prevEsc = false;
    for(size_t i = 0, j = 0; string_getAt(&tmp, i, &cur) == ERR_SUCCESS; ++i) {
        if(prevEsc) {
            switch(cur) {
                case 'n': out_token->value.strValue.buffer[j] = '\n'; break;
                case '"': out_token->value.strValue.buffer[j] = '"'; break;
                case '\\': out_token->value.strValue.buffer[j] = '\\'; break;
                default: out_token->value.strValue.buffer[j] = cur; break;
            }
            prevEsc = false;
            j++;
        } else {
            if(cur == '\\') {
                prevEsc = true;
                continue;
            }
            out_token->value.strValue.buffer[j] = cur;
            j++;
        }
    }
    return true;

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

gserror_t
token_toString(token_t * self, string_t * out) {
    char str_str[] = "TOKEN_STRING(";
    char str_sym[] = "TOKEN_SYMBOL(";
    int str_first_size = self->tokenKind == TOKEN_STRING ? sizeof(str_str) - 1 : sizeof(str_sym) - 1;
    char * str_first = self->tokenKind == TOKEN_STRING ? str_str : str_sym;
    switch (self->tokenKind) {
    case TOKEN_PAREN_OPEN: return string_new_deep2(out, "TOKEN_PAREN_OPEN");
    case TOKEN_QUOTE: return string_new_deep2(out, "TOKEN_QUOTE");
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
