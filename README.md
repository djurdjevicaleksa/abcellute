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
* Scalable up to 26 columns and 999 rows if memory allows
* Columns are separated using `|` character

## TBA
* Expression parsing
* Better memory handling
* Copying one cell to another
* ...


## Planned expressions
TBA.

# Uses
This program uses [StringStruct](https://github.com/djurdjevicaleksa/ss) to handle string stuff.