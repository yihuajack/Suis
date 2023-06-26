Attribute VB_Name = "FileHandling"
Option Explicit
Const nDashboardValues = 22
Const nDeviceValues = 12
Const nDeviceRanges = 21
Const nRecombValues = 8
Const nRecombRanges = 10
Const nIllumValues = 28
Const nIllumRanges = 1
Const nDataRanges = 2
Const nModelValues = 3
Const nModelRanges = 5

Function GetWindowsPath(ByRef filePath As Variant, ByVal bSave As Boolean, ByVal b3Ddata As Boolean) As Boolean
If b3Ddata Then
    filePath = Application.GetSaveAsFilename(FileFilter:="3D Data (*.csv), *.csv")
Else
    If bSave Then
        filePath = Application.GetSaveAsFilename(FileFilter:="PC3D Files (*.txt), *.txt")
    Else
        filePath = Application.GetOpenFilename(FileFilter:="PC3D Files (*.txt), *.txt")
    End If
End If
GetWindowsPath = filePath <> False
End Function

Function GetMacPath(ByRef filePath As Variant, ByVal bSave As Boolean, ByVal b3Ddata As Boolean) As Boolean
Dim saveMacScript As String, loadMacScript As String, fileName As String
If b3Ddata Then
    fileName = ""
    saveMacScript = "set applescript's text item delimiters to "","" " & vbNewLine & _
        "try " & vbNewLine & _
            "set theFile to (choose file name with prompt ""Save as file"" default name """ & _
               fileName & """ default location path to desktop) as text" & vbNewLine & _
            "if theFile does not end with "".csv"" then set theFile to theFile & "".csv""" & vbNewLine & _
            "on error errStr number errorNumber" & vbNewLine & _
            "return errorNumber " & vbNewLine & _
        "end try " & vbNewLine & _
        "return theFile"
    filePath = MacScript(saveMacScript)
    GetMacPath = VBA.Left(filePath, 1) <> "-"
Else
    fileName = Range(loadedFileCell)
    saveMacScript = "set applescript's text item delimiters to "","" " & vbNewLine & _
        "try " & vbNewLine & _
            "set theFile to (choose file name with prompt ""Save as file"" default name """ & _
               fileName & """ default location path to desktop) as text" & vbNewLine & _
            "if theFile does not end with "".txt"" then set theFile to theFile & "".txt""" & vbNewLine & _
            "on error errStr number errorNumber" & vbNewLine & _
            "return errorNumber " & vbNewLine & _
        "end try " & vbNewLine & _
        "return theFile"
    loadMacScript = "set applescript's text item delimiters to "","" " & vbNewLine & _
        "try " & vbNewLine & _
            "set theFile to (choose file with prompt ""Please select a file"" without multiple selections allowed) as string" & vbNewLine & _
            "on error errStr number errorNumber" & vbNewLine & _
            "return errorNumber " & vbNewLine & _
        "end try " & vbNewLine & _
        "return theFile"
    If bSave Then filePath = MacScript(saveMacScript) Else filePath = MacScript(loadMacScript)
    GetMacPath = VBA.Left(filePath, 1) <> "-"
End If
End Function

Sub SaveFile()
Dim filePath As Variant
If Not bDebug Then On Error GoTo ErrHandler:
If bMac Then
    If GetMacPath(filePath, True, False) Then Call SaveFileContents(filePath) Else Exit Sub
Else
    If GetWindowsPath(filePath, True, False) Then Call SaveFileContents(filePath) Else Exit Sub
End If
MsgBox ("File Saved Successfully")
Call BlackFilename
Exit Sub
    
ErrHandler:
    Range(progressCell).Value = "Error"
    MsgBox ("Error Saving File.")
End Sub

