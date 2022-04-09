#include <stdio.h>
#include "token.h"
#include "io.h"
#include "parse.h"

int main(void) {
    string_t str;
    string_new_deep2(&str, "(hoge piyo 3283 \"  gehiog\" (a b))");
    tokenizer_t t = tokenizer_new(str);
    schemeObject_t * so;
    parse(&so, &t);

    printf("end\n");
    return 0;
}