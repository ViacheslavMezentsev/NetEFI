Imports System.Numerics
Imports NetEFI.Functions
Imports NetEFI.Runtime

Namespace VbTest

    <Computable("vbsum", "a, b", "Calculates the complex sum of two scalars a and b.")>
    Public Class VbSum
        Inherits CustomFunction(Of Complex, Complex, Complex)

        Public Overrides Function Execute(a As Complex, b As Complex, context As Context) As Complex
            Return a + b
        End Function

    End Class

End Namespace