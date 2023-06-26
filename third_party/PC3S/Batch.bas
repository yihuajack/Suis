Attribute VB_Name = "Batch"
Option Explicit

Sub BatchClearHeaders()
If ActiveSheet.Name <> "Batch" Then Exit Sub
Range(batchOutputNameCells).ClearContents
Range(batchOutputDataCells).ClearContents
End Sub

Sub BatchClearData()
If ActiveSheet.Name <> "Batch" Then Exit Sub
Range(batchOutputDataCells).ClearContents
End Sub

Sub BatchClearAll()
If ActiveSheet.Name <> "Batch" Then Exit Sub
Range(batchInputNameCell).ClearContents
Range(batchOutputNameCells).ClearContents
Range(batchInputDataCells).ClearContents
Range(batchOutputDataCells).ClearContents
Range(batchDescriptionCell).ClearContents
Range(batchStartCell).Value = 0
Range(batchEndCell).Value = 1
Range(batchIntervalsCell).Value = 1
Range(batchScaleCell).Value = "Linear"
End Sub

Sub BatchDefaultSolve()
If ActiveSheet.Name <> "Batch" Then Exit Sub
Call BatchClearHeaders
Range(batchOutputNameCells).Cells(1, 1).Value = ZharmonicsCell
Range(batchOutputNameCells).Cells(1, 2).Value = errorCell
Range(batchOutputNameCells).Cells(1, 3).Value = iterationCell
Range(batchOutputNameCells).Cells(1, 4).Value = solutionTimeCell
Range(batchOutputNameCells).Cells(1, 5).Value = voltageCell
Range(batchOutputNameCells).Cells(1, 6).Value = generationCell
Range(batchOutputNameCells).Cells(1, 7).Value = bulkRecombinationCell
Range(batchOutputNameCells).Cells(1, 8).Value = surfaceRecombinationCell
Range(batchOutputNameCells).Cells(1, 9).Value = totalRecombinationCell
End Sub

Sub BatchAutoFill()
Dim m As Integer, mMax As Integer, nSign As Integer
Dim inputValue As Double, startValue As Double, endValue As Double
If ActiveSheet.Name <> "Batch" Then Exit Sub
Range(batchInputDataCells).ClearContents
mMax = Range(batchIntervalsCell) + 1
If mMax < 2 Or mMax > batchMaxIntervals + 1 Then
    MsgBox ("The number of intervals must be 1 to " & batchMaxIntervals)
    Exit Sub
End If
If Range(batchStartCell) = "" Or Range(batchEndCell) = "" Then
    MsgBox ("Please enter start and end values")
    Exit Sub
End If
startValue = Range(batchStartCell)
endValue = Range(batchEndCell)
If startValue * endValue <= 0 And Range(batchScaleCell) = "Log" Then
    MsgBox ("Start/end values must be compatible with Log scaling")
    Exit Sub
End If
If Range(batchScaleCell) = "Log" Then
    nSign = Sgn(startValue)
    startValue = Abs(startValue)
    endValue = Abs(endValue)
End If
For m = 1 To mMax
    If Range(batchScaleCell) = "Log" Then
        inputValue = nSign * ExpPower(Log(startValue) + (m - 1) * (Log(endValue) - Log(startValue)) / (mMax - 1))
    Else        'Linear
        inputValue = startValue + (m - 1) * (endValue - startValue) / (mMax - 1)
    End If
    Range(batchInputDataCells).Cells(m, 1).Value = inputValue
Next m
End Sub

Sub BatchOFAT()     'as in One Factor At'a Time
Dim inputCell As String
Dim outputCells(1 To batchMaxOutputValues) As String
Dim m As Integer, n As Integer  'm is used for input values, n is used for output values
Dim saveInput As Variant, fileNameColor As Variant
Dim saveVoltage As Double
Dim saveSpectral As Variant, saveTauN As Variant, saveTauP As Variant, saveIntrinsic As Variant
Dim bOutputSpecified As Boolean
If ActiveSheet.Name <> "Batch" Then Exit Sub

