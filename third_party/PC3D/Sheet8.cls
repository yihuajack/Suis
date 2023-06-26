VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
END
Attribute VB_Name = "Sheet8"
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
    
    Set KeyCells = Range(enableTemperatureCell & ", " & modelTempNiCells & ", " & modelTempOpticsCells & ", " & modelTempExponentCell _
                            & ", " & enableDopingCell & ", " & modelDopingBulkCells & ", " & modelDopingSurfaceCells)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Call MapInputParameters
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
    Set KeyCells = Range(modelJLCell & ", " & modelJo1Cell & ", " & modelJo2Cell & ", " & modelGshCell & ", " & modelRsCell)
    
    If Not Application.Intersect(KeyCells, Range(Target.Address)) Is Nothing Then
        bEnableChangeHandlers = False
        Application.Calculation = xlCalculationManual 'Necessary for Mac compatibility
        Range(modelPmaxCell).Value = TwoDiodePmax(Range(modelJLCell), 0.000000000000001 * Range(modelJo1Cell), 0.000000001 * Range(modelJo2Cell), 0.001 * Range(modelGshCell), Range(modelRsCell))
        Range(modelVocCell).Value = TwoDiodeVoc(Range(modelJLCell), 0.000000000000001 * Range(modelJo1Cell), 0.000000001 * Range(modelJo2Cell), 0.001 * Range(modelGshCell))
        Range(modelRMSCell).Value = ""
        Application.Calculation = xlCalculationAutomatic
        bEnableChangeHandlers = True
    End If
    
End Sub
