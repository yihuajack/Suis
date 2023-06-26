Attribute VB_Name = "SpreadsheetInterface"
Option Explicit

Sub OnResetAll()
If ActiveSheet.Name = "Dashboard" Then Range(programNameCell).Select
If ActiveSheet.Name = "Batch" Then Range("A1").Select
Call ResetAll
End Sub

Sub RedFilename()
Range(loadedFileCell).Cells.Font.Color = RGB(192, 0, 0)
End Sub

Sub BlackFilename()
Range(loadedFileCell).Cells.Font.Color = RGB(0, 0, 0)
End Sub

Sub On3DSelectionChange()
Call MapInputParameters
If bEnable3D Then
    Call Expand3D(frontContactCells):  Call Expand3D(rearContactCells)
    Call Expand3D(frontSheetRhoCells): Call Expand3D(rearSheetRhoCells)
    Call Expand3D(frontTransCells):    Call Expand3D(rearTransCells)
    Call Expand3D(frontHazeCells):     Call Expand3D(rearHazeCells)
    Call Expand3D(frontSpecRefCells):  Call Expand3D(rearSpecRefCells)
    Call Expand3D(frontDiffRefCells):  Call Expand3D(rearDiffRefCells)
    Call Expand3D(frontJo1Cells):      Call Expand3D(rearJo1Cells)
    Call Expand3D(frontJo2Cells):      Call Expand3D(rearJo2Cells)
    Call Expand3D(frontGshCells):      Call Expand3D(rearGshCells)
Else
    Call Clear3D(frontContactCells):  Call Clear3D(rearContactCells)
    Call Clear3D(frontSheetRhoCells): Call Clear3D(rearSheetRhoCells)
    Call Clear3D(frontTransCells):    Call Clear3D(rearTransCells)
    Call Clear3D(frontHazeCells):     Call Clear3D(rearHazeCells)
    Call Clear3D(frontSpecRefCells):  Call Clear3D(rearSpecRefCells)
    Call Clear3D(frontDiffRefCells):  Call Clear3D(rearDiffRefCells)
    Call Clear3D(frontJo1Cells):      Call Clear3D(rearJo1Cells)
    Call Clear3D(frontJo2Cells):      Call Clear3D(rearJo2Cells)
    Call Clear3D(frontGshCells):      Call Clear3D(rearGshCells)
End If
bNeedReset = True
bStartup = False
End Sub

Sub OnResetFrontUnits()
Dim n As Integer
Dim widthUnits As Variant, lengthUnits As Variant
ReDim widthUnits(1 To 1, 1 To 5) As String
ReDim lengthUnits(1 To 5, 1 To 1) As String
For n = 1 To 5
    widthUnits(1, n) = "fit mm"
    lengthUnits(n, 1) = "fit mm"
Next n
Range(frontWidthUnitCells).Value = widthUnits
Range(frontLengthUnitCells).Value = lengthUnits
Call FitFrontDimensions
bNeedReset = True
bStartup = False
End Sub

Sub OnResetRearUnits()
Dim n As Integer
Dim widthUnits As Variant, lengthUnits As Variant
ReDim widthUnits(1 To 1, 1 To 5) As String
ReDim lengthUnits(1 To 5, 1 To 1) As String
For n = 1 To 5
    widthUnits(1, n) = "fit mm"
    lengthUnits(n, 1) = "fit mm"
Next n
Range(rearWidthUnitCells).Value = widthUnits
Range(rearLengthUnitCells).Value = lengthUnits
Call FitRearDimensions
bNeedReset = True
bStartup = False
End Sub

Sub OnDeviceChange()
Call MapInputParameters
Call FitFrontDimensions
Call FitRearDimensions
Range(contactWarningCellP).Value = "": Range(contactWarningCellN).Value = ""
If Not bContactP Then Range(contactWarningCellP).Value = "No positive polarity contact"
If Not bContactN Then Range(contactWarningCellN).Value = "No negative polarity contact"
Range(deviceAreaCell).Value = deviceWidth * deviceLength
b3D = Requires3D
If b3D Then Range(dimensionsCell).Value = "Resolution (3D):" Else Range(dimensionsCell).Value = "Resolution (2D):"
If bSiDiffusivities Then
    Range(deviceDnCell).Value = SiElectronDiffusivity(rDoping)
    Range(deviceDpCell).Value = SiHoleDiffusivity(rDoping)
End If
Call ResetElectrical    'Recalculates and posts diffusion lengths, wafer resistivity, etc.
bNeedReset = True
bStartup = False
End Sub

Sub OnDefaultSpectrum()
Dim index As Integer
Dim colRange As Variant
Call MapInputParameters
Range(spectrumDataCells).ClearContents
Range(opticalFilterCell).Value = "Disable"
Range(spectrumCell).Value = "Spectrum"
Range(frontSunsCell).Value = 1
Range(rearSunsCell).Value = 0
Range(frontPowerCell).Value = 1
Range(rearPowerCell).Value = 0
Range(lambdaCell).Value = 1000
colRange = Range(spectrumDataCells)
For index = 0 To UBound(rLambdaAMg)
    colRange(index + 1, 1) = rLambdaAMg(index)
    colRange(index + 1, 2) = rPowerAMg(index)
    colRange(index + 1, 3) = rSiAlpha(index)
    colRange(index + 1, 4) = 100
Next index
For index = UBound(rLambdaAMg) + 2 To UBound(colRange)
    colRange(index, 1) = ""
    colRange(index, 2) = ""
    colRange(index, 3) = ""
    colRange(index, 4) = ""
Next index
Range(spectrumDataCells) = colRange
Call OnIlluminationChange
End Sub

