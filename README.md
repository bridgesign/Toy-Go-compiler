# Toy Go Compiler

This is a toy Go compiler. The focus is on making a compiler as it is a part of the CS327: Compilers Course. This is a single pass compiler with focus on register allocation. .

## Requirements
1. flex 2.6.4
2. GNU Bison 3.5.1
3. gcc version 9.3.0
4. GNU Make 4.2.1
5. MARS 4.5

Note: The requirements are not hard, but only inform the versions on which compilation is tested.

## Compiling
For compiling, do a make.

```
make
```

To avoid file cleaning, use

```
make build
```

For cleaning after build, use

```
make clean
```

## Using the Compiler

```
./go<input_file

```

The assembly output will be written to asmb.asm

For running the code, use MARS to load the assembly and run it.

# The Language

## Variable Declarartion
The current support is only for integer calculation so any variable declaration is integer.

A variable when declared needs to be allocated the memory which is done in the following manner.

```
a:=1
```

The above statement declares the variable "a" with a memory allocation. On the other hand, the statetment below reassigns "a"

```
a=2
```

## Expressions and Printing
A variable cannot be reassigned with allocating memory and the variable can only be allocated memory only once.

The expressions can be nested as show below.

```
a = ((c+b)*3 + a)*4
```

The statements can end with a semicolon or a new line. The below statement will print the final value as output.

```
print a*3
```

## If and While
There is support for "if" statement.

```
if (condition) statement|block
if (condition) statement|block else block
if (condition) statement|block else IfStatement
```
The condition is of the form Exp Relop Exp but both expressions cannot be constants. Similar format for "while" loop. Also, these can be nested.

## Blocks and Scopes

There can exist variables with same name but with separate memory if they are in different blocks.

```
a:=1
{
a:=2
print a
}
print a
```

The ouput of this will be 2 and then 1. A block has its own scope.

```
a:=1
{
print a
a:=2
print a
}
print a
```

The ouput here will be 1,2,1. This is because "a" was defined in previous scope which is accessible and then new memory is allocated to "a" and thus prints 2. But then after the end of block, the "a" declared in the block will go out of scope and the initial declared will come in scope. Hence, it will print 1 again.

# Example Tests
Examples are provided in the tests folder.
