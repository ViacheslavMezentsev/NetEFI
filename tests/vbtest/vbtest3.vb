Imports System.Reflection
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

        _info = New FunctionInfo("vbtest3", "n, m", "return matrix n, m", _
            GetType(TComplex(,)), _
            New Type() {GetType(TComplex), GetType(TComplex)})
    End Sub

    Public Function GetFunctionInfo(ByVal lang As String) As FunctionInfo _
        Implements IFunction.GetFunctionInfo

        Return Info
    End Function

    Public Function NumericEvaluation(ByVal args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        Dim mat As TComplex(,) = Nothing

        result = mat

        Try

            Dim n = CInt(CType(args(0), TComplex).Real)
            Dim m = CInt(CType(args(1), TComplex).Real)

            mat = New TComplex(n - 1, m - 1) {}

            For r As Integer = 0 To n - 1
                For c As Integer = 0 To m - 1
                    mat(r, c) = New TComplex(r, c)
                Next
            Next

            result = mat

        Catch ex As Exception

            Return False

        End Try

        Return True

    End Function

End Class