Sub OnFilterChange()
Dim n As Integer
Dim eV As Double, bandEdge As Double, absEdge As Double, maxTransmission As Double
Call MapInputParameters
For n = 1 To UBound(rSpectrumData)
    If bFilter Then
        If Not rSpectrumData(n, 1) > 0 Then Exit For        'No more wavelengths in table
        eV = hc / rSpectrumData(n, 1)
        bandEdge = Range(filterBandgapCell)
        absEdge = Range(filterBandgapCell) + Range(filterEdgeCell)
        maxTransmission = 100 - Range(filterSubgapCell)
        If absEdge - bandEdge > 0 Then
            rSpectrumData(n, 4) = 100 * (absEdge - eV) / (absEdge - bandEdge)
        Else
            If eV < bandEdge Then rSpectrumData(n, 4) = maxTransmission Else rSpectrumData(n, 4) = 0
        End If
        If rSpectrumData(n, 4) < 0 Then rSpectrumData(n, 4) = 0
        If rSpectrumData(n, 4) > maxTransmission Then rSpectrumData(n, 4) = maxTransmission
    Else
        If rSpectrumData(n, 1) > 0 Then rSpectrumData(n, 4) = 100 Else rSpectrumData(n, 1) = ""
    End If
Next n
Range(spectrumDataCells) = rSpectrumData
Call OnIlluminationChange
End Sub

Sub OnIlluminationChange()
Dim i As Integer, j As Integer, n As Integer
Dim monoFilter As Double, eV As Double, bandEdge As Double, absEdge As Double, maxTransmission As Double
Dim rUniformity As Variant
Call InitializeAll     'Necessary to set node count for the plots on this sheet. Calls MapInputParameters.
Range(uniformityRange).ClearContents
rUniformity = Range(uniformityRange)

'*** FRONT SURFACE ***
i = 0: j = 0
monoFilter = 0.01 * InterpolateColumns(rMonoLambda, 1, rSpectrumData, 4, rSpectrumData, False, False)
Call CalculateIlluminationUniformity(front, sFrontIllumination)
If frontShape = "Uniform" Then j = Round((nNodesY - 1) / 2, 0) + 1
If frontShape = "Point" Then j = Round(nNodesY - rFrontPointY * (nNodesY - 1) / 100, 0)
If frontShape = "Line" Then
    If frontLineDirection = "Side" Then i = Round((nNodesX - 1) / 2, 0) + 1          'X-axis
    If frontLineDirection = "Face" Then j = Round((nNodesY - 1) / 2, 0) + 1              'Y-axis
    If frontLineDirection = "Diagonal" Then j = Round((nNodesY - 1) / 2, 0) + 1        'Diagonal
End If
If i > 0 And j = 0 Then
    For j = 1 To nNodesY
        rUniformity(nNodesY - j + 1, 2) = 0
        If bSpectrum Then rUniformity(nNodesY - j + 1, 2) = rUniformity(nNodesY - j + 1, 2) + rFrontSuns * sFrontIllumination(i, j)
        If bMono Then rUniformity(nNodesY - j + 1, 2) = rUniformity(nNodesY - j + 1, 2) + rFrontMonoPower * sFrontIllumination(i, j) * monoFilter
    Next j
End If
If i = 0 And j > 0 Then
    For i = 1 To nNodesX
        rUniformity(i, 2) = 0
        If bSpectrum Then rUniformity(i, 2) = rUniformity(i, 2) + rFrontSuns * sFrontIllumination(i, j)
        If bMono Then rUniformity(i, 2) = rUniformity(i, 2) + rFrontMonoPower * sFrontIllumination(i, j) * monoFilter
    Next i
End If
'*** REAR SURFACE ***
'Note optical filter is not applied to rear-surface illumination
i = 0: j = 0
Call CalculateIlluminationUniformity(rear, sRearIllumination)
If rearShape = "Uniform" Then j = Round((nNodesY - 1) / 2, 0) + 1
If rearShape = "Point" Then j = Round(nNodesY - rRearPointY * (nNodesY - 1) / 100, 0)
If rearShape = "Line" Then
    If rearLineDirection = "Side" Then i = Round((nNodesX - 1) / 2, 0) + 1             'X-axis
    If rearLineDirection = "Face" Then j = Round((nNodesY - 1) / 2, 0) + 1               'Y-axis
    If rearLineDirection = "Diagonal" Then j = Round((nNodesY - 1) / 2, 0) + 1           'Diagonal
End If
If i > 0 And j = 0 Then
    For j = 1 To nNodesY
        rUniformity(nNodesY - j + 1, 3) = 0
        If bSpectrum Then rUniformity(nNodesY - j + 1, 3) = rUniformity(nNodesY - j + 1, 3) + rRearSuns * sRearIllumination(i, j)
        If bMono Then rUniformity(nNodesY - j + 1, 3) = rUniformity(nNodesY - j + 1, 3) + rRearMonoPower * sRearIllumination(i, j)
    Next j
End If
If i = 0 And j > 0 Then
    For i = 1 To nNodesX
        rUniformity(i, 3) = 0
        If bSpectrum Then rUniformity(i, 3) = rUniformity(i, 3) + rRearSuns * sRearIllumination(i, j)
        If bMono Then rUniformity(i, 3) = rUniformity(i, 3) + rRearMonoPower * sRearIllumination(i, j)
    Next i
End If
For n = 1 To nNodesX
    rUniformity(n, 1) = n
Next n
Range(uniformityRange) = rUniformity
Call ResetGeneration
bNeedReset = True
bStartup = False
End Sub

Sub Clear3D(ByVal cellRange As String)
Dim i As Integer, j As Integer
Dim rArray As Variant
rArray = Range(cellRange)
For i = 2 To UBound(rArray, 1): For j = 1 To UBound(rArray, 2)
    rArray(i, j) = ""
Next j: Next i
Range(cellRange) = rArray
End Sub