Sub SaveFileContents(ByVal filePath As Variant)
Dim index As Integer, f As Integer
Dim fLine, myDir As String
f = FreeFile
Close #f        'In case it's already open
Open filePath For Output As #f
myDir = Dir(filePath)
Range(loadedFileCell).Value = VBA.Left(Dir(filePath), Len(Dir(filePath)) - 4)
Dim valueArray As Variant, valueNames As Variant
Dim rangeArray As Variant, rangeNames As Variant
'PROGRAM
Print #f, MakeValueStr("Program", "Name", Range(programNameCell))
Print #f, MakeValueStr("Program", "Version", Range(programVersionCell))
'DASHBOARD
ReDim valueArray(1 To nDashboardValues) As Variant
ReDim valueNames(1 To nDashboardValues) As String
Call SetDashboardValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #f, MakeValueStr("Dashboard", valueNames(index), valueArray(index))
Next index
'DEVICE
ReDim valueArray(1 To nDeviceValues) As Variant
ReDim valueNames(1 To nDeviceValues) As String
Call SetDeviceValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #f, MakeValueStr("Device", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nDeviceRanges) As Variant
ReDim rangeNames(1 To nDeviceRanges) As String
Call SetDeviceRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #f, MakeArrayStr("Device", rangeNames(index), rangeArray(index))
Next index
'RECOMBINATION
ReDim valueArray(1 To nRecombValues) As Variant
ReDim valueNames(1 To nRecombValues) As String
Call SetRecombValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #f, MakeValueStr("Recombination", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nRecombRanges) As Variant
ReDim rangeNames(1 To nRecombRanges) As String
Call SetRecombRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #f, MakeArrayStr("Recombination", rangeNames(index), rangeArray(index))
Next index
'ILLUMINATION
ReDim valueArray(1 To nIllumValues) As Variant
ReDim valueNames(1 To nIllumValues) As String
Call SetIllumValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #f, MakeValueStr("Illumination", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nIllumRanges) As Variant
ReDim rangeNames(1 To nIllumRanges) As String
Call SetIllumRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #f, MakeArrayStr("Illumination", rangeNames(index), rangeArray(index))
Next index
'DATA
ReDim rangeArray(1 To nDataRanges) As Variant
ReDim rangeNames(1 To nDataRanges) As String
Call SetDataRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #f, MakeArrayStr("Data", rangeNames(index), rangeArray(index))
Next index
'MODELS
ReDim valueArray(1 To nModelValues) As Variant
ReDim valueNames(1 To nModelValues) As String
Call SetModelValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #f, MakeValueStr("Models", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nModelRanges) As Variant
ReDim rangeNames(1 To nModelRanges) As String
Call SetModelRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #f, MakeArrayStr("Models", rangeNames(index), rangeArray(index))
Next index
Close #f
End Sub

Sub LoadFile()
Dim filePath As Variant
If Not bDebug Then On Error GoTo ErrHandler:
bEnableChangeHandlers = False
Application.Calculation = xlCalculationManual
If bMac Then
    If GetMacPath(filePath, False, False) Then Call LoadFileContents(filePath) Else Exit Sub
Else
    If GetWindowsPath(filePath, False, False) Then Call LoadFileContents(filePath) Else Exit Sub
End If
MsgBox ("File Loaded Successfully")
Call BackwardCompatibility
Call UpdateCircuitModel
Call AdjustGraphAxes
Application.Calculation = xlCalculationAutomatic    'Apply formulas before calling ResetAll, which restores xCalculationManual
Call ResetAll
Call BlackFilename
Exit Sub
    
ErrHandler:
    Range(progressCell).Value = "Error"
    MsgBox ("Error Loading File.")
    Application.Calculation = xlCalculationAutomatic
    bEnableChangeHandlers = True
End Sub

Sub LoadFileContents(ByVal filePath As Variant)
Dim index As Integer, nRows As Integer, nCols As Integer
Dim f As Integer
Dim fLine As Variant
Dim thisGroup As String, thisName As String
Dim thisValue As Variant, valueArray As Variant, valueNames As Variant
Dim thisRange As Variant, rangeArray As Variant, rangeNames As Variant
f = FreeFile
Open filePath For Input As #f
If Not ReadProgramLine(f) Then
    MsgBox ("That is not a PC3D file."): Close #f: Exit Sub
End If
Range(loadedFileCell).Value = VBA.Left(Dir(filePath), Len(Dir(filePath)) - 4)
If Not ReadVersionLine(f) Then MsgBox ("This file was created in a different version of PC3D." _
                            & vbCrLf & "Some parameters may not be loaded.")
