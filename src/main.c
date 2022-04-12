#include <stdio.h>
#include "token.h"
#include "io.h"
#include "parse.h"

int main(void) {
    string_t str;
    string_new_deep2(&str, "(hoge piyo 3283 \"  gehiog\" (a b) nil)");
    tokenizer_t t = tokenizer_new(str);
    schemeObject_t * so;
    if(parse(&so, &t)) printf("parse fail\n");
    string_t outstr;
    if(schemeObject_toString(&outstr, so)) printf("toString fail\n");
    string_writeLine(stdout, &outstr);
    printf("end\n");
    return 0;
}