Sub Expand3D(ByVal cellRange As String)
Dim i As Integer, j As Integer
Dim rArray As Variant
rArray = Range(cellRange)
For i = 2 To UBound(rArray, 1): For j = 1 To UBound(rArray, 2)
    rArray(i, j) = rArray(1, j)
Next j: Next i
Range(cellRange) = rArray
End Sub

Sub FitFrontDimensions()
Dim columnWidth As Variant, rowLength As Variant
Dim widthUnits As Variant, lengthUnits As Variant
Dim specifiedDimension As Double, fitDimension As Double, eachDimension As Double
Dim i As Integer, nFit As Integer

deviceWidth = Range(deviceWidthCell) * 0.1
deviceLength = Range(deviceLengthCell) * 0.1

columnWidth = Range(frontColumnWidthCells)
widthUnits = Range(frontWidthUnitCells)

specifiedDimension = 0
fitDimension = 0
nFit = 0
For i = 1 To 5
    If widthUnits(1, i) = "%" Then specifiedDimension = specifiedDimension + columnWidth(1, i) * 10 * deviceWidth / 100 '%
    If widthUnits(1, i) = "mm" Then specifiedDimension = specifiedDimension + columnWidth(1, i)          'mm
    If widthUnits(1, i) = "fit mm" Then    'fitting column
        nFit = nFit + 1
        fitDimension = fitDimension + columnWidth(1, i)
    End If
Next i
If nFit > 0 Then
    eachDimension = (10 * deviceWidth - specifiedDimension)
    If eachDimension < 0.001 Then eachDimension = 0.001 Else eachDimension = eachDimension / nFit
    For i = 1 To 5
        If widthUnits(1, i) = "fit mm" Then columnWidth(1, i) = eachDimension
    Next i
End If
For i = 1 To 5
    If widthUnits(1, i) = "fit mm" Then Range(frontColumnWidthCells).Cells(1, i) = columnWidth(1, i)
Next i

rowLength = Range(frontRowLengthCells).Formula
lengthUnits = Range(frontLengthUnitCells)

specifiedDimension = 0
fitDimension = 0
nFit = 0
For i = 1 To 5
    If lengthUnits(i, 1) = "%" Then specifiedDimension = specifiedDimension + rowLength(i, 1) * 10 * deviceLength / 100 '%
    If lengthUnits(i, 1) = "mm" Then specifiedDimension = specifiedDimension + rowLength(i, 1)       'mm
    If lengthUnits(i, 1) = "fit mm" Then nFit = nFit + 1 'fitting row
Next i
If nFit > 0 Then
    eachDimension = (10 * deviceLength - specifiedDimension)
    If eachDimension < 0.001 Then eachDimension = 0.001 Else eachDimension = eachDimension / nFit
    For i = 1 To 5
        If lengthUnits(i, 1) = "fit mm" Then rowLength(i, 1) = eachDimension
    Next i
End If
For i = 1 To 5
    If lengthUnits(i, 1) = "fit mm" Then Range(frontRowLengthCells).Cells(i, 1) = rowLength(i, 1)
Next i

Range(frontDimensionWarningCell).Value = ""
If WorksheetFunction.CountIf(Range("Device!B11:F11"), "<=0") + WorksheetFunction.CountIf(Range("Device!G6:G10"), "<=0") Then _
    Range(frontDimensionWarningCell) = "Specified dimensions must not exceed Solution Volume dimensions"
If WorksheetFunction.CountIf(Range("Device!B12:F12"), "fit mm") * WorksheetFunction.CountIf(Range("Device!H6:H10"), "fit mm") = 0 Then _
    Range(frontDimensionWarningCell) = "At least one column and row must be set to fit mm"
End Sub

Sub FitRearDimensions()
Dim columnWidth As Variant, rowLength As Variant
Dim widthUnits As Variant, lengthUnits As Variant
Dim specifiedDimension As Double, fitDimension As Double, eachDimension As Double
Dim i As Integer, nFit As Integer

deviceWidth = Range(deviceWidthCell) * 0.1
deviceLength = Range(deviceLengthCell) * 0.1

columnWidth = Range(rearColumnWidthCells)
widthUnits = Range(rearWidthUnitCells)

specifiedDimension = 0
fitDimension = 0
nFit = 0
For i = 1 To 5
    If widthUnits(1, i) = "%" Then specifiedDimension = specifiedDimension + columnWidth(1, i) * 10 * deviceWidth / 100 '%
    If widthUnits(1, i) = "mm" Then specifiedDimension = specifiedDimension + columnWidth(1, i)          'mm
    If widthUnits(1, i) = "fit mm" Then nFit = nFit + 1 'fitting column
Next i
If nFit > 0 Then
    eachDimension = (10 * deviceWidth - specifiedDimension)
    If eachDimension < 0.001 Then eachDimension = 0.001 Else eachDimension = eachDimension / nFit
    For i = 1 To 5
        If widthUnits(1, i) = "fit mm" Then columnWidth(1, i) = eachDimension
    Next i
End If
For i = 1 To 5
    If widthUnits(1, i) = "fit mm" Then Range(rearColumnWidthCells).Cells(1, i) = columnWidth(1, i)
Next i

rowLength = Range(rearRowLengthCells)
lengthUnits = Range(rearLengthUnitCells)

specifiedDimension = 0
fitDimension = 0
nFit = 0
For i = 1 To 5
    If lengthUnits(i, 1) = "%" Then specifiedDimension = specifiedDimension + rowLength(i, 1) * 10 * deviceLength / 100 '%
    If lengthUnits(i, 1) = "mm" Then specifiedDimension = specifiedDimension + rowLength(i, 1)         'mm
    If lengthUnits(i, 1) = "fit mm" Then nFit = nFit + 1   'fitting row
