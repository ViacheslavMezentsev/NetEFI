Imports System.IO
Imports NetEFI.Computables
Imports NetEFI.Design


Public Class vbrfile
    Implements IComputable

    Public ReadOnly Property Info() As FunctionInfo Implements IComputable.Info

        Get
            Return New FunctionInfo("vbrfile", "file", "return file content",
                GetType(String), New Type() {GetType(String)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IComputable.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, context As Context) As Boolean _
        Implements IComputable.NumericEvaluation

        Dim path = CStr(args(0))

        result = If(File.Exists(path), File.ReadAllText(path), "")

        Return True

    End Function

End Class
