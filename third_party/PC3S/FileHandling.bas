Attribute VB_Name = "FileHandling"
Option Explicit
Const nDashboardValues = 10
Const nDashboardRanges = 1
Const nDeviceValues = 33
Const nDeviceRanges = 2
Const nIllumValues = 5
Const nIllumRanges = 2
Const nDataValues = 2
Const nDataRanges = 2

Function GetWindowsPath(ByRef filePath As Variant, ByVal bSave As Boolean, ByVal b3Ddata As Boolean) As Boolean
If b3Ddata Then
    filePath = Application.GetSaveAsFilename(FileFilter:="3D Data (*.csv), *.csv")
Else
    If bSave Then
        filePath = Application.GetSaveAsFilename(FileFilter:="PC3S Files (*.txt), *.txt")
    Else
        filePath = Application.GetOpenFilename(FileFilter:="PC3S Files (*.txt), *.txt")
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
    GetMacPath = VBA.left(filePath, 1) <> "-"
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
    GetMacPath = VBA.left(filePath, 1) <> "-"
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
Dim index As Integer, F As Integer
Dim fLine, myDir As String
F = FreeFile
Close #F        'In case it's already open
Open filePath For Output As #F
myDir = Dir(filePath)
Range(loadedFileCell).Value = VBA.left(Dir(filePath), Len(Dir(filePath)) - 4)
Dim valueArray As Variant, valueNames As Variant
Dim rangeArray As Variant, rangeNames As Variant
'PROGRAM
Print #F, MakeValueStr("Program", "Name", Range(programNameCell))
Print #F, MakeValueStr("Program", "Version", Range(programVersionCell))
'DASHBOARD
ReDim valueArray(1 To nDashboardValues) As Variant
ReDim valueNames(1 To nDashboardValues) As String
Call SetDashboardValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #F, MakeValueStr("Dashboard", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nDashboardRanges) As Variant
ReDim rangeNames(1 To nDashboardRanges) As String
Call SetDashboardRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #F, MakeArrayStr("Dashboard", rangeNames(index), rangeArray(index))
Next index
'DEVICE
ReDim valueArray(1 To nDeviceValues) As Variant
ReDim valueNames(1 To nDeviceValues) As String
Call SetDeviceValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #F, MakeValueStr("Device", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nDeviceRanges) As Variant
ReDim rangeNames(1 To nDeviceRanges) As String
Call SetDeviceRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #F, MakeArrayStr("Device", rangeNames(index), rangeArray(index))
Next index
'ILLUMINATION
ReDim valueArray(1 To nIllumValues) As Variant
ReDim valueNames(1 To nIllumValues) As String
Call SetIllumValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #F, MakeValueStr("Illumination", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nIllumRanges) As Variant
ReDim rangeNames(1 To nIllumRanges) As String
Call SetIllumRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #F, MakeArrayStr("Illumination", rangeNames(index), rangeArray(index))
Next index
'DATA
ReDim valueArray(1 To nDataValues) As Variant
ReDim valueNames(1 To nDataValues) As String
Call SetDataValues(valueArray, valueNames)
For index = LBound(valueArray) To UBound(valueArray)
    Print #F, MakeValueStr("Data", valueNames(index), valueArray(index))
Next index
ReDim rangeArray(1 To nDataRanges) As Variant
ReDim rangeNames(1 To nDataRanges) As String
Call SetDataRanges(rangeArray, rangeNames)
For index = LBound(rangeArray) To UBound(rangeArray)
    Print #F, MakeArrayStr("Data", rangeNames(index), rangeArray(index))
Next index

Close #F
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
Application.Calculation = xlCalculationAutomatic    'Apply formulas before calling ResetAll, which restores xCalculationManual
currentPlot = "Doping"
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
Dim F As Integer
Dim fLine As Variant
Dim thisGroup As String, thisName As String
Dim thisValue As Variant, valueArray As Variant, valueNames As Variant
Dim thisRange As Variant, rangeArray As Variant, rangeNames As Variant
F = FreeFile
Open filePath For Input As #F
If Not ReadProgramLine(F) Then
    MsgBox ("That is not a PC3S file."): Close #F: Exit Sub