Next i
If nFit > 0 Then
    eachDimension = (10 * deviceLength - specifiedDimension)
    If eachDimension < 0.001 Then eachDimension = 0.001 Else eachDimension = eachDimension / nFit
    For i = 1 To 5
        If lengthUnits(i, 1) = "fit mm" Then rowLength(i, 1) = eachDimension
    Next i
End If
For i = 1 To 5
    If lengthUnits(i, 1) = "fit mm" Then Range(rearRowLengthCells).Cells(i, 1) = rowLength(i, 1)
Next i

Range(rearDimensionWarningCell).Value = ""
If WorksheetFunction.CountIf(Range("Device!J11:N11"), "<=0") + WorksheetFunction.CountIf(Range("Device!O6:O10"), "<=0") Then _
    Range(rearDimensionWarningCell) = "Specified dimensions must not exceed Solution Volume dimensions"
If WorksheetFunction.CountIf(Range("Device!J12:N12"), "fit mm") * WorksheetFunction.CountIf(Range("Device!P6:P10"), "fit mm") = 0 Then _
    Range(rearDimensionWarningCell) = "At least one column and row must be set to fit mm"
End Sub

Sub MapInputParameters()
'Set global variables from spreadsheet values
'This routine must not change any spreadsheet values that trigger change handlers, to maintain cross-platform compatibility.
Dim m As Integer, n As Integer
deviceWidth = Range(deviceWidthCell) * 0.1       'cm
deviceLength = Range(deviceLengthCell) * 0.1      'cm
deviceHeight = Range(deviceHeightCell) * 0.0001    'cm
rConvergence = Range(convergenceCell)   'A/cm2
rVoltage = Range(voltageCell)           'volts (forward bias)
rTemperature = Range(temperatureCell) + rToffset
rVt = rVt300 * rTemperature / 300    'volts (thermal)
rNi = Range(intrinsicCell)         'cm-3
rDoping = Range(deviceDopingCell)           'cm-3
pType = rDoping >= 0
nType = Not pType
rPsiEq = -Sgn(rDoping) * rVt * Log(Abs(rDoping) / (2 * rNi) + Sqr((rDoping / (2 * rNi)) ^ 2 + 1))
rMinEq = rNi * Exp(-Abs(rPsiEq) / rVt)
rEqN = rNi * Exp(rPsiEq / rVt)
rEqP = rNi * Exp(-rPsiEq / rVt)
rSeries = Range(deviceSeriesCell)            'ohm-cm2
bSiDiffusivities = (Range(enableSiDiffusivitiesCell) = "Silicon")
bEnableTemperatureModel = (Range(enableTemperatureCell) = "Enable")
bEnableDopingModel = (Range(enableDopingCell) = "Enable")
rDp = Range(deviceDpCell)               'cm2/s
If rDp <= 0 Then rDp = 1
rDn = Range(deviceDnCell)               'cm2/s
If rDn <= 0 Then rDn = 1
rDmin = 2 * rDp * rDn / (rDp + rDn + (rEqN - rEqP) / (rEqP + rEqN) * (rDn - rDp))
rDamb = 2 * rDp * rDn / (rDp + rDn)
rTauN = Range(tauNCell)
If rTauN <= 0 Then rTauN = 1
If Range(tauNUnitsCell) = "µs" Then rTauN = rTauN * 0.000001  'us
If Range(tauNUnitsCell) = "ms" Then rTauN = rTauN * 0.001  'ms
rTauP = Range(tauPCell)
If rTauP <= 0 Then rTauP = 0
If Range(tauPUnitsCell) = "µs" Then rTauP = rTauP * 0.000001  'us
If Range(tauPUnitsCell) = "ms" Then rTauP = rTauP * 0.001  'ms
rCa = Range(augerCell)           'cm6/s
selectSurface = Range(selectSurfaceCell)
selectQF = Range(selectQFCell)
selectJCarrier = Range(selectJCarrierCell)
selectJDirection = Range(selectJDirectionCell)
selectJPortion = Range(selectJPortionCell)
rFrontContacts = Range(frontContactCells)
rFrontSheetRho = Range(frontSheetRhoCells)
rFrontTrans = Range(frontTransCells)
rFrontHaze = Range(frontHazeCells)
rFrontSpecRef = Range(frontSpecRefCells)
rFrontDiffRef = Range(frontDiffRefCells)
rFrontJo1 = Range(frontJo1Cells)
rFrontJo2 = Range(frontJo2Cells)
rFrontGsh = Range(frontGshCells)
rRearContacts = Range(rearContactCells)
rRearSheetRho = Range(rearSheetRhoCells)
rRearTrans = Range(rearTransCells)
rRearHaze = Range(rearHazeCells)
rRearSpecRef = Range(rearSpecRefCells)
rRearDiffRef = Range(rearDiffRefCells)
rRearJo1 = Range(rearJo1Cells)
rRearJo2 = Range(rearJo2Cells)
rRearGsh = Range(rearGshCells)
bFilter = (Range(opticalFilterCell) = "Enable")
bSpectrum = (Range(spectrumCell) = "Spectrum") Or (Range(spectrumCell) = "Both")
If bSpectrum Then rFrontSuns = Range(frontSunsCell) Else rFrontSuns = 0
If bSpectrum Then rRearSuns = Range(rearSunsCell) Else rRearSuns = 0
bMono = (Range(spectrumCell) = "Mono") Or (Range(spectrumCell) = "Both")
If bMono Then rFrontMonoPower = Range(frontPowerCell) Else rFrontMonoPower = 0
If bMono Then rRearMonoPower = Range(rearPowerCell) Else rRearMonoPower = 0
rMonoLambda = Range(lambdaCell)           'nm
frontShape = Range(frontShapeCell)
rFrontPointX = Range(frontPointXcenterCell)       '%
rFrontPointY = Range(frontPointYcenterCell)       '%
rFrontPointZ = Range(frontPointZcenterCell)       '%
rFrontPointDiameter = Range(frontPointDiameterCell)     'mm
frontLineDirection = Range(frontLineDirectionCell)
frontLineShape = Range(frontLineShapeCell)
rFrontLinePeak = Range(frontLinePeakCell)         '%
rFrontLineWidth = Range(frontLineWidthCell)       'mm
rearShape = Range(rearShapeCell)
rRearPointX = Range(rearPointXcenterCell)       '%
rRearPointY = Range(rearPointYcenterCell)       '%
rRearPointZ = Range(rearPointZcenterCell)       '%
rRearPointDiameter = Range(rearPointDiameterCell)     'mm
rearLineDirection = Range(rearLineDirectionCell)
rearLineShape = Range(rearLineShapeCell)
rRearLinePeak = Range(rearLinePeakCell)         '%
rRearLineWidth = Range(rearLineWidthCell)       'mm
rSpectrumData = Range(spectrumDataCells)    'nm, W/m2, cm-1, filter
rSpectralLambdas = Range(spectralLambdasCells)
rSpectralFront = Range(spectralFrontCells)
rSpectralRear = Range(spectralRearCells)
bEnable3D = (Range(enable3DCell) = "3D layout")
bOpenCircuit = (Range(openCircuitCell) = "Voc")
bEnablePointDefect = (Range(defectEnableCells).Cells(point, 1) = "Enable")
bEnableLineDefect = (Range(defectEnableCells).Cells(line, 1) = "Enable")
bEnablePlaneDefect = (Range(defectEnableCells).Cells(plane, 1) = "Enable")
rPointDefectIo = Range(defectIoCells).Cells(point, 1)
rLineDefectIo = Range(defectIoCells).Cells(line, 1)
rPlaneDefectIo = Range(defectIoCells).Cells(plane, 1)
rPointDefectN = Range(defectNCells).Cells(point, 1)
rLineDefectN = Range(defectNCells).Cells(line, 1)
rPlaneDefectN = Range(defectNCells).Cells(plane, 1)
lineDefectDirection = Range(lineDefectDirectionCell)
planeDefectDirection = Range(planeDefectDirectionCell)
rPointDamping = Range(defectDampingCells).Cells(point, 1)
rLineDamping = Range(defectDampingCells).Cells(line, 1)
rPlaneDamping = Range(defectDampingCells).Cells(plane, 1)
rDefectPosition = Range(defectPositionCells)    'point to plane, xAxis to zAxis
'Determine whether p and n contacts are present to set boolean variables
bContactP = False: bContactN = False
rFrontContacts = Range(frontContactCells): rRearContacts = Range(rearContactCells)
rFrontSheetRho = Range(frontSheetRhoCells): rRearSheetRho = Range(rearSheetRhoCells)
For m = 1 To nRows: For n = 1 To nColumns
    If pType And rFrontContacts(m, n) > 0 Then
        If rFrontSheetRho(m, n) < 0 Then bContactN = True Else bContactP = True
    End If
    If nType And rFrontContacts(m, n) > 0 Then
        If rFrontSheetRho(m, n) <= 0 Then bContactN = True Else bContactP = True
    End If
    If pType And rRearContacts(m, n) > 0 Then
        If rRearSheetRho(m, n) < 0 Then bContactN = True Else bContactP = True
    End If
    If nType And rRearContacts(m, n) > 0 Then
        If rRearSheetRho(m, n) <= 0 Then bContactN = True Else bContactP = True
    End If
