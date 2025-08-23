Imports System.IO
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Functions

Namespace VbTest

    <Computable("vbrfile", "filePath", "Returns the content of a specified text file.")>
    Public Class VbReadFile
        Inherits MathcadFunction(Of String, String)

        Public Overrides Function Execute(filePath As String, context As Context) As String
            Try
                If File.Exists(filePath) Then
                    Return File.ReadAllText(filePath)
                Else
                    Return $"ERROR: File not found at '{filePath}'"
                End If
            Catch ex As Exception
                context.LogError($"Failed to read file '{filePath}': {ex.Message}")
                Return $"ERROR: {ex.Message}"
            End Try
        End Function

    End Class

End Namespace