End If
Range(loadedFileCell).Value = VBA.left(Dir(filePath), Len(Dir(filePath)) - 4)
If Not ReadVersionLine(F) Then MsgBox ("This file was created in a different version of PC3S." _
                            & vbCrLf & "Some parameters may not be loaded.")
Range(progressCell).Value = "Loading"
Do While Not EOF(F)
    Line Input #F, fLine
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
        ReDim rangeArray(1 To nDashboardRanges) As Variant
        ReDim rangeNames(1 To nDashboardRanges) As String
        Call SetDashboardRanges(rangeArray, rangeNames)
        thisName = GetName(fLine)
        For index = 1 To nDashboardRanges
            If rangeNames(index) = thisName Then
                nRows = rangeArray(index).Rows.Count
                nCols = rangeArray(index).Columns.Count
                thisRange = GetRange(nRows, nCols, fLine)
                Range(rangeArray(index).Address(True, True, xlA1, True)) = thisRange
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
        ReDim valueArray(1 To nDataValues) As Variant
        ReDim valueNames(1 To nDataValues) As String
        Call SetDataValues(valueArray, valueNames)
        thisName = GetName(fLine)
        For index = 1 To nDataValues
            If valueNames(index) = thisName Then
                thisValue = GetValue(fLine)
                Range(valueArray(index).Address(True, True, xlA1, True)) = thisValue
            End If
        Next index
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
    DoEvents    'This is necessary to keep from overloading the operating system event stack
Loop
Close #F
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
MakeArrayStr = VBA.left(MakeArrayStr, Len(MakeArrayStr) - 1)
End Function

Function ReadProgramLine(ByRef F As Integer) As Boolean
Dim fLine As String
If Not EOF(F) Then Line Input #F, fLine
ReadProgramLine = InStr(1, fLine, "PC3S") > 0
End Function

Function ReadVersionLine(ByRef F As Integer) As Boolean
Dim fLine As String, vLine As String
ReadVersionLine = False
If Not EOF(F) Then Line Input #F, fLine
If GetGroup(fLine) <> "Program" Then Exit Function
If GetName(fLine) <> "Version" Then Exit Function
fLine = GetValue(fLine)
fLine = VBA.left(fLine, InStr(1, fLine, ".") - 1)
fLine = VBA.right(fLine, 1) 'This is the major version number of the file
vLine = Range(programVersionCell)
vLine = VBA.left(vLine, InStr(1, vLine, ".") - 1)
vLine = VBA.right(vLine, 1) 'This is the major version number of the program
ReadVersionLine = (fLine = vLine)
End Function

Function GetGroup(ByVal fLine As String) As String
Dim index As Integer
index = InStr(1, fLine, ".", vbTextCompare)
If index > 1 Then GetGroup = VBA.left(fLine, index - 1)
VBA.Trim (GetGroup)
End Function

Function GetName(ByVal fLine As String) As String
Dim index As Integer
index = VBA.InStr(1, fLine, ".", vbTextCompare)
If index > 0 Then fLine = VBA.right(fLine, VBA.Len(fLine) - index)
index = VBA.InStr(1, fLine, "=", vbTextCompare)
If index > 1 Then GetName = VBA.left(fLine, index - 1)
VBA.Trim (GetName)
End Function

Function GetValue(ByVal fLine As String) As Variant
Dim index As Integer
Dim valueStr As String
Do While VBA.right(fLine, 1) = vbLf Or VBA.right(fLine, 1) = vbCr   'Strip CR/LF characters
    fLine = VBA.left(fLine, VBA.Len(fLine) - 1)
