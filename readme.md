A basic command line calculator mainly written as a test of the "two stack" infix parsing technique that was apparently used in the visicalc spreadsheet. This uses a value stack (that numbers are pushed to) and an op stack (that operators are pushed to), and every time you push to the op stack, if hte precedence of the operator on the top of the stack is higher then that of the operator you're pushing you evaluate the stack untill its not true.

Currently supports + - * / % and ^  but not brackets.
