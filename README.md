
# BoboScheme

BoboScheme is a Scheme interpreter written in C and Scheme. I was following Peter Michaux's [Scheme From Scratch](http://michaux.ca/articles/scheme-from-scratch-introduction) blog series but I'd like this to go further, implementing most of R5RS scheme. I eventually want to refactor this into as small of a C core as possible.

## Use
Running `make` and `bin/bbscheme` will drop you into the REPL.

The C core defines the syntax:
    begin, cond, define, if, lambda, let, quote, set!, and, or
In addition, there are implementation specific functions exposed beginning with `%bb-`.
Everything else is defined in src/stdlib.scm.

## TODO
Macros are the next milestone, which will replace a bit of the C-exposed sytnax.

## License
GNU Affero General Public License. See LICENSE file.