Range(progressCell).Value = "Loading"
Do While Not EOF(f)
    Line Input #f, fLine
    thisGroup = GetGroup(fLine)
    If thisGroup = "Dashboard" Then
        ReDim valueArray(1 To nDashboardValues) As Variant
        ReDim valueNames(1 To nDashboardValues) As String
        Call SetDashboardValues(valueArray, valueNames)
        thisName = GetName(fLine)
        For index = 1 To nDashboardValues
            If valueNames(index) = thisName Then
                thisValue = GetValue(fLine)
                Range(valueArray(index).Address(True, True, xlA1, True)) = thisValue
            End If
        Next index
    End If
    If thisGroup = "Device" Then
        ReDim valueArray(1 To nDeviceValues) As Variant
        ReDim valueNames(1 To nDeviceValues) As String
        Call SetDeviceValues(valueArray, valueNames)
        thisName = GetName(fLine)
        For index = 1 To nDeviceValues
            If valueNames(index) = thisName Then
                thisValue = GetValue(fLine)
                Range(valueArray(index).Address(True, True, xlA1, True)) = thisValue
            End If
        Next index
        ReDim rangeArray(1 To nDeviceRanges) As Variant
        ReDim rangeNames(1 To nDeviceRanges) As String
        Call SetDeviceRanges(rangeArray, rangeNames)
        thisName = GetName(fLine)
        For index = 1 To nDeviceRanges
            If rangeNames(index) = thisName Then
                nRows = rangeArray(index).Rows.Count
                nCols = rangeArray(index).Columns.Count
                thisRange = GetRange(nRows, nCols, fLine)
                Range(rangeArray(index).Address(True, True, xlA1, True)) = thisRange
            End If
        Next index
    End If
    If thisGroup = "Recombination" Then
        ReDim valueArray(1 To nRecombValues) As Variant
        ReDim valueNames(1 To nRecombValues) As String
        Call SetRecombValues(valueArray, valueNames)
        thisName = GetName(fLine)
        For index = 1 To nRecombValues
            If valueNames(index) = thisName Then
                thisValue = GetValue(fLine)
                Range(valueArray(index).Address(True, True, xlA1, True)) = thisValue
            End If
        Next index
        ReDim rangeArray(1 To nRecombRanges) As Variant
        ReDim rangeNames(1 To nRecombRanges) As String
        Call SetRecombRanges(rangeArray, rangeNames)
        thisName = GetName(fLine)
        For index = 1 To nRecombRanges
            If rangeNames(index) = thisName Then
                nRows = rangeArray(index).Rows.Count
                nCols = rangeArray(index).Columns.Count
                thisRange = GetRange(nRows, nCols, fLine)
                Range(rangeArray(index).Address(True, True, xlA1, True)) = thisRange
            End If
        Next index
    End If
    If thisGroup = "Illumination" Then
        ReDim valueArray(1 To nIllumValues) As Variant
        ReDim valueNames(1 To nIllumValues) As String
        Call SetIllumValues(valueArray, valueNames)
        thisName = GetName(fLine)
        For index = 1 To nIllumValues
            If valueNames(index) = thisName Then
                thisValue = GetValue(fLine)
                Range(valueArray(index).Address(True, True, xlA1, True)) = thisValue
            End If
        Next index
        ReDim rangeArray(1 To nIllumRanges) As Variant
        ReDim rangeNames(1 To nIllumRanges) As String
        Call SetIllumRanges(rangeArray, rangeNames)
        thisName = GetName(fLine)
        For index = 1 To nIllumRanges
            If rangeNames(index) = thisName Then
                nRows = rangeArray(index).Rows.Count
                nCols = rangeArray(index).Columns.Count
                thisRange = GetRange(nRows, nCols, fLine)
                Range(rangeArray(index).Address(True, True, xlA1, True)) = thisRange
            End If
        Next index
    End If
    If thisGroup = "Data" Then
        ReDim rangeArray(1 To nDataRanges) As Variant
        ReDim rangeNames(1 To nDataRanges) As String
        Call SetDataRanges(rangeArray, rangeNames)
        thisName = GetName(fLine)
        For index = 1 To nDataRanges
            If rangeNames(index) = thisName Then
                nRows = rangeArray(index).Rows.Count
                nCols = rangeArray(index).Columns.Count
                thisRange = GetRange(nRows, nCols, fLine)
                Range(rangeArray(index).Address(True, True, xlA1, True)) = thisRange
            End If
        Next index
    End If
    If thisGroup = "Models" Then
        ReDim valueArray(1 To nModelValues) As Variant
        ReDim valueNames(1 To nModelValues) As String
        Call SetModelValues(valueArray, valueNames)
        thisName = GetName(fLine)
        For index = 1 To nModelValues
            If valueNames(index) = thisName Then
                thisValue = GetValue(fLine)
                Range(valueArray(index).Address(True, True, xlA1, True)) = thisValue
            End If
        Next index
        ReDim rangeArray(1 To nModelRanges) As Variant
        ReDim rangeNames(1 To nModelRanges) As String
        Call SetModelRanges(rangeArray, rangeNames)
        thisName = GetName(fLine)
        For index = 1 To nModelRanges
            If rangeNames(index) = thisName Then
                nRows = rangeArray(index).Rows.Count
                nCols = rangeArray(index).Columns.Count
                thisRange = GetRange(nRows, nCols, fLine)
                Range(rangeArray(index).Address(True, True, xlA1, True)) = thisRange
            End If
        Next index
    End If
    DoEvents    'This is necessary to keep from overloading the operating system event stack
