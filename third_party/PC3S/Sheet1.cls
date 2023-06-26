VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Sheet1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = True
Option Explicit

Private Sub Worksheet_Change(ByVal Target As Range)
    Dim KeyCells As Range

    'Don't execute this routine when loading a new file or running a scan macro
    If Not bEnableChangeHandlers Or bScan Then Exit Sub
    
    Set KeyCells = Range(temperatureCell)       'Requires recalculation of values shown on this sheet
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bStartup = False
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnTemperatureChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(dopingChangeCells)   'Requires recalculation of values shown on this sheet
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bStartup = False
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnDopingChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(textureChangeCells)      'Changes dimensions displayed on this sheet, and triggers need to recalc diffusion
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bStartup = False
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnTextureChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(frontSshapeCell & ", " & frontChargeShapeCell & ", " & diffusionShapeCell)      'Changes background color of valley value
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bStartup = False
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call OnShapeChange
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(defectValueCells & ", " & defectDropdownCells)      'May change dimensionality of solution
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bStartup = False
        bNeedReset = True
    End If
    
    Set KeyCells = Range(diffusionChangeCells & ", " & BGNCells)     'Triggers need to recalc diffusion
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bStartup = False
        bNeedReset = True
    End If
End Sub