fileNameColor = Range(loadedFileCell).Cells.Font.Color  'Save color to be restored after batch completion
inputCell = Range(batchInputNameCell)
If inputCell = "" Then
    MsgBox ("You must specify the Variable Cell location (e.g. Device!T15)")
    Exit Sub
End If
If Not ValidInputField(inputCell) Then
    MsgBox ("The Variable Cell supplied is not on the list of valid user-input cells")
    Exit Sub
End If

bOutputSpecified = False
For n = 1 To batchMaxOutputValues
    If Range(batchOutputNameCells).Cells(1, n) <> "" Then
        outputCells(n) = Range(batchOutputNameCells).Cells(1, n)
        bOutputSpecified = True
    Else
        outputCells(n) = ""
    End If
Next n
If Not bOutputSpecified Then
    MsgBox ("You must specify at least one Output Cell location (e.g. Dashboard!G5)")
    Exit Sub
End If

saveInput = Range(inputCell).Formula
Range(batchOutputDataCells).ClearContents

For m = 1 To batchMaxIntervals + 1
    If Range(batchInputDataCells).Cells(m, 1) <> "" And IsNumeric(Range(batchInputDataCells).Cells(m, 1)) Then
        Range(inputCell).Value = Range(batchInputDataCells).Cells(m, 1) 'Triggers change handler
        Call ResetAll
        Range(batchRunningCell).Value = "*RUNNING*"
        bBatch = True
        Call Solve
        bBatch = False
        For n = 1 To batchMaxOutputValues
            If outputCells(n) <> "" Then _
                Range(batchOutputDataCells).Cells(m, n).Value = Range(outputCells(n))
        Next n
    End If
Next m

Range(inputCell).Value = saveInput        'Restore input cell. May trigger change handler.
Call ResetAll       'Clears batchRunningCell
Range(loadedFileCell).Cells.Font.Color = fileNameColor
End Sub

Function ValidInputField(ByVal inputField As String) As Boolean
'Tests inputField to see if it is in the list of valid input parameters for batch solution
If InStr(inputField, "dashboard") Or InStr(inputField, "device") Or InStr(inputField, "illumination") _
    Or InStr(inputField, "DASHBOARD") Or InStr(inputField, "DEVICE") Or InStr(inputField, "ILLUMINATION") Then _
        MsgBox ("Sheet names must be capitalized (first-letter only).")
ValidInputField = True
If InStr(inputField, "Dashboard") > 0 Then
    If Not Application.Intersect(Range(inputField), Range(voltageCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(resolutionCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(convergenceCell)) Is Nothing Then Exit Function
End If
If InStr(inputField, "Device") > 0 Then
    If Not Application.Intersect(Range(inputField), Range(textureDepthCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(baseHeightCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(textureAngleCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(deviceDiffusionPeakCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(deviceDiffusionDepthCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(deviceDopingCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(frontChargePeakCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(frontChargeValleyCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(permittivityCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(frontSpeakCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(frontSvalleyCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(temperatureCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(tauNCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(tauPCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(intrinsic25Cell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(augerPtypeCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(augerNtypeCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(BGNCells)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(defectPositionCells)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(defectTauNCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(defectTauPCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(defectBGNCell)) Is Nothing Then Exit Function
End If
If InStr(inputField, "Illumination") > 0 Then
    If Not Application.Intersect(Range(inputField), Range(frontSunsCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(frontPowerCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(lambdaCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(encapsulationIndexCell)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(spectrumDataCells)) Is Nothing Then Exit Function
    If Not Application.Intersect(Range(inputField), Range(coatingDataCells)) Is Nothing Then Exit Function
End If
ValidInputField = False 'If not trapped in any of the above, it's not allowed
End Function
