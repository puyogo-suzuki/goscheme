#include <stdio.h>
#include "token.h"
#include "io.h"
#include "parse.h"
#include "machine.h"
#include "gc.h"

int main(void) {
    machine_t vm;
    machine_new(&vm);
    while (1) {
        char buf[1024];
        fgets(buf, 1023, stdin);
        string_t str;
        string_new_shallow2(&str,  buf);
        tokenizer_t t = tokenizer_new(str);
        schemeObject_t * so, * res;
        if (parse(&so, &t)) printf("parse fail\n");
        else {
            if (machine_eval(&vm, &(vm.env), &res, so)) printf("eval fail\n");
            else {
                schemeObject_toString(&str, res);
                string_writeLine(stdout, &str);
                string_free(&str);
                gc_deref_schemeObject(res);
            }
            gc_deref_schemeObject(so);
        }
    }
    printf("end\n");
    return 0;
}