Loop
Close #f
Range(progressCell).Value = ""
End Sub

Function MakeValueStr(ByVal groupStr As String, ByVal nameStr As String, ByVal rValue As Variant) As String
MakeValueStr = groupStr & "." & nameStr & "=" & rValue.Formula
End Function

Function MakeArrayStr(ByVal groupStr As String, ByVal nameStr As String, ByRef rArray As Variant) As String
Dim item As Variant
MakeArrayStr = groupStr & "." & nameStr & "="
For Each item In rArray
    MakeArrayStr = MakeArrayStr & item.Formula & ","
Next item
MakeArrayStr = VBA.Left(MakeArrayStr, Len(MakeArrayStr) - 1)
End Function

Function ReadProgramLine(ByRef f As Integer) As Boolean
Dim fLine As String
If Not EOF(f) Then Line Input #f, fLine
ReadProgramLine = InStr(1, fLine, "PC3D") > 0
End Function

Function ReadVersionLine(ByRef f As Integer) As Boolean
Dim fLine As String, vLine As String
ReadVersionLine = False
If Not EOF(f) Then Line Input #f, fLine
If GetGroup(fLine) <> "Program" Then Exit Function
If GetName(fLine) <> "Version" Then Exit Function
fLine = GetValue(fLine)
fLine = VBA.Left(fLine, InStr(1, fLine, ".") - 1)
fLine = VBA.Right(fLine, 1) 'This is the major version number of the file
vLine = Range(programVersionCell)
vLine = VBA.Left(vLine, InStr(1, vLine, ".") - 1)
vLine = VBA.Right(vLine, 1) 'This is the major version number of the program
ReadVersionLine = (fLine = vLine)
End Function

Function GetGroup(ByVal fLine As String) As String
Dim index As Integer
index = InStr(1, fLine, ".", vbTextCompare)
If index > 1 Then GetGroup = VBA.Left(fLine, index - 1)
VBA.Trim (GetGroup)
End Function

Function GetName(ByVal fLine As String) As String
Dim index As Integer
index = VBA.InStr(1, fLine, ".", vbTextCompare)
If index > 0 Then fLine = VBA.Right(fLine, VBA.Len(fLine) - index)
index = VBA.InStr(1, fLine, "=", vbTextCompare)
If index > 1 Then GetName = VBA.Left(fLine, index - 1)
VBA.Trim (GetName)
End Function

Function GetValue(ByVal fLine As String) As Variant
Dim index As Integer
Dim valueStr As String
Do While VBA.Right(fLine, 1) = vbLf Or VBA.Right(fLine, 1) = vbCr   'Strip CR/LF characters
    fLine = VBA.Left(fLine, VBA.Len(fLine) - 1)
Loop
index = VBA.InStr(1, fLine, "=", vbTextCompare)
If index > 0 Then valueStr = VBA.Right(fLine, VBA.Len(fLine) - index)
If valueStr = "True" Then
    GetValue = True: Exit Function
End If
If valueStr = "False" Then
    GetValue = False: Exit Function
End If
If IsNumeric(valueStr) Then
    GetValue = Val(valueStr): Exit Function
End If
GetValue = valueStr 'If none of the above, then interpret as a String (which may be a formula).
End Function

Function GetRange(ByVal nRows As Integer, ByVal nCols As Integer, ByVal fLine As String) As Variant
Dim index As Integer, row As Integer, col As Integer
Dim localRange As Variant
Dim localStr As String
ReDim localRange(1 To nRows, 1 To nCols) As Variant
Do While VBA.Right(fLine, 1) = vbLf Or VBA.Right(fLine, 1) = vbCr   'Strip CR/LF characters
    fLine = VBA.Left(fLine, VBA.Len(fLine) - 1)
