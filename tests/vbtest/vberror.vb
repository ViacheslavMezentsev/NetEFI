Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Functions
Imports NetEFI.Runtime

Namespace VbTest

    <Computable("vberror", "n", "Throws a custom error specified by index n.")>
    Public Class VbError
        Inherits MathcadFunction(Of Complex, String)

        ' The list of custom error messages for this function.
        Public Shared Errors As String() = {
            "vberror: This is custom error message 1.",
            "vberror: This is custom error message 2.",
            "vberror: This is custom error message 3.",
            "vberror: This is custom error message 4."
        }

        Public Overrides Function Execute(n As Complex, context As Context) As String
            ' Throws a custom EFIException.
            ' The host will catch this and display the corresponding message from the Errors array.
            Throw New EFIException(CInt(n.Real), 1)
        End Function

    End Class

End Namespace