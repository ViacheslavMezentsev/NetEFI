Imports NetEFI.Functions
Imports NetEFI.Runtime

Namespace VbTest

    <Computable("vbecho", "s", "Returns the input string.")>
    Public Class VbEcho
        Inherits CustomFunction(Of String, String)

        Public Overrides Function Execute(s As String, context As Context) As String
            Return s
        End Function

    End Class

End Namespace