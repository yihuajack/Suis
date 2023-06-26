Attribute VB_Name = "SpreadsheetInterface"
Option Explicit

Sub OnResetAll()
bEnableChangeHandlers = False
If ActiveSheet.Name = "Dashboard" Then Range(programNameCell).Select
If ActiveSheet.Name = "Batch" Then Range("A1").Select
Range(viewCell).Value = "Face"
Range(positionCell).Value = 0
Range(selectCarrierCell).Value = "Combined"
currentPlot = "Doping"
Call ResetAll
End Sub

Sub RedFilename()
Range(loadedFileCell).Cells.Font.Color = RGB(192, 0, 0)
End Sub

Sub BlackFilename()
Range(loadedFileCell).Cells.Font.Color = RGB(0, 0, 0)
End Sub

Sub OnShapeChange()
'Handles changes to the shape drop-down lists for Diffusion, S, and Q
If Range(diffusionShapeCell) = "none" Then
    Range(deviceDiffusionPeakCell).Interior.Color = RGB(230, 230, 230)
    Range(deviceDiffusionPeakCell).Font.Color = RGB(120, 120, 120)
Else
    Range(deviceDiffusionPeakCell).Interior.Color = 65535
    Range(deviceDiffusionPeakCell).Font.Color = RGB(0, 0, 0)
End If
Select Case Range(frontSshapeCell)
    Case "Uniform"
        Range(frontSpeakCell).Interior.Color = 65535
        Range(frontSpeakCell).Font.Color = RGB(0, 0, 0)
        Range(frontSvalleyCell).Value = Range(frontSpeakCell)
        Range(frontSvalleyCell).Interior.Color = RGB(230, 230, 230)
        Range(frontSvalleyCell).Font.Color = RGB(120, 120, 120)
    Case Else
        Range(frontSpeakCell).Interior.Color = 65535
        Range(frontSpeakCell).Font.Color = RGB(0, 0, 0)
        Range(frontSvalleyCell).Interior.Color = 65535
        Range(frontSvalleyCell).Font.Color = RGB(0, 0, 0)
End Select
Select Case Range(frontChargeShapeCell)
    Case "Uniform"
        Range(frontChargePeakCell).Interior.Color = 65535
        Range(frontChargePeakCell).Font.Color = RGB(0, 0, 0)
        Range(frontChargeValleyCell).Value = Range(frontChargePeakCell)
        Range(frontChargeValleyCell).Interior.Color = RGB(230, 230, 230)
        Range(frontChargeValleyCell).Font.Color = RGB(120, 120, 120)
    Case Else
        Range(frontChargeValleyCell).Interior.Color = 65535
        Range(frontChargeValleyCell).Font.Color = RGB(0, 0, 0)
End Select
bNeedReset = True
End Sub

Sub OnTextureChange()
'Handles changes to the device that do not require an immediate ResetAll
Call MapInputParameters
Call CalculateTexture
bNeedReset = True
End Sub

Sub OnDopingChange()
Call MapInputParameters
rBackgroundDp = SiHoleDiffusivity(rBackgroundP, rBackgroundN)
rBackgroundDn = SiElectronDiffusivity(rBackgroundP, rBackgroundN)
Range(resistivityCell).Value = rVt / (rQ * (rBackgroundP * rBackgroundDp + rBackgroundN * rBackgroundDn))
Select Case nBackgroundType
    Case ptype: Range(minorityCell).Value = rNi ^ 2 / rBackgroundP
    Case ntype: Range(minorityCell).Value = rNi ^ 2 / rBackgroundN
End Select
bNeedReset = True
End Sub

Sub OnTemperatureChange()
Dim i As Integer, j As Integer, k As Integer
MapInputParameters
Range(thermalVoltageCell).Value = rVt
Range(intrinsicCell).Value = SiIntrinsicConcentration(rTemperature)
Select Case nBackgroundType
    Case ptype: Range(minorityCell).Value = rNi ^ 2 / rBackgroundP
    Case ntype: Range(minorityCell).Value = rNi ^ 2 / rBackgroundN