Next n: Next m
End Sub

Function MapSurfaceParameters() As Boolean
'This routine maps the user spreadsheet surface parameters onto the solution volume sufaces (nNodesX x nNodesY per surface)
'An average over the region surrounding each node is used where appropriate.
'This is done by sampling that region in an nGrid x nGrid matrix centered around the node.
'In general, spreadsheet variables starting with r have units shown on the spreadsheet, but these are
'converted to cgs units for the correponding surface parameter values starting with s.
Dim columnWidth As Variant, columnUnits As Variant
Dim rowLength As Variant, rowUnits As Variant
Dim i As Integer, j As Integer, m As Integer, n As Integer
Dim elementX As Integer, elementY As Integer
Dim rSum As Double, deltaX As Double, deltaY As Double, rX As Double, rY As Double      'mm
Dim rXminus As Double, rXplus As Double, rYminus As Double, rYplus As Double            'mm
Dim columnBoundaries(1 To nColumns) As Double
Dim rowBoundaries(1 To nRows) As Double

MapSurfaceParameters = True
If Range(frontDimensionWarningCell) <> "" Or Range(rearDimensionWarningCell) <> "" Then
    If Not bBatch Then MsgBox ("Error in Dimensions on Device Sheet")
    MapSurfaceParameters = False
    Exit Function
End If

Range(progressCell).Value = "Meshing": DoEvents
Call MapInputParameters
Call CalculateTransforms

deltaX = 10 * deviceWidth / (nNodesX - 1)        'mm
deltaY = 10 * deviceLength / (nNodesY - 1)       'mm
    
'   *** FRONT SURFACE ***

'Set local Range variables from front-surface spreadsheet values
columnWidth = Range(frontColumnWidthCells)
columnUnits = Range(frontWidthUnitCells)
rowLength = Range(frontRowLengthCells)
rowUnits = Range(frontLengthUnitCells)

