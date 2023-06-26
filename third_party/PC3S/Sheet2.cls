VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Sheet2"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Option Explicit

Private Sub Worksheet_Activate()
        
        If bNeedReset And Not bStartup And Not bImage Then
            Call RedFilename
            Call ResetAll
        End If

End Sub

Private Sub Worksheet_Change(ByVal Target As Range)
    Dim KeyCells As Range

    'Don't execute this routine when loading a new file or running a scan macro
    If Not bEnableChangeHandlers Or bScan Then Exit Sub
    
    ' The variable KeyCells contains the cells that will
    ' cause an alert when they are changed.
    Set KeyCells = Range(plotChangeCells)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnPlotChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(resolutionCell & ", " & contactModeCell)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then Call ResetAll
    
End Sub


