Imports System
Imports System.Numerics
Imports NetEFI.Functions
Imports NetEFI.Runtime

Namespace VbTest

    <[Error]("Error1", "vberror: This is custom error message 1.")>
    <[Error]("Error2", "vberror: This is custom error message 2.")>
    <[Error]("Error3", "vberror: This is custom error message 3.")>
    <[Error]("Error4", "vberror: This is custom error message 4.")>
    <Computable("vberror", "n", "Throws a custom error specified by its index n.")>
    Public Class VbError
        Inherits CustomFunction(Of Complex, String)

        Public Overrides Function Execute(n As Complex, context As Context) As String
            Dim errorIndex As Integer = CInt(n.Real)

            Dim errorKey As String

            Select Case errorIndex
                Case 1
                    errorKey = "Error1"
                Case 2
                    errorKey = "Error2"
                Case 3
                    errorKey = "Error3"
                Case 4
                    errorKey = "Error4"
                Case Else
                    Throw New ArgumentOutOfRangeException(
                        NameOf(n),
                        $"The provided error index '{errorIndex}' is not valid. It must be between 1 and 4."
                    )
            End Select

            Throw New EFIException(errorKey, 1)
        End Function

    End Class

End Namespace