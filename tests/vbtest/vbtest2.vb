Imports NetEFI

Public Class vbtest2

    Implements IFunction

    Private _info As FunctionInfo

    Public ReadOnly Property Info() As FunctionInfo _
        Implements IFunction.Info

        Get
            Return _info
        End Get
    End Property

    Public Sub New()

        _info = New FunctionInfo("vbtest2", "separ, v", "return string: v[0] separ v[1] separ ...", _
            GetType(String), _
            New Type() {GetType(String), GetType(TComplex(,))})
    End Sub

    Public Function GetFunctionInfo(lang As String) As FunctionInfo _
        Implements IFunction.GetFunctionInfo

        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        Try

            Dim d = CType(args(0), String)
            Dim v = CType(args(1), TComplex(,))

            Dim len As Integer = v.GetLength(0)

            Dim list = New List(Of String)()

            For n As Integer = 0 To len - 1
                list.Add(String.Format("{0} + {1} * i", v(n, 0).Real, v(n, 0).Imaginary))
            Next

            result = String.Join(d, list.ToArray())

        Catch ex As Exception

            Return False

        End Try

        Return True

    End Function

End Class
