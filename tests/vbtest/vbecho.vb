Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Functions

Namespace VbTest

    <Computable("vbecho", "s", "Returns the input string.")>
    Public Class VbEcho
        Inherits MathcadFunction(Of String, String)

        Public Overrides Function Execute(s As String, context As Context) As String
            Return s
        End Function

    End Class

End Namespace