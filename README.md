# LogicUnit
Read instructions via command line and get the result of the it

# Requirement
UNITS: it can have serval UNITs, for each Unit, the format below:
        a := max
        a is the unit name, max is the operator, it can support sum,max,min,negate,mul
INPUTS: specify the inputs count
CONNECTIONS: it specify the connections between user input and unit input, be aware that the output of one unit also can be as input of another unit, the format of connections below:
        input/0 -- a/in/0
        user input0 connect to input0 of unit a
        a/out/0 -- b/in/0
        output of unit a is connected to input0 of unit b
        c/out/0 -- result
        output of unit c is the final result
VALUES: specify the value of inputs, should do calculation for each line, if can't get the result then no output
        input/0 := 3
        specify the user input0 is 3

# Input sample
UNITS: 3
a := max
b := sum
c := negate
INPUTS: 3
CONNECTIONS: 6
input/0 -- a/in/0
input/1 -- a/in/1
a/out/0 -- b/in/0
input/2 -- b/in/1
b/out/0 -- c/in/0
c/out/0 -- result
VALUES: 5
input/0 := 3
input/1 := 5
input/2 := 7
input/2 := 9
input/0 := 9

# Output sample
-12
-14
-18
