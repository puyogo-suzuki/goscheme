# goscheme - Go's Scheme(subset) Implementation 
 *NOT* SCHEME IMPLEMENTATION BY **GOLANG**.

## requirements
 - OS : macOS, DragonFlyBSD, FreeBSD, Windows 11 (maybe Linux)
 - GNU make
 - Compiler : APPLE Clang or Clang or GCC or Visual C++ 2019

Below microcomputers could run.
 - ESP32(not included in this repository)

## How to build and execute
### Unix-like Systems
```sh
 $ make
 $ ./gs.out
```
*( Gauche's execulable is `gosh`)*

### Windows
Open goscheme.sln by Visual Studio, then Run.  
or 
```cmd
$ MSBuild.exe goscheme.sln 
$ .\bin\Debug\x64\goscheme.exe
```

# Using
```scheme
> (define a (lambda (l r) (+ l r)))
a
> (a 12 34)
46
```

# Example
## GC and Tail Call Optimization (may) work.
```scheme
> (define a (lambda (v) (b v)))
a
> (define b (lambda (v) (a v)))
b
> (a 32)
; ... inifinity loop
```
This can be run forever.  
It will stop if Garbage Collection doesn't works(Out of Memory) or Tail Call Optimization is incomplete(Stack Over Flow).

## Closure (may) work.
```scheme
> (define gen-counter (lambda (start) (define v start) (cons (lambda () (set! v (+ v 1))) (lambda () (set! v (- v 1))))))
gen-counter
> (define c0 (gen-counter 0))
c0
> (define c0up (car c0))
c0up
> (c0up)
1
> (c0up)
2
> (define c1 (gen-counter 0))
c1
> (define c1up (car c1))
c1up
> (c1up)
1
> (c0up)
3
```

# Supported Features
 - [x] Garbage Collection(Reference Counting)
 - [x] Tail Call Optimizatioon
 - [ ] macro?
 - [ ] Rational Number?
 ---
 - [x] quote
 - [ ] negative number
 - [ ] escape sequence
 - [ ] dot
 - [ ] comment
  ---
 - [x] define (partial, doesn't support`(define (a b c) (+ b c))`)
 - [x] quote
 - [x] lambda
 - [x] set!
 - [ ] let
 - [ ] let*
 - [ ] letrec
 - [x] if
 - [ ] cond
 - [x] begin
 - [ ] do
 - [x] number?
 - [x] +
 - [x] -
 - [x] *
 - [x] /
 - [ ] =
 - [ ] <
 - [ ] <=
 - [ ] >
 - [ ] >=
 - [ ] pair?
 - [ ] list?
 - [x] symbol?
 - [x] car
 - [x] cdr
 - [x] cons
 - [ ] length
 - [ ] memq
 - [ ] last
 - [ ] append
 - [ ] set-car!
 - [ ] set-cdr!
 - [x] string?
 - [ ] string-append
 - [ ] symbol->string
 - [ ] string->symbol
 - [ ] string->number
 - [ ] number->string
 - [x] procedure?(手抜き実装でScheme非互換)
 - [ ] eq?
 - [ ] neq?
 - [ ] equal?
 - [x] boolean?
 - [ ] not
 - [ ] and
 - [ ] or
 