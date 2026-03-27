namespace FSharpExamples

open System.Numerics
open NetEFI.Functions
open NetEFI.Runtime

// Example 1: A simple sum function
[<Computable("fs_sum", "a, b", "Calculates the sum of two complex numbers")>]
type FsSumFunction() =
    inherit CustomFunction<Complex, Complex, Complex>()
    override this.Execute(a, b, context) =
        a + b

// Example 2: A function that throws a custom error
[<Error("NegativeInput", "Input cannot be negative.")>]
[<Computable("fs_sqrt", "x", "Calculates the square root of a non-negative number")>]
type FsSqrtFunction() =
    inherit CustomFunction<Complex, Complex>()
    override this.Execute(x, context) =
        if x.Real < 0.0 && x.Imaginary = 0.0 then
            raise (EFIException("NegativeInput", 1))
        else
            Complex.Sqrt(x)