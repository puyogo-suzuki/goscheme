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

# Supported Features
 - [x] Garbage Collection(Reference Counting)
 - [x] Tail Call Optimizatioon
 - [] macro?
  ---
 - [x] define (partial, doesn't support`(define (a b c) (+ b c))`)
 - [x] quote
 - [x] lambda
 - [x] set!
 - [] let
 - [] let*
 - [] letrec
 - [x] if
 - [] cond
 - [x] begin
 - [] do
 - [x] number?
 - [] pair?
 - [] list?
 - [x] symbol?
 - [x] car
 - [x] cdr
 - [x] cons
 - [] length
 - [] memq
 - [] last
 - [] append
 - [] set-car!
 - [] set-cdr!
 - [x] string?
 - [] string-append
 - [] symbol->string
 - [] string->symbol
 - [] string->number
 - [] number->string
 - [x] procedure?(手抜き実装でScheme非互換)
 - [] eq?
 - [] neq?
 - [] equal?
 - [] and
 - [] or
 