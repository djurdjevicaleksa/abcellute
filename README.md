# Abcellute, the abcellutely worst Excel clone

# Usage
Using `./run.sh` will build and run the program.

```
$    chmod +x run.sh
$    ./run.sh
```

Optionally, change input file name inside run.sh:

```
$   ./main <input.csv>
```

## Supports:
* Input file as argument to program call
* Numbers, text, expressions
* Neat way to format and print the table back to the user
* Automatically resizes to accommodate up to 26 columns and 999 rows if memory allows
* Columns are separated using `|` character
* Thorough expression validity analysis
* Dependency cycle and invalid dependency detection
* Expression solving
* Thorough and precise error logging
* Colouring in the cells

## TBA
* Parentheses in expressions
* ...


## Allowed expressions
Every expression must start with a `=` sign. Other than that, you can do basically anyting involving numbers and cell references(A0, J17...).
Brackets are not implemented. Allowed operations are +, -, *, / and ^. Supports negative numbers.

## Colours
To colour in a cell, type `#` and then uppercase name of the colour. Supported colours are WHITE, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA and CYAN.

# Uses
This program uses [StringStruct](https://github.com/djurdjevicaleksa/ss) to handle string stuff, although being slightly modified to do the job.