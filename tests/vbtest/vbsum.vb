Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Functions

Namespace VbTest

    <Computable("vbsum", "a, b", "Calculates the complex sum of two scalars a and b.")>
    Public Class VbSum
        Inherits MathcadFunction(Of Complex, Complex, Complex)

        Public Overrides Function Execute(a As Complex, b As Complex, context As Context) As Complex
            Return a + b
        End Function

    End Class

End Namespace