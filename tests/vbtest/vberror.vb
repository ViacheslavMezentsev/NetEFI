Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Runtime


Public Class vberror
    Implements IComputable

    Public Shared Errors As String() = _
    { _
        "vberror: text example 1", _
        "vberror: text example 2", _
        "vberror: text example 3", _
        "vberror: text example 4" _
    }

    Public ReadOnly Property Info() As FunctionInfo Implements IComputable.Info

        Get
            Return New FunctionInfo("vberror", "n", "return error string",
                    GetType(String), New Type() {GetType(Complex)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IComputable.GetFunctionInfo
        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, context As Context) As Boolean _
        Implements IComputable.NumericEvaluation

        Throw New EFIException(CType(args(0), Complex).Real, 1)

    End Function

End Class
