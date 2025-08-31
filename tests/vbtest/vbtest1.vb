Imports System.Numerics
Imports NetEFI.Functions
Imports NetEFI.Runtime

Namespace VbTest

    <Computable("vbtest1", "x", "Returns the complex scalar 2 * x.")>
    Public Class VbTest1
        Inherits CustomFunction(Of Complex, Complex)

        Public Overrides Function Execute(x As Complex, context As Context) As Complex
            Return 2 * x
        End Function

    End Class

End Namespace