Loop
index = VBA.InStr(1, fLine, "=", vbTextCompare)
If index > 0 Then
    fLine = VBA.Right(fLine, VBA.Len(fLine) - index)
    For row = 1 To nRows: For col = 1 To nCols
        index = VBA.InStr(1, fLine, ",", vbTextCompare)
        If index > 1 Then
            localStr = VBA.Left(fLine, index - 1)
            localRange(row, col) = localStr
        Else
            localRange(row, col) = ""
        End If
        fLine = VBA.Right(fLine, VBA.Len(fLine) - index)
    Next col: Next row
    If VBA.Len(fLine) > 0 Then localRange(nRows, nCols) = fLine
End If
GetRange = localRange
End Function

Sub SetDashboardValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(voltageCell): rNames(1) = "ContactVoltage"
Set rValues(2) = Range(lastPlotViewCell): rNames(2) = "LastPlotView"
Set rValues(3) = Range(selectSurfaceCell): rNames(3) = "SelectSurface"
Set rValues(4) = Range(ivTypeCell): rNames(4) = "IVtype"
Set rValues(5) = Range(voltageIncrementsCell): rNames(5) = "IVincrements"
Set rValues(6) = Range(darkIVmaxCell): rNames(6) = "DarkIVmax"
Set rValues(7) = Range(illuminationPowerCell): rNames(7) = "IlluminationPower"
Set rValues(8) = Range(efficiencyCell): rNames(8) = "Efficiency"
Set rValues(9) = Range(ivWarningCell): rNames(9) = "IVwarning"
Set rValues(10) = Range(iscCell): rNames(10) = "Jsc"
Set rValues(11) = Range(vocCell): rNames(11) = "Voc"
Set rValues(12) = Range(pmaxCell): rNames(12) = "Pmax"
Set rValues(13) = Range(ffCell): rNames(13) = "FillFactor"
Set rValues(14) = Range(eqeSideCell): rNames(14) = "EQEside"
Set rValues(15) = Range(minLambdaCell): rNames(15) = "EQEminLambda"
Set rValues(16) = Range(maxLambdaCell): rNames(16) = "EQEmaxLambda"
Set rValues(17) = Range(eqeIncrementsCell): rNames(17) = "EQEstepsLambda"
Set rValues(18) = Range(eqeBiasCell): rNames(18) = "EQEbias"
Set rValues(19) = Range(eqeTotalCurrentCell): rNames(19) = "EQEtotalCurrent"
Set rValues(20) = Range(eqeWarningCell): rNames(20) = "EQEwarning"
Set rValues(21) = Range(eqeFastCell): rNames(21) = "EQEfast"
Set rValues(22) = Range(viewCell): rNames(22) = "View"
End Sub

Sub SetDeviceValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(deviceDescriptionCell): rNames(1) = "Description"
Set rValues(2) = Range(enable3DCell): rNames(2) = "Enable3D"
Set rValues(3) = Range(deviceWidthCell): rNames(3) = "Width"
Set rValues(4) = Range(deviceLengthCell): rNames(4) = "Length"
Set rValues(5) = Range(deviceHeightCell): rNames(5) = "Depth"   'Name retained for backward compatibility
Set rValues(6) = Range(deviceHeightCell): rNames(6) = "Height"
Set rValues(7) = Range(temperatureCell): rNames(7) = "Temperature"
Set rValues(8) = Range(deviceSeriesCell): rNames(8) = "SeriesR"
Set rValues(9) = Range(deviceDopingCell): rNames(9) = "Doping"
Set rValues(10) = Range(enableSiDiffusivitiesCell): rNames(10) = "SiDiffusivities"
Set rValues(11) = Range(deviceDnCell): rNames(11) = "Dn"
Set rValues(12) = Range(deviceDpCell): rNames(12) = "Dp"
End Sub