Loop
index = VBA.InStr(1, fLine, "=", vbTextCompare)
If index > 0 Then valueStr = VBA.right(fLine, VBA.Len(fLine) - index)
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
Do While VBA.right(fLine, 1) = vbLf Or VBA.right(fLine, 1) = vbCr   'Strip CR/LF characters
    fLine = VBA.left(fLine, VBA.Len(fLine) - 1)
Loop
index = VBA.InStr(1, fLine, "=", vbTextCompare)
If index > 0 Then
    fLine = VBA.right(fLine, VBA.Len(fLine) - index)
    For row = 1 To nRows: For col = 1 To nCols
        index = VBA.InStr(1, fLine, ",", vbTextCompare)
        If index > 1 Then
            localStr = VBA.left(fLine, index - 1)
            localRange(row, col) = localStr
        Else
            localRange(row, col) = ""
        End If
        fLine = VBA.right(fLine, VBA.Len(fLine) - index)
    Next col: Next row
    If VBA.Len(fLine) > 0 Then localRange(nRows, nCols) = fLine
End If
GetRange = localRange
End Function

Sub SetDashboardValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(voltageCell): rNames(1) = "ContactVoltage"
Set rValues(2) = Range(contactModeCell): rNames(2) = "ContactMode"
Set rValues(3) = Range(illuminationCell): rNames(3) = "Illumination"
Set rValues(4) = Range(viewCell): rNames(4) = "View"
Set rValues(5) = Range(selectCarrierCell): rNames(5) = "Carrier"
Set rValues(6) = Range(positionCell): rNames(6) = "Position"
Set rValues(7) = Range(macroJo1Cell): rNames(7) = "Jo1"
Set rValues(8) = Range(macroJo2Cell): rNames(8) = "Jo2"
Set rValues(9) = Range(macroGshCell): rNames(9) = "Gsh"
Set rValues(10) = Range(macroSheetRhoCell): rNames(10) = "SheetRho"
End Sub

Sub SetDashboardRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(spectralRange): rNames(1) = "SpectralTransmission"
End Sub

Sub SetDeviceValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(deviceDescriptionCell): rNames(1) = "Description"
Set rValues(2) = Range(textureTypeCell): rNames(2) = "TextureType"
Set rValues(3) = Range(textureDepthCell): rNames(3) = "TextureDepth"
Set rValues(4) = Range(baseHeightCell): rNames(4) = "BaseHeight"
Set rValues(5) = Range(textureAngleCell): rNames(5) = "TextureAngle"
Set rValues(6) = Range(deviceDiffusionTypeCell): rNames(6) = "DiffusionType"
Set rValues(7) = Range(deviceDiffusionPeakCell): rNames(7) = "DiffusionPeak"
Set rValues(8) = Range(deviceDiffusionDepthCell): rNames(8) = "DiffusionDepth"
Set rValues(9) = Range(diffusionShapeCell): rNames(9) = "DiffusionShape"
Set rValues(10) = Range(deviceDopingTypeCell): rNames(10) = "BaseDopingType"
Set rValues(11) = Range(deviceDopingCell): rNames(11) = "BaseDoping"
Set rValues(12) = Range(frontChargePeakCell): rNames(12) = "PeakCharge"
Set rValues(13) = Range(frontChargeValleyCell): rNames(13) = "ValleyCharge"
Set rValues(14) = Range(frontChargeShapeCell): rNames(14) = "ChargeShape"
Set rValues(15) = Range(permittivityCell): rNames(15) = "Permittivity"
Set rValues(16) = Range(frontSpeakCell): rNames(16) = "PeakSRV"
Set rValues(17) = Range(frontSvalleyCell): rNames(17) = "ValleySRV"
Set rValues(18) = Range(frontSshapeCell): rNames(18) = "SRVshape"
Set rValues(19) = Range(intrinsic25Cell): rNames(19) = "Intrinsic25C"
Set rValues(20) = Range(tauNCell): rNames(20) = "TauN"
Set rValues(21) = Range(tauPCell): rNames(21) = "TauP"
Set rValues(22) = Range(augerPtypeCell): rNames(22) = "AugerP"
Set rValues(23) = Range(augerNtypeCell): rNames(23) = "AugerN"
Set rValues(24) = Range(temperatureCell): rNames(24) = "Temperature"
Set rValues(25) = Range(valenceBGNslopeCell): rNames(25) = "ValenceBGNslope"
Set rValues(26) = Range(valenceBGNonsetCell): rNames(26) = "ValenceBGNonset"
Set rValues(27) = Range(conductionBGNslopeCell): rNames(27) = "ConductionBGNslope"
Set rValues(28) = Range(conductionBGNonsetCell): rNames(28) = "ConductionBGNonset"
Set rValues(29) = Range(lineDefectDirectionCell): rNames(29) = "LineDefectDirection"
Set rValues(30) = Range(planeDefectDirectionCell): rNames(30) = "PlaneDefectDirection"
Set rValues(31) = Range(defectTauNCell): rNames(31) = "DefectTauN"
Set rValues(32) = Range(defectTauPCell): rNames(32) = "DefectTauP"
Set rValues(33) = Range(defectBGNCell): rNames(33) = "DefectBGN"
End Sub

