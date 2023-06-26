VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Sheet7"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Option Explicit

Private Sub Worksheet_Change(ByVal Target As Range)
    Dim KeyCells As Range

    'Don't execute this routine when loading a new file!
    If Not bEnableChangeHandlers Then Exit Sub
    
    ' The variable KeyCells contains the cells that will
    ' cause an alert when they are changed.
    Set KeyCells = Range(opticalFilterCell)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnFilterChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(illuminationValueCells)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnIlluminationChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(illuminationShapeCells)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnIlluminationChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(illuminationDropdownCells)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnIlluminationChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(spectrumDataCells)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then bNeedReset = True
    
End Sub