End Select
'Diffusivities
rBackgroundDp = SiHoleDiffusivity(rBackgroundP, rBackgroundN)
rBackgroundDn = SiElectronDiffusivity(rBackgroundP, rBackgroundN)
Range(resistivityCell).Value = rVt / (rQ * (rBackgroundP * rBackgroundDp + rBackgroundN * rBackgroundDn))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    vDp(i, j, k) = SiHoleDiffusivity(vDopingP(i, j, k), vDopingN(i, j, k))
    vDn(i, j, k) = SiElectronDiffusivity(vDopingP(i, j, k), vDopingN(i, j, k))
Next k: Next j: Next i
End Sub

Sub OnDefaultSpectrum()
Dim index As Integer
Dim colRange As Variant
Call MapInputParameters
Range(spectrumDataCells).ClearContents
Range(spectrumCell).Value = "Spectrum"
Range(frontSunsCell).Value = 1
colRange = Range(spectrumDataCells)
For index = 0 To UBound(rLambdaAMg)
    colRange(index + 1, 1) = rLambdaAMg(index)
    colRange(index + 1, 2) = rPowerAMg(index)
    colRange(index + 1, 3) = rSiAlpha(index)
    colRange(index + 1, 4) = rSiIndex(index)
Next index
For index = UBound(rLambdaAMg) + 2 To UBound(colRange, 1)
    colRange(index, 1) = ""
    colRange(index, 2) = ""
    colRange(index, 3) = ""
    colRange(index, 4) = ""
Next index
Range(spectrumDataCells) = colRange
End Sub

Sub MapInputParameters()
'Set global variables from spreadsheet values
'This routine must not change any spreadsheet values that trigger change handlers, to maintain cross-platform compatibility.
Dim m As Integer, n As Integer
nMaxIterations = Range(maxIterationsCell)
rDamping = Range(dampingCell)
Select Case Range(textureTypeCell)
    Case "Planar": nTextureType = planar
    Case "Grooves": nTextureType = grooves
    Case "Pyramids": nTextureType = pyramids
    Case "Inverted": nTextureType = inverted
End Select
textureAngle = Range(textureAngleCell) * pi / 180
textureDepth = Range(textureDepthCell) * 0.0001
If nTextureType = planar Then textureAngle = 0
If textureAngle = 0 Then textureDepth = 0
If textureDepth = 0 Then nTextureType = planar    'Override selection if there is no actual texture
If nTextureType = planar Then rCosTextureAngle = 1 Else rCosTextureAngle = Cos(textureAngle)
If nTextureType = planar Then rSinTextureAngle = 0 Else rSinTextureAngle = Sin(textureAngle)
baseHeight = Range(baseHeightCell) * 0.0001
rConvergence = Range(convergenceCell)   'A/cm2
Select Case Range(contactModeCell)
    Case "Bipolar": nSolutionMode = bipolar: Range(contactUnitsCell).Value = "V": rVoltage = Range(voltageCell)
    Case "Floating": nSolutionMode = floating: Range(contactUnitsCell).Value = "V": rVoltage = Range(voltageCell)
    Case "Open": nSolutionMode = opencircuit: Range(contactUnitsCell).Value = "N/A":  rVoltage = 0
    Case "FlatBand": nSolutionMode = flatband: Range(contactUnitsCell).Value = "V": rVoltage = Range(voltageCell)
    Case "Lateral": nSolutionMode = lateral: Range(contactUnitsCell).Value = "mV": rVoltage = Range(voltageCell) * 0.001
End Select
bIllumination = (Range(illuminationCell) = "Light")
rTemperature = Range(temperatureCell) + rToffset
rVt = rVt300 * rTemperature / 300    'volts (thermal)
rNi25C = Range(intrinsic25Cell)         'cm-3
rNi = SiIntrinsicConcentration(rTemperature)
rEps = Range(permittivityCell)
rQ_Eps = rQ / (rEps0 * rEps)
rBackgroundDoping = Range(deviceDopingCell)           'cm-3
Select Case Range(deviceDopingTypeCell)
    Case "p-type": nBackgroundType = ptype
    Case "n-type": nBackgroundType = ntype
