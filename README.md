# ss

~A radically Simple Shell.~ 
A Super Simple repl.
Please don't be offended by its name, it was inspired by elon musk.

## syntax

Heavily inspired by [haskell](https://www.haskell.org/) and [nix](https://nixos.org/).
See `./src/flexer/parser.h` for EBNF grammar definition. 

## examples

Run `make && ./ss` or `nix run` to start the repl.
Run `make && ./ss -f filename.nlys` or `nix run . -- -f filename.nlys` to evaluate an ss script.

```
# if statement

if 5 > 2 then "truthy" else "falsy"

# let in block (evaluates to 98)

let 
  # function definition with parameters x and y
  fun x y = (y + x) * 7;
  # variable declaration
  var = 9;
in 
  # function call
  fun var 5

# quit repl

:q
```