Sub SetDeviceRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(frontLengthUnitCells): rNames(1) = "FrontLengthUnits"
Set rValues(2) = Range(frontWidthUnitCells): rNames(2) = "FrontWidthUnits"
Set rValues(3) = Range(rearLengthUnitCells): rNames(3) = "RearLengthUnits"
Set rValues(4) = Range(rearWidthUnitCells): rNames(4) = "RearWidthUnits"
Set rValues(5) = Range(frontRowLengthCells): rNames(5) = "FrontRowLengths"
Set rValues(6) = Range(frontColumnWidthCells): rNames(6) = "FrontColWidths"
Set rValues(7) = Range(rearRowLengthCells): rNames(7) = "RearRowLengths"
Set rValues(8) = Range(rearColumnWidthCells): rNames(8) = "RearColWidths"
Set rValues(9) = Range(spectralTransCells): rNames(9) = "SpectralTransmission"
Set rValues(10) = Range(frontContactCells): rNames(10) = "FrontContacts"
Set rValues(11) = Range(frontSheetRhoCells): rNames(11) = "FrontSheetRho"
Set rValues(12) = Range(frontTransCells): rNames(12) = "FrontTransmission"
Set rValues(13) = Range(frontHazeCells): rNames(13) = "FrontHaze"
Set rValues(14) = Range(frontSpecRefCells): rNames(14) = "FrontSpecularReflectance"
Set rValues(15) = Range(frontDiffRefCells): rNames(15) = "FrontDiffuseReflectance"
Set rValues(16) = Range(rearContactCells): rNames(16) = "RearContacts"
Set rValues(17) = Range(rearSheetRhoCells): rNames(17) = "RearSheetRho"
Set rValues(18) = Range(rearTransCells): rNames(18) = "RearTransmission"
Set rValues(19) = Range(rearHazeCells): rNames(19) = "RearHaze"
Set rValues(20) = Range(rearSpecRefCells): rNames(20) = "RearSpecularReflectance"
Set rValues(21) = Range(rearDiffRefCells): rNames(21) = "RearDiffuseReflectance"
End Sub

Sub SetRecombValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(intrinsicCell): rNames(1) = "IntrinsicConcentration"
Set rValues(2) = Range(tauNCell): rNames(2) = "ElectronLifetime"
Set rValues(3) = Range(tauNUnitsCell): rNames(3) = "ElectronLifetimeUnits"
Set rValues(4) = Range(tauPCell): rNames(4) = "HoleLifetime"
Set rValues(5) = Range(tauPUnitsCell): rNames(5) = "HoleLifetimeUnits"
Set rValues(6) = Range(augerCell): rNames(6) = "AugerCoefficient"
Set rValues(7) = Range(lineDefectDirectionCell): rNames(7) = "LineDefectDirection"
Set rValues(8) = Range(planeDefectDirectionCell): rNames(8) = "PlaneDefectDirection"
End Sub

Sub SetRecombRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(frontJo1Cells): rNames(1) = "FrontJo1"
Set rValues(2) = Range(frontJo2Cells): rNames(2) = "FrontJo2"
Set rValues(3) = Range(frontGshCells): rNames(3) = "FrontGsh"
Set rValues(4) = Range(rearJo1Cells): rNames(4) = "RearJo1"
Set rValues(5) = Range(rearJo2Cells): rNames(5) = "RearJo2"
Set rValues(6) = Range(rearGshCells): rNames(6) = "RearGsh"
Set rValues(7) = Range(defectPositionCells): rNames(7) = "DefectPosition"
Set rValues(8) = Range(defectEnableCells): rNames(8) = "EnableDefects"
Set rValues(9) = Range(defectIoCells): rNames(9) = "DefectIo"
Set rValues(10) = Range(defectNCells): rNames(10) = "DefectN"
End Sub

