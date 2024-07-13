# MS Excel batch program

# Usage

```
$    chmod +x run.sh
$    ./run.sh
```

Optionally, change input file name inside run.sh:

```
./main <input.csv>
```

## Supports:
* Input file as argument to program call
* Numbers, text, expressions
* Neat way to format and print the table back to the user
* Automatically resizes to accommodate up to 26 columns and 999 rows if memory allows
* Columns are separated using `|` character
* Syntax analysis for expressions
* Lexical analysis for expressions
* Dependency cycle detection
* Invalid dependency detection
* Thorough and precise error logging

## TBA
* Expression solving using shunting yard algorithm
* Colouring in the cells
* ...


## Allowed expressions
* ...

# Uses
This program uses [StringStruct](https://github.com/djurdjevicaleksa/ss) to handle string stuff.