'Locate upper boundaries of surface elements
rSum = 0
For i = 1 To nColumns   'nColumns and deviceWidth (cm) are global
    If columnUnits(1, i) = "%" Then columnBoundaries(i) = rSum + columnWidth(1, i) * 10 * deviceWidth / 100
    If columnUnits(1, i) = "mm" Then columnBoundaries(i) = rSum + columnWidth(1, i)
    If columnUnits(1, i) = "fit mm" Then columnBoundaries(i) = rSum + columnWidth(1, i)
    rSum = columnBoundaries(i)
Next i
rSum = 0
For j = 1 To nRows   'nRows and deviceLength (cm) are global
    If rowUnits(j, 1) = "%" Then rowBoundaries(j) = rSum + rowLength(j, 1) * 10 * deviceLength / 100
    If rowUnits(j, 1) = "mm" Then rowBoundaries(j) = rSum + rowLength(j, 1)
    If rowUnits(j, 1) = "fit mm" Then rowBoundaries(j) = rSum + rowLength(j, 1)
    rSum = rowBoundaries(j)
Next j

'Now populate arrays by averaging over the nodal region
'Sheet conductance is averaged across the nodal region (not sheet resistance)
'Contact conductance is likewise averaged across the nodal region
For i = 1 To nNodesX: For j = 1 To nNodesY
    sFrontTransmission(i, j) = 0
    sFrontHaze(i, j) = 0
    sFrontSpecReflect(i, j) = 0
    sFrontDiffReflect(i, j) = 0
    sJo1(front, i, j) = 0
    sJo2(front, i, j) = 0
    sGsh(front, i, j) = 0
    sSigma(front, i, j) = 0
    sContact(front, holes, i, j) = 0
    sContact(front, electrons, i, j) = 0
    For m = 1 To nGrid: For n = 1 To nGrid
        rX = (i - 1) * deltaX + (m - (nGrid + 1) / 2) * deltaX / nGrid
        rY = (j - 1) * deltaY + (n - (nGrid + 1) / 2) * deltaY / nGrid
        'Test for outside solution volume and reflect position back within solution volume
        If rX < 0 Then rX = -rX
        If rX > 10 * deviceWidth Then rX = 20 * deviceWidth - rX
        If rY < 0 Then rY = -rY
        If rY > 10 * deviceLength Then rY = 20 * deviceLength - rY
        elementX = IDelement(rX, columnBoundaries)
        If bEnable3D Then elementY = IDelement(rY, rowBoundaries) Else elementY = 1
        sFrontTransmission(i, j) = sFrontTransmission(i, j) + 0.01 * rFrontTrans(elementY, elementX) / nGrid ^ 2
        sFrontHaze(i, j) = sFrontHaze(i, j) + 0.01 * rFrontHaze(elementY, elementX) / nGrid ^ 2
        sFrontSpecReflect(i, j) = sFrontSpecReflect(i, j) + 0.01 * rFrontSpecRef(elementY, elementX) / nGrid ^ 2
        sFrontDiffReflect(i, j) = sFrontDiffReflect(i, j) + 0.01 * rFrontDiffRef(elementY, elementX) / nGrid ^ 2
        sJo1(front, i, j) = sJo1(front, i, j) + 0.000000000000001 * rFrontJo1(elementY, elementX) / nGrid ^ 2
        sJo2(front, i, j) = sJo2(front, i, j) + 0.000000001 * rFrontJo2(elementY, elementX) / nGrid ^ 2
        sGsh(front, i, j) = sGsh(front, i, j) + 0.001 * rFrontGsh(elementY, elementX) / nGrid ^ 2
        If rFrontSheetRho(elementY, elementX) <> 0 Then sSigma(front, i, j) = sSigma(front, i, j) + (1 / rFrontSheetRho(elementY, elementX)) / nGrid ^ 2
        If pType And rFrontContacts(elementY, elementX) > 0 Then
            If rFrontSheetRho(elementY, elementX) < 0 Then
                sContact(front, electrons, i, j) = sContact(front, electrons, i, j) + (1000 / rFrontContacts(elementY, elementX)) / nGrid ^ 2
            Else
                sContact(front, holes, i, j) = sContact(front, holes, i, j) + (1000 / rFrontContacts(elementY, elementX)) / nGrid ^ 2
            End If
        End If
        If nType And rFrontContacts(elementY, elementX) > 0 Then
            If rFrontSheetRho(elementY, elementX) <= 0 Then
                sContact(front, electrons, i, j) = sContact(front, electrons, i, j) + (1000 / rFrontContacts(elementY, elementX)) / nGrid ^ 2
            Else
                sContact(front, holes, i, j) = sContact(front, holes, i, j) + (1000 / rFrontContacts(elementY, elementX)) / nGrid ^ 2
            End If
        End If
    Next n: Next m
Next j: Next i

'Define coarse "elements" for numerical solver as equal partitions across the solution dimensions
For i = 1 To nElements
    elementBoundariesX(front, i) = 10 * deviceWidth * i / nElements
    elementBoundariesY(front, i) = 10 * deviceLength * i / nElements
Next i

Call AllocateNodesToRegions(nNodesX, front, elementBoundariesX, eNodeAllocationX, eRegionLowerX, eRegionUpperX)
Call AllocateNodesToRegions(nNodesY, front, elementBoundariesY, eNodeAllocationY, eRegionLowerY, eRegionUpperY)
Call AssignNodesToElementNodes(nNodesX, front, elementBoundariesX, eElementLowerX, eElementUpperX)
Call AssignNodesToElementNodes(nNodesY, front, elementBoundariesY, eElementLowerY, eElementUpperY)

'   *** REAR SURFACE ***

'Set local Range variables from rear-surface spreadsheet values
columnWidth = Range(rearColumnWidthCells)
columnUnits = Range(rearWidthUnitCells)
rowLength = Range(rearRowLengthCells)
rowUnits = Range(rearLengthUnitCells)