Sub SetIllumValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(spectrumCell): rNames(1) = "SpectrumSelect"
Set rValues(2) = Range(frontSunsCell): rNames(2) = "FrontSuns"
Set rValues(3) = Range(rearSunsCell): rNames(3) = "RearSuns"
Set rValues(4) = Range(frontPowerCell): rNames(4) = "FrontPower"
Set rValues(5) = Range(rearPowerCell): rNames(5) = "RearPower"
Set rValues(6) = Range(lambdaCell): rNames(6) = "MonoWavelength"
Set rValues(7) = Range(opticalFilterCell): rNames(7) = "OpticalFilter"
Set rValues(8) = Range(filterBandgapCell): rNames(8) = "FilterBandgap"
Set rValues(9) = Range(filterEdgeCell): rNames(9) = "FilterEdge"
Set rValues(10) = Range(filterSubgapCell): rNames(10) = "FilterSubgap"
Set rValues(11) = Range(frontShapeCell): rNames(11) = "FrontShape"
Set rValues(12) = Range(rearShapeCell): rNames(12) = "RearShape"
Set rValues(13) = Range(frontPointXcenterCell): rNames(13) = "PointX"
Set rValues(14) = Range(frontPointYcenterCell): rNames(14) = "PointY"
Set rValues(15) = Range(frontPointZcenterCell): rNames(15) = "PointZ"
Set rValues(16) = Range(frontPointDiameterCell): rNames(16) = "PointDiameter"
Set rValues(17) = Range(frontLineDirectionCell): rNames(17) = "LineDirection"
Set rValues(18) = Range(frontLineShapeCell): rNames(18) = "LineShape"
Set rValues(19) = Range(frontLinePeakCell): rNames(19) = "LinePeak"
Set rValues(20) = Range(frontLineWidthCell): rNames(20) = "LineWidth"
Set rValues(21) = Range(rearPointXcenterCell): rNames(21) = "RearPointX"
Set rValues(22) = Range(rearPointYcenterCell): rNames(22) = "RearPointY"
Set rValues(23) = Range(rearPointZcenterCell): rNames(23) = "RearPointZ"
Set rValues(24) = Range(rearPointDiameterCell): rNames(24) = "RearPointDiameter"
Set rValues(25) = Range(rearLineDirectionCell): rNames(25) = "RearLineDirection"
Set rValues(26) = Range(rearLineShapeCell): rNames(26) = "RearLineShape"
Set rValues(27) = Range(rearLinePeakCell): rNames(27) = "RearLinePeak"
Set rValues(28) = Range(rearLineWidthCell): rNames(28) = "RearLineWidth"
End Sub

Sub SetIllumRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(spectrumDataCells): rNames(1) = "SpectrumData"
End Sub

Sub SetDataRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(IVrange): rNames(1) = "IVdata"
Set rValues(2) = Range(eqeRange): rNames(2) = "EQEdata"
End Sub

Sub SetModelValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(enableTemperatureCell): rNames(1) = "EnableTemperature"
Set rValues(2) = Range(modelTempExponentCell): rNames(2) = "TemperatureExponent"
Set rValues(3) = Range(enableDopingCell): rNames(3) = "EnableDoping"
End Sub

Sub SetModelRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(modelTempNiCells): rNames(1) = "IntrinsicTemperatureModel"
Set rValues(2) = Range(modelTempOpticsCells): rNames(2) = "OpticsTemperatureModel"
Set rValues(3) = Range(modelTempSurfaceCells): rNames(3) = "SurfaceTemperatureModel"
Set rValues(4) = Range(modelDopingBulkCells): rNames(4) = "BulkDopingModel"
Set rValues(5) = Range(modelDopingSurfaceCells): rNames(5) = "SurfaceDopingModel"
End Sub

Sub BackwardCompatibility()
If Range(enableSiDiffusivitiesCell) = "True" Or Range(enableSiDiffusivitiesCell) = "TRUE" Then _
    Range(enableSiDiffusivitiesCell).Value = "Silicon"
If Range(enableSiDiffusivitiesCell) = "False" Or Range(enableSiDiffusivitiesCell) = "FALSE" Then _
    Range(enableSiDiffusivitiesCell).Value = "Manual"
If Range(opticalFilterCell) = "True" Or Range(opticalFilterCell) = "TRUE" Then _
    Range(opticalFilterCell).Value = "Enable"
If Range(opticalFilterCell) = "False" Or Range(opticalFilterCell) = "FALSE" _
    Then Range(opticalFilterCell).Value = "Disable"
If Range(enable3DCell) = "True" Or Range(enable3DCell) = "TRUE" Then _
    Range(enable3DCell).Value = "3D layout"
If Range(enable3DCell) = "False" Or Range(enable3DCell) = "FALSE" Then _
    Range(enable3DCell).Value = "2D layout"
If Range(eqeFastCell) = "True" Or Range(eqeFastCell) = "TRUE" Then _
    Range(eqeFastCell).Value = "Fast"
If Range(eqeFastCell) = "False" Or Range(eqeFastCell) = "FALSE" Then _
    Range(eqeFastCell).Value = "Accurate"
If Range(defectEnableCells).Cells(point, 1) = "True" Or Range(defectEnableCells).Cells(point, 1) = "TRUE" Then _
    Range(defectEnableCells).Cells(point, 1).Value = "Enable"
If Range(defectEnableCells).Cells(point, 1) = "False" Or Range(defectEnableCells).Cells(point, 1) = "FALSE" Then _
    Range(defectEnableCells).Cells(point, 1).Value = "Disable"
