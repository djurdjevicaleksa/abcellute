# Abcellute, the abcellutely worst Excel clone

This is MS Excel implemented as a batch program using a menu loop. 
# Usage
Using `./run.sh` will build and run the program.

```
$    chmod +x run.sh
$    ./run.sh
```
# Main functions

## Import
Imports a table from a file specified during dialogue. Table is written by following these rules:
* Columns are separated using `|` character
* Rows are separated by a new line
* Expressions start with `=` character. Cell references must be formatted like `A8` or `Z12`, letter and numbers cannot be separated.
* Colours occupy an entire cell and start with `#` character. Supported colours are `WHITE`, `BLACK`, `RED`, `GREEN`, `YELLOW`, `BLUE`, `MAGENTA` and `CYAN`. Noting else should be in the cell other than the colour specifier.

* Numbers can be positive or negative, integers or floating point
* Text is considered anything that doesn't fall into categories mentioned prior
* Example given in `input.txt`
  
## Create
Creates an empty table of size specified during dialogue. Table can be up to 26 columns wide and 999 rows long.

## Modify
Modifies existing cell. Cannot modify cells that are outside the table, meaning if the table has 10 rows, you cannot modify a cell from row 11.

## Export
Exports the table to a file specified during dialogue. Colours are represented as a series of `#` characters.

## Solve
Checks for syntax and lexical errors, then checks for dependency cycles and invalid dependencies and then solves expressions if no error was found.

# Quirks:
* Uses a menu loop
* Neat way to format and print the table back to the user
* Automatically resizes to accommodate up to 26 columns and 999 rows
* Thorough expression validity analysis
* Dependency cycle and invalid dependency detection
* Expression solving
* Thorough and precise error logging
* Colouring in the cells

# Expression details
Every expression must start with a `=` sign. Other than that, you can do basically anyting involving numbers and cell references(`A0`, `J17`...).
Valid cell references are `A10`, `J57`, `B999`. Invalid cell references look like `A 10`, 
`J   5  7`, `B1000`.
Brackets are not implemented. Allowed operations are `+`, `-`, `*`, `/` and `^`.
 Supports negative numbers. (`A8 * - A6`).


# TBA
* Arena for changes in the table, because all imported data is owned by a big allocated buffer
* Parentheses in expressions
* ...

# Uses
This program uses [StringStruct](https://github.com/djurdjevicaleksa/ss) to handle string stuff, although being slightly modified to fit the job.