'Locate upper boundaries of surface elements (nColumns then nRows)
rSum = 0
For i = 1 To nColumns   'nColumns and deviceWidth (cm) are global
    If columnUnits(1, i) = "%" Then columnBoundaries(i) = rSum + columnWidth(1, i) * 10 * deviceWidth / 100
    If columnUnits(1, i) = "mm" Then columnBoundaries(i) = rSum + columnWidth(1, i)
    If columnUnits(1, i) = "fit mm" Then columnBoundaries(i) = rSum + columnWidth(1, i)
    rSum = columnBoundaries(i)
Next i
rSum = 0
For j = 1 To nRows   'nRows and deviceLength (cm) are global
    If rowUnits(j, 1) = "%" Then rowBoundaries(j) = rSum + rowLength(j, 1) * 10 * deviceLength / 100
    If rowUnits(j, 1) = "mm" Then rowBoundaries(j) = rSum + rowLength(j, 1)
    If rowUnits(j, 1) = "fit mm" Then rowBoundaries(j) = rSum + rowLength(j, 1)
    rSum = rowBoundaries(j)
Next j

'Now populate arrays by averaging over the nodal region
'Sheet conductance is averaged across the nodal region (not sheet resistance)
'Contact conductance is likewise averaged across the nodal region
For i = 1 To nNodesX: For j = 1 To nNodesY
    sRearTransmission(i, j) = 0
    sRearHaze(i, j) = 0
    sRearSpecReflect(i, j) = 0
    sRearDiffReflect(i, j) = 0
    sJo1(rear, i, j) = 0
    sJo2(rear, i, j) = 0
    sGsh(rear, i, j) = 0
    sSigma(rear, i, j) = 0
    sContact(rear, holes, i, j) = 0
    sContact(rear, electrons, i, j) = 0
    For m = 1 To nGrid: For n = 1 To nGrid
        rX = (i - 1) * deltaX + (m - (nGrid + 1) / 2) * deltaX / nGrid
        rY = (j - 1) * deltaY + (n - (nGrid + 1) / 2) * deltaY / nGrid
        'Test for outside solution volume and reflect position back within solution volume
        If rX < 0 Then rX = -rX
        If rX > 10 * deviceWidth Then rX = 20 * deviceWidth - rX
        If rY < 0 Then rY = -rY
        If rY > 10 * deviceLength Then rY = 20 * deviceLength - rY
        elementX = IDelement(rX, columnBoundaries)
        If bEnable3D Then elementY = IDelement(rY, rowBoundaries) Else elementY = 1
        sRearTransmission(i, j) = sRearTransmission(i, j) + 0.01 * rRearTrans(elementY, elementX) / nGrid ^ 2
        sRearHaze(i, j) = sRearHaze(i, j) + 0.01 * rRearHaze(elementY, elementX) / nGrid ^ 2
        sRearSpecReflect(i, j) = sRearSpecReflect(i, j) + 0.01 * rRearSpecRef(elementY, elementX) / nGrid ^ 2
        sRearDiffReflect(i, j) = sRearDiffReflect(i, j) + 0.01 * rRearDiffRef(elementY, elementX) / nGrid ^ 2
        sJo1(rear, i, j) = sJo1(rear, i, j) + 0.000000000000001 * rRearJo1(elementY, elementX) / nGrid ^ 2
        sJo2(rear, i, j) = sJo2(rear, i, j) + 0.000000001 * rRearJo2(elementY, elementX) / nGrid ^ 2
        sGsh(rear, i, j) = sGsh(rear, i, j) + 0.001 * rRearGsh(elementY, elementX) / nGrid ^ 2
        If rRearSheetRho(elementY, elementX) <> 0 Then sSigma(rear, i, j) = sSigma(rear, i, j) + (1 / rRearSheetRho(elementY, elementX)) / nGrid ^ 2
        If pType And rRearContacts(elementY, elementX) > 0 Then
            If rRearSheetRho(elementY, elementX) < 0 Then
                sContact(rear, electrons, i, j) = sContact(rear, electrons, i, j) + (1000 / rRearContacts(elementY, elementX)) / nGrid ^ 2
            Else
                sContact(rear, holes, i, j) = sContact(rear, holes, i, j) + (1000 / rRearContacts(elementY, elementX)) / nGrid ^ 2
            End If
        End If
        If nType And rRearContacts(elementY, elementX) > 0 Then
            If rRearSheetRho(elementY, elementX) <= 0 Then
                sContact(rear, electrons, i, j) = sContact(rear, electrons, i, j) + (1000 / rRearContacts(elementY, elementX)) / nGrid ^ 2
            Else
                sContact(rear, holes, i, j) = sContact(rear, holes, i, j) + (1000 / rRearContacts(elementY, elementX)) / nGrid ^ 2
            End If
        End If
    Next n: Next m
Next j: Next i

'Define coarse "elements" for numerical solver as equal partiitons across the solution dimensions.
For i = 1 To nElements
    elementBoundariesX(rear, i) = 10 * deviceWidth * i / nElements
    elementBoundariesY(rear, i) = 10 * deviceLength * i / nElements
Next i

Call AllocateNodesToRegions(nNodesX, rear, elementBoundariesX, eNodeAllocationX, eRegionLowerX, eRegionUpperX)
Call AllocateNodesToRegions(nNodesY, rear, elementBoundariesY, eNodeAllocationY, eRegionLowerY, eRegionUpperY)
Call AssignNodesToElementNodes(nNodesX, rear, elementBoundariesX, eElementLowerX, eElementUpperX)
Call AssignNodesToElementNodes(nNodesY, rear, elementBoundariesY, eElementLowerY, eElementUpperY)

If (pType And Not bContactP) Or (nType And Not bContactN) Then
    If Not bBatch Then MsgBox ("No Base Contact on Device Sheet")
    MapSurfaceParameters = False
End If