If Range(defectEnableCells).Cells(line, 1) = "True" Or Range(defectEnableCells).Cells(line, 1) = "TRUE" Then _
    Range(defectEnableCells).Cells(line, 1).Value = "Enable"
If Range(defectEnableCells).Cells(line, 1) = "False" Or Range(defectEnableCells).Cells(line, 1) = "FALSE" Then _
    Range(defectEnableCells).Cells(line, 1).Value = "Disable"
If Range(defectEnableCells).Cells(plane, 1) = "True" Or Range(defectEnableCells).Cells(plane, 1) = "TRUE" Then _
    Range(defectEnableCells).Cells(plane, 1).Value = "Enable"
If Range(defectEnableCells).Cells(plane, 1) = "False" Or Range(defectEnableCells).Cells(plane, 1) = "FALSE" Then _
    Range(defectEnableCells).Cells(plane, 1).Value = "Disable"
If Range(lineDefectDirectionCell) = "X-axis" Then Range(lineDefectDirectionCell).Value = "Side"
If Range(lineDefectDirectionCell) = "Y-axis" Then Range(lineDefectDirectionCell).Value = "Face"
If Range(lineDefectDirectionCell) = "Z-axis" Then Range(lineDefectDirectionCell).Value = "Top"
If Range(planeDefectDirectionCell) = "X-axis" Then Range(planeDefectDirectionCell).Value = "Side"
If Range(planeDefectDirectionCell) = "Y-axis" Then Range(planeDefectDirectionCell).Value = "Face"
If Range(planeDefectDirectionCell) = "Z-axis" Then Range(planeDefectDirectionCell).Value = "Top"
If Range(frontLineDirectionCell) = "X-axis" Then Range(frontLineDirectionCell).Value = "Side"
If Range(frontLineDirectionCell) = "Y-axis" Then Range(frontLineDirectionCell).Value = "Face"
If Range(frontLineShapeCell) = "Uniform" Then Range(frontLineShapeCell).Value = "Box"
End Sub

Sub Save3D()
'Saves the hole quasi-Fermi potential, the electron quasi-Fermi potential, and the electrostatic potential,
'at each node in the solution volume, in a comma-separated (csv) text file for use with other programs.
Dim filePath As Variant
If Not bDebug Then On Error GoTo ErrHandler:
If bMac Then
    If GetMacPath(filePath, True, True) Then Call Save3Dcontent(filePath)
Else
    If GetWindowsPath(filePath, True, True) Then Call Save3Dcontent(filePath)
End If
Exit Sub
    
ErrHandler:
    MsgBox ("Error Saving File.")
End Sub

Sub Save3Dcontent(ByVal filePath As Variant)
Dim i As Integer, j As Integer, k As Integer, f As Integer
Dim x As Double, y As Double, z As Double
Dim fLine, myDir As String
f = FreeFile
Close #f        'In case it's already open
If Not bGamma Then Call CalculateRealGamma
If Not bPsi Then Call CalculateRealPsi
Open filePath For Output As #f
myDir = Dir(filePath)
Dim valueArray As Variant, valueNames As Variant
Dim rangeArray As Variant, rangeNames As Variant
Print #f, "x(mm), y(mm), z(mm), PhiP(V), PhiN(V), Psi(V)"
For k = 1 To nNodesZ: For j = 1 To nNodesY: For i = 1 To nNodesX
    x = 10 * (i - 1) / (nNodesX - 1) * deviceWidth
    y = 10 * (j - 1) / (nNodesY - 1) * deviceLength
    z = 10 * (k - 1) / (nNodesZ - 1) * deviceHeight
    Print #f, x & ", " & y & ", " & z & ", " & _
            PhiP(vGamma(i, j, k), vPsi(i, j, k)) & ", " & _
            PhiN(vGamma(i, j, k), vPsi(i, j, k)) & ", " & _
            vPsi(i, j, k) + rPsiEq
Next i: Next j: Next k
Close #f
End Sub

Sub UpdateCircuitModel()
If Range(ivTypeCell) = "Light IV" Then Call TwoDiodeModel(True, True, True)
If Range(ivTypeCell) = "Dark IV" Then Call TwoDiodeModel(False, True, True)
If Range(ivTypeCell) = "Pseudo IV" Then Call TwoDiodeModel(True, True, False)
If Range(ivTypeCell) = "SunsVoc" Then Call TwoDiodeModel(False, True, False)
End Sub

