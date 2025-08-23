Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Functions

Namespace VbTest

    <Computable("vbtest3", "n, m", "Returns an n x m matrix of complex numbers.")>
    Public Class VbTest3
        Inherits MathcadFunction(Of Complex, Complex, Complex(,))

        Public Overrides Function Execute(nComplex As Complex, mComplex As Complex, context As Context) As Complex(,)
            Try
                Dim n = CInt(nComplex.Real)
                Dim m = CInt(mComplex.Real)

                If n <= 0 OrElse m <= 0 Then
                    Return New Complex(,) {} ' Return empty matrix for invalid dimensions
                End If

                Dim matrix = New Complex(n - 1, m - 1) {}
                For r As Integer = 0 To n - 1
                    For c As Integer = 0 To m - 1
                        matrix(r, c) = New Complex(r, c)
                    Next
                Next
                Return matrix
            Catch ex As Exception
                context.LogError($"vbtest3 failed: {ex.Message}")
                Return New Complex(,) {} ' Return empty matrix on error
            End Try
        End Function

    End Class

End Namespace