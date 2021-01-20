Imports System.Numerics
Imports NetEFI

Public Class vbtest3
    Implements IFunction

    Private _info As FunctionInfo

    Public ReadOnly Property Info() As FunctionInfo _
        Implements IFunction.Info

        Get
            Return _info
        End Get
    End Property

    Public Sub New()

        _info = New FunctionInfo( "vbtest3", "n, m", "return matrix n x m", _
            GetType( Complex(,) ), _
            New Type() { GetType( Complex ), GetType( Complex ) } )
    End Sub

    Public Function GetFunctionInfo(lang As String) As FunctionInfo _
        Implements IFunction.GetFunctionInfo

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
