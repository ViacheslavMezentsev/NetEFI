Imports System.Numerics
Imports NetEFI.Functions
Imports NetEFI.Runtime

Namespace VbTest

    <Computable("vbtest2", "separator, vector", "Joins the elements of a vector with a separator.")>
    Public Class VbTest2
        Inherits CustomFunction(Of String, Complex(,), String)

        Public Overrides Function Execute(separator As String, vector As Complex(,), context As Context) As String
            Try
                Dim elements = New List(Of String)()
                For i As Integer = 0 To vector.GetLength(0) - 1
                    ' Assuming a column vector
                    elements.Add(vector(i, 0).ToString())
                Next
                Return String.Join(separator, elements)
            Catch ex As Exception
                context.LogError($"vbtest2 failed: {ex.Message}")
                Return $"ERROR: {ex.Message}"
            End Try
        End Function

    End Class

End Namespace