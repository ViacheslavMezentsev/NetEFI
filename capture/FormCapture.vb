Imports System.Drawing
Imports System.Windows.Forms
Imports System.Drawing.Imaging


Public Class FormCapture

    Declare Function SendMessage Lib "user32" Alias "SendMessageA" (ByVal hwnd As Integer, ByVal wMsg As Integer, ByVal wParam As Integer, ByVal lParam As Object) As Integer
    Declare Function SetWindowPos Lib "user32" Alias "SetWindowPos" (ByVal hwnd As Integer, ByVal hWndInsertAfter As Integer, ByVal x As Integer, ByVal y As Integer, ByVal cx As Integer, ByVal cy As Integer, ByVal wFlags As Integer) As Integer
    Declare Function DestroyWindow Lib "user32" (ByVal hndw As Integer) As Boolean
    Declare Function capCreateCaptureWindowA Lib "avicap32.dll" (ByVal lpszWindowName As String, ByVal dwStyle As Integer, ByVal x As Integer, ByVal y As Integer, ByVal nWidth As Integer, ByVal nHeight As Short, ByVal hWndParent As Integer, ByVal nID As Integer) As Integer
    Declare Function capGetDriverDescriptionA Lib "avicap32.dll" (ByVal wDriver As Short, ByVal lpszName As String, ByVal cbName As Integer, ByVal lpszVer As String, ByVal cbVer As Integer) As Boolean

    Const WM_CAP As Short = &H400S
    Const WM_CAP_DRIVER_CONNECT As Integer = WM_CAP + 10
    Const WM_CAP_DRIVER_DISCONNECT As Integer = WM_CAP + 11
    Const WM_CAP_EDIT_COPY As Integer = WM_CAP + 30
    Const WM_CAP_SET_PREVIEW As Integer = WM_CAP + 50
    Const WM_CAP_SET_PREVIEWRATE As Integer = WM_CAP + 52
    Const WM_CAP_SET_SCALE As Integer = WM_CAP + 53
    Const WS_CHILD As Integer = &H40000000
    Const WS_VISIBLE As Integer = &H10000000
    Const SWP_NOMOVE As Short = &H2S
    Const SWP_NOSIZE As Short = 1
    Const SWP_NOZORDER As Short = &H4S
    Const HWND_BOTTOM As Short = 1

    Dim iDevice As Integer = 0
    Dim hHwnd As Integer
    Dim fun As Capture.capture


    Public Sub New(id As Integer, ByRef cap As Capture.capture)

        iDevice = id
        fun = cap

        InitializeComponent()

    End Sub

    Private Sub LoadDeviceList()

        Dim bReturn As Boolean
        Dim x As Integer = 0

        Dim strName As String = Space(100)
        Dim strVer As String = Space(100)

        Try

            Do
                bReturn = capGetDriverDescriptionA(x, strName, 100, strVer, 100)

                If bReturn Then lstDevices.Items.Add(strName.Trim)

                x += 1

            Loop Until bReturn = False

            If lstDevices.Items.Count > 0 Then lstDevices.SelectedIndex = 0

        Catch ex As Exception

            StatusLabel1.Text = ex.Message

        End Try

    End Sub

    Private Sub OpenPreviewWindow()

        Dim iHeight As Integer = picCapture.ClientSize.Height
        Dim iWidth As Integer = picCapture.ClientSize.Width

        Try

            hHwnd = capCreateCaptureWindowA(iDevice, WS_VISIBLE Or WS_CHILD, 0, 0, iWidth, iHeight, picCapture.Handle.ToInt32, 0)

            If SendMessage(hHwnd, WM_CAP_DRIVER_CONNECT, iDevice, 0) Then

                SendMessage(hHwnd, WM_CAP_SET_SCALE, True, 0)
                SendMessage(hHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0)
                SendMessage(hHwnd, WM_CAP_SET_PREVIEW, True, 0)

                SetWindowPos(hHwnd, HWND_BOTTOM, 0, 0, picCapture.ClientSize.Width, picCapture.ClientSize.Height, SWP_NOMOVE Or SWP_NOZORDER)

                ButtonStop.Enabled = True
                ButtonStart.Enabled = False

            Else

                DestroyWindow(hHwnd)

            End If

        Catch ex As Exception

            StatusLabel1.Text = ex.Message

        End Try

    End Sub

    Private Sub ClosePreviewWindow()

        SendMessage(hHwnd, WM_CAP_DRIVER_DISCONNECT, iDevice, 0)
        DestroyWindow(hHwnd)

    End Sub

    Public Sub ResizeVideoWindow()

        'Resize the video preview window to match owner window size
        'left , top , width , height
        If Not hHwnd = 0 Then

            SetWindowPos(hHwnd, HWND_BOTTOM, 0, 0, picCapture.ClientSize.Width, picCapture.ClientSize.Height, SWP_NOMOVE Or SWP_NOZORDER)

        End If

    End Sub

    Private Sub GetSnapShot()

        Try

            Dim s As Size = picCapture.ClientSize

            fun.img = New Bitmap(s.Width, s.Height, PixelFormat.Format24bppRgb)

            Dim memoryGraphics As Graphics = Graphics.FromImage(fun.img)

            Dim sp = picCapture.PointToScreen(picCapture.ClientRectangle.Location)

            memoryGraphics.CopyFromScreen(sp.X, sp.Y, 0, 0, s)

            picCapture.Image = fun.img

        Catch ex As Exception

            StatusLabel1.Text = ex.Message
        End Try

    End Sub

    Private Sub ButtonStop_Click( sender As Object, e As EventArgs ) Handles ButtonStop.Click

        GetSnapShot()

        ClosePreviewWindow()

        ButtonStop.Enabled = False
        ButtonStart.Enabled = True

    End Sub

    Private Sub ButtonReturn_Click( sender As Object, e As EventArgs ) Handles ButtonReturn.Click

        DialogResult = DialogResult.OK
        Close()

    End Sub

    Private Sub Form1_FormClosed( sender As Object, e As FormClosedEventArgs ) Handles MyBase.FormClosed

        ClosePreviewWindow()

    End Sub


    Private Sub Form1_Resize( sender As Object, e As EventArgs ) Handles MyBase.Resize

        ResizeVideoWindow()

    End Sub

    Private Sub Form1_Load( sender As Object, e As EventArgs ) Handles MyBase.Load

        LoadDeviceList()
        ResizeVideoWindow()

    End Sub

    Private Sub ButtonStart_Click( sender As Object, e As EventArgs ) Handles ButtonStart.Click

        StatusLabel1.Text = "Ready"

        OpenPreviewWindow()

        ButtonStart.Enabled = False
        ButtonStop.Enabled = True

    End Sub

End Class
