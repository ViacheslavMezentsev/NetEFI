Imports System.Numerics
Imports NetEFI

Public Class vbtest3
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo( "vbtest3", "n, m", "return matrix n x m", _
                GetType( Complex(,) ), New Type() { GetType( Complex ), GetType( Complex ) } )
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        Dim mat As Complex(,) = Nothing

        result = mat

        Try

            Dim n = CInt( CType( args(0), Complex).Real )
            Dim m = CInt( CType( args(1), Complex).Real )

            mat = New Complex(n - 1, m - 1) {}

            For r As Integer = 0 To n - 1
                For c As Integer = 0 To m - 1
                    mat(r, c) = New Complex(r, c)
                Next
            Next

            result = mat

        Catch ex As Exception

            Return False

        End Try

        Return True

    End Function

End Class
