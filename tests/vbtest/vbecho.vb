Imports NetEFI.Computables
Imports NetEFI.Design


Public Class vbecho
    Implements IComputable

    Public ReadOnly Property Info() As FunctionInfo Implements IComputable.Info

        Get
            Return New FunctionInfo("vbecho", "s", "return string",
                GetType(String), New Type() {GetType(String)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IComputable.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, context As Context) As Boolean _
        Implements IComputable.NumericEvaluation

        result = args(0)

        Return True

    End Function

End Class