Sub SetDeviceRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(defectEnableCells): rNames(1) = "DefectEnable"
Set rValues(2) = Range(defectPositionCells): rNames(2) = "DefectPosition"
End Sub

Sub SetIllumValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(spectrumCell): rNames(1) = "SpectrumSelect"
Set rValues(2) = Range(frontSunsCell): rNames(2) = "FrontSuns"
Set rValues(3) = Range(frontPowerCell): rNames(3) = "FrontPower"
Set rValues(4) = Range(lambdaCell): rNames(4) = "MonoWavelength"
Set rValues(5) = Range(encapsulationIndexCell): rNames(5) = "EncapsulationIndex"
End Sub

Sub SetIllumRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(spectrumDataCells): rNames(1) = "SpectrumData"
Set rValues(2) = Range(coatingDataCells): rNames(2) = "CoatingData"
End Sub

Sub SetDataValues(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(sheetRhoVoltageCell): rNames(1) = "SheetRhoVoltage"
Set rValues(2) = Range(sheetRhoCurrentCell): rNames(2) = "SheetRhoCurrent"
End Sub


Sub SetDataRanges(ByRef rValues As Variant, ByRef rNames As Variant)
Set rValues(1) = Range(dataIVCells): rNames(1) = "IVdata"
Set rValues(2) = Range(dataSpectralCells): rNames(2) = "SpectralData"
End Sub

Sub BackwardCompatibility()
'Ready to insert tests to accommodate backward compatibility
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
Dim i As Integer, j As Integer, k As Integer, F As Integer
Dim x As Double, y As Double, z As Double
Dim fLine, myDir As String
F = FreeFile
Close #F        'In case it's already open
Open filePath For Output As #F
myDir = Dir(filePath)
Dim valueArray As Variant, valueNames As Variant
Dim rangeArray As Variant, rangeNames As Variant
Print #F, "x(µm), y(µm), z(µm), PhiP(V), PhiN(V), Psi(V)"
For k = 1 To nNodesZ: For j = 1 To nNodesY: For i = 1 To nNodesX
    x = 10000# * (i - 1) / (nNodesX - 1) * deviceWidth
    y = 10000# * (j - 1) / (nNodesY - 1) * deviceLength
    z = 10000# * (k - 1) / (nNodesZ - 1) * deviceHeight
    Print #F, x & ", " & y & ", " & z & ", " & vPhiP(i, j, k) & ", " & vPhiN(i, j, k) & ", " & vPsi(i, j, k)
Next i: Next j: Next k
Close #F
End Sub