End Function

Sub DefineRegionBoundaries(ByVal nSurface As Integer, ByRef elementBoundaries As Variant, ByRef regionBoundaries As Variant)
'For the given set of coarse element boundaries, returns an array defining the regions that surround each element node.
'regionBoundaries(front to rear, 1 to nRegions) is the upper boundary location for each of these regions.
Dim iElement As Integer, iRegion As Integer
Dim rTotalDistance As Double
rTotalDistance = elementBoundaries(nSurface, nElements)
'Leftmost element gets special treatment since zero boundary is assumed, not stored
regionBoundaries(nSurface, 1) = elementBoundaries(nSurface, 1) / 2
For iElement = 2 To nElements
    regionBoundaries(nSurface, iElement) = _
        elementBoundaries(nSurface, iElement - 1) + _
            (elementBoundaries(nSurface, iElement) - elementBoundaries(nSurface, iElement - 1)) / 2
Next iElement
regionBoundaries(nSurface, nRegions) = rTotalDistance
End Sub

Sub AllocateNodesToRegions(ByVal nNodes As Integer, ByVal nSurface As Integer, ByRef elementBoundaries As Variant, _
                           ByRef eArray As Variant, ByRef eLower As Variant, ByRef eUpper As Variant)
'Assigns a fraction of each surface node's area to each surface region, where a region
'is the area surrounding each element node (intersections of the element boundaries).
'Returns eArray(1 to nNodes, 1 to nRegions) that contains these allocations.
'Also returns eLower(front to rear, 1 to nRegions) and eUpper(front to rear, 1 to nRegions)
'as the lower and upper node whose area is allocated (at least partially) to the region surrounding each element node.
'Can be applied to either X or Y Direction, for either front or rear surface.
'The allocation to a specific region in XY is the product of the X allocation times the Y allocation.
Dim iNode As Integer, iRegion As Integer
Dim rDeltaDistance As Double, rHalfDistance As Double, rNodePosition As Double
Dim rLeft As Double, rRight As Double
Dim rTotalDistance As Double
Dim regionBoundaries As Variant     'These are the midpoints between the element boundaries
rTotalDistance = elementBoundaries(nSurface, nElements)
rDeltaDistance = rTotalDistance / (nNodes - 1)
rHalfDistance = rDeltaDistance / 2
ReDim regionBoundaries(front To rear, 1 To nRegions) As Double 'right edge of each region, with left edge of first region at zero
Call DefineRegionBoundaries(nSurface, elementBoundaries, regionBoundaries)
For iNode = 1 To nNodes
    rNodePosition = (iNode - 1) * rDeltaDistance
    For iRegion = 1 To nRegions
        eArray(nSurface, iNode, iRegion) = 0
        If iRegion = 1 Then rLeft = rNodePosition Else rLeft = rNodePosition - regionBoundaries(nSurface, iRegion - 1)
        rRight = regionBoundaries(nSurface, iRegion) - rNodePosition
        If rLeft > -rHalfDistance And rRight > -rHalfDistance Then
            If rLeft < rHalfDistance And rRight < rHalfDistance Then eArray(nSurface, iNode, iRegion) = (rLeft + rRight) / rDeltaDistance
            If rLeft < rHalfDistance And rRight >= rHalfDistance Then eArray(nSurface, iNode, iRegion) = (rLeft + rHalfDistance) / rDeltaDistance
            If rLeft >= rHalfDistance And rRight < rHalfDistance Then eArray(nSurface, iNode, iRegion) = (rHalfDistance + rRight) / rDeltaDistance
            If rLeft >= rHalfDistance And rRight >= rHalfDistance Then eArray(nSurface, iNode, iRegion) = 1
        End If
    Next iRegion
Next iNode
For iRegion = 1 To nRegions
    For iNode = 1 To nNodes
        If eArray(nSurface, iNode, iRegion) > 0 Then
            eLower(nSurface, iRegion) = iNode
            Exit For
        End If
    Next iNode
    For iNode = eLower(nSurface, iRegion) To nNodes
        If eArray(nSurface, iNode, iRegion) > 0 Then
            eUpper(nSurface, iRegion) = iNode
        End If
    Next iNode
Next iRegion
End Sub

Sub AssignNodesToElementNodes(ByVal nNodes As Integer, ByVal nSurface As Integer, ByRef elementBoundaries As Variant, _
                              ByRef eLower As Variant, eUpper As Variant)
'Assigns the range of nodes that lie within each element.
'Returns eLower(front to rear, 1 to nElementsX) and eUpper(front to rear, 1 to nElementsY)
'Interior nodes that lie on element boundaries are assigned to the lower-numbered element
Dim iElement As Integer, iNode As Integer
Dim rScale As Double, rPosition As Double
rScale = elementBoundaries(nSurface, UBound(elementBoundaries, 2))
'Leftmost element
eLower(nSurface, 1) = 1
For iNode = 1 To nNodes
    rPosition = rScale * (iNode - 1) / (nNodes - 1)
    If rPosition <= elementBoundaries(nSurface, 1) Then
        eUpper(nSurface, 1) = iNode
    Else: Exit For
    End If
Next iNode
'Central elements
For iElement = 2 To nElements - 1
    eLower(nSurface, iElement) = eUpper(nSurface, iElement - 1) + 1
    For iNode = eLower(nSurface, iElement) To nNodes
        rPosition = rScale * (iNode - 1) / (nNodes - 1)
        If rPosition <= elementBoundaries(nSurface, iElement) Then
            eUpper(nSurface, iElement) = iNode
        Else: Exit For
        End If
    Next iNode
Next iElement
'Rightmost element
eLower(nSurface, nElements) = eUpper(nSurface, nElements - 1) + 1
eUpper(nSurface, nElements) = nNodes
End Sub