End Select
rBackgroundPsi = rVt * Log(Abs(rBackgroundDoping) / (2 * rNi) + Sqr((rBackgroundDoping / (2 * rNi)) ^ 2 + 1))
If nBackgroundType = ptype Then rBackgroundPsi = -rBackgroundPsi
rBackgroundN = rNi * ExpPower(rBackgroundPsi / rVt)
rBackgroundP = rNi * ExpPower(-rBackgroundPsi / rVt)
Select Case Range(diffusionShapeCell)
    Case "gaussian": nDiffusionShape = gaussian: rDiffusionPeak = Range(deviceDiffusionPeakCell)
    Case "erfc": nDiffusionShape = erfc: rDiffusionPeak = Range(deviceDiffusionPeakCell)
    Case "none": nDiffusionShape = none: rDiffusionPeak = 0
End Select
rDiffusionDepth = Range(deviceDiffusionDepthCell) * 0.0001  'cm
Select Case Range(deviceDiffusionTypeCell)
    Case "p-type": nDiffusionType = ptype
    Case "n-type": nDiffusionType = ntype
End Select
nSurfaceType = SurfaceType
rTauN = Range(tauNCell)
If rTauN <= 0 Then rTauN = 1 Else rTauN = rTauN * 0.000001  'us
rTauP = Range(tauPCell)
If rTauP <= 0 Then rTauP = 0 Else rTauP = rTauP * 0.000001  'us
rCp = Range(augerPtypeCell)           'cm6/s
rCn = Range(augerNtypeCell)           'cm6/s
rVpSlope = Range(valenceBGNslopeCell)
rVpOnset = Range(valenceBGNonsetCell)
rVnSlope = Range(conductionBGNslopeCell)
rVnOnset = Range(conductionBGNonsetCell)
Select Case Range(selectCarrierCell)
    Case "Holes": nSelectCarrier = holes
    Case "Electrons": nSelectCarrier = electrons
    Case "Combined": nSelectCarrier = combined
End Select
rFrontSpeak = Range(frontSpeakCell)
rFrontSvalley = Range(frontSvalleyCell)
rFrontQpeak = Range(frontChargePeakCell)
rFrontQvalley = Range(frontChargeValleyCell)
bSpectrum = (Range(spectrumCell) = "Spectrum") Or (Range(spectrumCell) = "Both")
If bSpectrum Then rFrontSuns = Range(frontSunsCell) Else rFrontSuns = 0
bMono = (Range(spectrumCell) = "Mono") Or (Range(spectrumCell) = "Both")
If bMono Then rFrontMonoPower = Range(frontPowerCell) Else rFrontMonoPower = 0
rMonoLambda = Range(lambdaCell)           'nm
rEncapsulationIndex = Range(encapsulationIndexCell)
rSpectrumData = Range(spectrumDataCells)    'wavelengths top-to-bot, units left-to-right are nm, W/m2, cm-1, None
rCoatingData = Range(coatingDataCells)      'layers left to right. Units top-to-bot are nm, dimensionless, eV-1, eV, 4xNone
bEnablePointDefect = (Range(defectEnableCells).Cells(point, 1) = "Enable")
bEnableLineDefect = (Range(defectEnableCells).Cells(line, 1) = "Enable")
bEnablePlaneDefect = (Range(defectEnableCells).Cells(plane, 1) = "Enable")
rDefectTauN = Range(defectTauNCell) * 0.000000001
rDefectTauP = Range(defectTauPCell) * 0.000000001
rDefectBGN = Range(defectBGNCell)
lineDefectDirection = Range(lineDefectDirectionCell)
planeDefectDirection = Range(planeDefectDirectionCell)
rDefectPosition = Range(defectPositionCells)    'point to plane, xAxis to zAxis
End Sub

